
#define FROXEL_GRID_SIZE_X 160
#define FROXEL_GRID_SIZE_Y 90
#define FROXEL_GRID_SIZE_Z 92

// hardcoded constants to prevent computational overhead.
static const float Pi = 3.14159265358979323846;
static const float InvPi = 0.31830988618379067154;
static const float Inv2Pi = 0.15915494309189533577;
static const float Inv4Pi = 0.07957747154594766788;
static const float PiOver2 = 1.57079632679489661923;
static const float PiOver4 = 0.78539816339744830961;
static const float Sqrt2 = 1.41421356237309504880;

// Henyey and Greenstein implementation of phase function
float PhaseHG(float cosTheta, float g)
{
    float denom = 1 + g * g + 2 * g * cosTheta;
    if (denom <= 0)
    {
        return 0;
    }
    return Inv4Pi * (1.f - g * g) / (denom * sqrt(denom));
}
float Phase(in float3 wo, in float3 wi, in float g)
{
    return PhaseHG(dot(wo, wi), g);
}

float4 RayleighScattering(float cosTheta, float4 rayleighParams, float4 camPos)
{
    float4 beta = rayleighParams; // Rayleigh extinction coefficient
    float4 sigma = beta / (4.0 * Pi); // Rayleigh scattering coefficient
    
    float camPosLength = length(camPos);
    if (camPosLength <= 0)
    {
        return float4(0, 0, 0, 0);
    }
    
    float4 rayleighScattering = sigma * exp(-beta * camPosLength); // Rayleigh scattering contribution
    
    return rayleighScattering;
}

float4 MieScattering(float cosTheta, float4 mieParams)
{
    float g = mieParams.x; // Mie asymmetry parameter
    float4 beta = float4(mieParams.yzw, 1); // Mie extinction coefficient
    float4 sigma = float4(mieParams.xyz, 1); // Mie scattering coefficient
    
    float3 v = float3(0.0, 0.0, 1.0); // View direction
    float3 w = float3(0.0, 1.0, 0.0); // Up direction
    float3 u = cross(v, w); // Right direction
    float3 direction = normalize(cosTheta * v + sqrt(1.0 - cosTheta * cosTheta) * (cos(g) * u + sin(g) * w)); // Scattered direction
    
    float4 mieTerm = exp(-beta * sigma); // Mie scattering term
    float denom = 1.0 + g * g - 2.0 * g * cosTheta;
    if (denom <= 0)
    {
        return float4(0, 0, 0, 0);
    }
    float miePhase = 1.5 * (1.0 - g * g) / (2.0 + g * g) * (1.0 + cosTheta * cosTheta) / pow(denom, 1.5); // Mie phase function
    float4 mieScattering = sigma * mieTerm * miePhase; // Mie scattering contribution
    
    return mieScattering;
}

float LinearizeDepth(float depthValue, float nearPlane, float farPlane)
{
    return (2.0 * nearPlane) / (farPlane + nearPlane - depthValue * (farPlane - nearPlane));
}

float ExpToLinearDepth(float z, float n, float f)
{
    float z_buffer_params_y = f / n;
    float z_buffer_params_x = 1.0f - z_buffer_params_y;

    return 1.0f / (z_buffer_params_x * z + z_buffer_params_y);
}

// ------------------------------------------------------------------

float LinearToExpDepth(float z, float n, float f)
{
    float z_buffer_params_y = f / n;
    float z_buffer_params_x = 1.0f - z_buffer_params_y;

    return (1.0f / z - z_buffer_params_y) / z_buffer_params_x;
}

float3 ToUv(uint3 id, float near, float far, float jitter)
{
    float view_z = near * pow(abs(far / near), (float(id.z) + 0.5f + jitter) / float(FROXEL_GRID_SIZE_Z));

    return float3((float(id.x) + 0.5f) / float(FROXEL_GRID_SIZE_X),
                (float(id.y) + 0.5f) / float(FROXEL_GRID_SIZE_Y),
                view_z / far);
}

float3 UvToNdc(float3 uv, float n, float f, float depth_power)
{
    float3 ndc;
        
    ndc.x = 2.0f * uv.x - 1.0f;
    ndc.y = 2.0f * uv.y - 1.0f;
    ndc.z = 2.0f * LinearToExpDepth(uv.z, n, f) - 1.0f;
        
    return ndc;
}

float3 NdcToWorld(float3 ndc, float4x4 inverseVP)
{
    float4 p = mul(inverseVP, float4(ndc, 1.0f));
        
    p.x /= p.w;
    p.y /= p.w;
    p.z /= p.w;
        
    return p.xyz;
}
float3 WorldToNdc(float3 worldpos, float4x4 vp)
{
    float4 p = mul(vp, float4(worldpos, 1.0f));
        
    if (p.w > 0.0f)
    {
        p.x /= p.w;
        p.y /= p.w;
        p.z /= p.w;
    }
    
    return p.xyz;
}

float3 NdcToUv(float3 ndc, float n, float f, float depth_power)
{
    float3 uv;

    uv.x = ndc.x * 0.5f + 0.5f;
    uv.y = ndc.y * 0.5f + 0.5f;

    float expDepth = ExpToLinearDepth(ndc.z * 0.5f + 0.5f, n, f);

    if (n >= 0 && f >= 0 && expDepth >= 0) // Check for negative values
    {
        // Exponential View-Z
        float2 params = float2(float(FROXEL_GRID_SIZE_Z) / log2(f / n), -(float(FROXEL_GRID_SIZE_Z) * log2(n) / log2(f / n)));

        float view_z = expDepth * f;
        float log_z = log2(view_z);

        if (params.x != 0) // Check for division by zero
        {
            uv.z = (max(log_z * params.x + params.y, 0.0f)) / FROXEL_GRID_SIZE_Z;
        }
        else
        {
            uv.z = 0.0f;
        }
    }
    else
    {
        uv.z = 0.0f;
    }

    return uv;
}

float3 WorldToUv(float3 worldPos, float near, float far, float depthPow, float4x4 vp)
{
    float3 ndc = WorldToNdc(worldPos, vp);
    return NdcToUv(ndc, near, far, depthPow);
}

float3 ToWorld(uint3 id, float near, float far, float depth_power, float4x4 inverseVP, float jitter)
{
    float3 uv = ToUv(id, near, far, jitter);
    float3 ndc = UvToNdc(uv, near, far, depth_power);
    return NdcToWorld(ndc, inverseVP);
}

float zSliceThickness(int z)
{
    return exp(-float(FROXEL_GRID_SIZE_Z - z - 1) / float(FROXEL_GRID_SIZE_Z));
}

// https://gist.github.com/Fewes/59d2c831672040452aa77da6eaab2234
//! Tricubic interpolated texture lookup, using unnormalized coordinates.
//! Fast implementation, using 8 trilinear lookups.
//! @param tex  3D texture
//! @param coord  normalized 3D texture coordinate
//! @param textureSize  the size (resolution) of the texture
float4 tex3DTricubic(Texture3D tex, SamplerState state, float3 coord, float3 textureSize)
{
	// Shift the coordinate from [0,1] to [-0.5, textureSize-0.5]
    float3 coord_grid = coord * textureSize - 0.5;
    float3 index = floor(coord_grid);
    float3 fraction = coord_grid - index;
    float3 one_frac = 1.0 - fraction;

    float3 w0 = 1.0 / 6.0 * one_frac * one_frac * one_frac;
    float3 w1 = 2.0 / 3.0 - 0.5 * fraction * fraction * (2.0 - fraction);
    float3 w2 = 2.0 / 3.0 - 0.5 * one_frac * one_frac * (2.0 - one_frac);
    float3 w3 = 1.0 / 6.0 * fraction * fraction * fraction;

    float3 g0 = w0 + w1;
    float3 g1 = w2 + w3;
    float3 mult = 1.0 / textureSize;
    float3 h0 = mult * ((w1 / g0) - 0.5 + index); //h0 = w1/g0 - 1, move from [-0.5, textureSize-0.5] to [0,1]
    float3 h1 = mult * ((w3 / g1) + 1.5 + index); //h1 = w3/g1 + 1, move from [-0.5, textureSize-0.5] to [0,1]

	// Fetch the eight linear interpolations
	// Weighting and fetching is interleaved for performance and stability reasons
    float4 tex000 = tex.SampleLevel(state, float3(h0), 0);
    float4 tex100 = tex.SampleLevel(state, float3(h1.x, h0.y, h0.z), 0);
    tex000 = lerp(tex100, tex000, g0.x); // Weigh along the x-direction

    float4 tex010 = tex.SampleLevel(state, float3(h0.x, h1.y, h0.z), 0);
    float4 tex110 = tex.SampleLevel(state, float3(h1.x, h1.y, h0.z), 0);
    tex010 = lerp(tex110, tex010, g0.x); // Weigh along the x-direction
    tex000 = lerp(tex010, tex000, g0.y); // Weigh along the y-direction

    float4 tex001 = tex.SampleLevel(state, float3(h0.x, h0.y, h1.z), 0);
    float4 tex101 = tex.SampleLevel(state, float3(h1.x, h0.y, h1.z), 0);
    tex001 = lerp(tex101, tex001, g0.x); // Weigh along the x-direction

    float4 tex011 = tex.SampleLevel(state, float3(h0.x, h1.y, h1.z), 0);
    float4 tex111 = tex.SampleLevel(state, float3(h1), 0);
    tex011 = lerp(tex111, tex011, g0.x); // Weigh along the x-direction
    tex001 = lerp(tex011, tex001, g0.y); // Weigh along the y-direction

    return lerp(tex001, tex000, g0.z); // Weigh along the z-direction
}