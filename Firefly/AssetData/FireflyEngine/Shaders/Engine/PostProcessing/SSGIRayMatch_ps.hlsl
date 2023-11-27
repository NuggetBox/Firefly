#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>
#include <Includes/Math.hlsli>


Texture2D u_StochasticNormalsTexture : register(t0);
Texture2D u_BlueNoiseTexture : register(t1);
Texture2D u_DepthTexture : register(t2);
Texture2D u_ColorTexture : register(t3);


float2 mod_dither3(float2 u)
{
    float noiseX = fmod(u.x + u.y + fmod(208. + u.x * 3.58, 13. + fmod(u.y * 22.9, 9.)), 7.) * .143;
    float noiseY = fmod(u.y + u.x + fmod(203. + u.y * 3.18, 12. + fmod(u.x * 27.4, 8.)), 6.) * .139;
    return float2(noiseX, noiseY) * 2.0 - 1.0;
}

float2 dither(float2 coord, float seed, float2 size)
{
    float noiseX = ((frac(1.0 - (coord.x + seed * 1.0) * (size.x / 2.0)) * 0.25) + (frac((coord.y + seed * 2.0) * (size.y / 2.0)) * 0.75)) * 2.0 - 1.0;
    float noiseY = ((frac(1.0 - (coord.x + seed * 3.0) * (size.x / 2.0)) * 0.75) + (frac((coord.y + seed * 4.0) * (size.y / 2.0)) * 0.25)) * 2.0 - 1.0;
    return float2(noiseX, noiseY);
}

float lenSq(float3 v)
{
    return pow(v.x, 2.0) + pow(v.y, 2.0) + pow(v.z, 2.0);
}

float3 ReconstructPositionFromDepth(in float2 uv, in float4x4 inverseViewProj, in float2 offset = float2(0,0))
{
    // improved normal reconstructed from Wicked engine.
    // https://wickedengine.net/2019/09/22/improved-normal-reconstruction-from-depth/
    float x = uv.x * 2.f - 1.f;
    float y = (1.f - uv.y) * 2.f - 1.f;
    float depth = u_DepthTexture.SampleLevel(WrapSampler, uv, 0, int2(offset)).r;
    float4 positionS = float4(x, y, depth, 1.f);
    float4 positionV = mul(inverseViewProj, positionS);
    float3 pos = positionV.xyz / positionV.w;
    return pos / 1.f;
}

float3 GetNormal(in float2 uv, in float4x4 inverseViewProj)
{
    float xSize = resolution.x;
    float ySize = resolution.y;
    
    float3 p1 = ReconstructPositionFromDepth(uv + float2(xSize, 0.0f), inverseViewProj).xyz;
    float3 p2 = ReconstructPositionFromDepth(uv + float2(0.f, ySize), inverseViewProj).xyz;
    float3 p3 = ReconstructPositionFromDepth(uv + float2(-xSize, 0.0f), inverseViewProj).xyz;
    float3 p4 = ReconstructPositionFromDepth(uv + float2(0.f, -ySize), inverseViewProj).xyz;
    
    float3 vPoint = ReconstructPositionFromDepth(uv, inverseViewProj, float2(0,0));
    
    float3 dx = vPoint - p1;
    float3 dy = p2 - vPoint;
    float3 dx2 = p3 - vPoint;
    float3 dy2 = vPoint - p4;
    
    if (length(dx2) < length(dx) && uv.x - xSize >= 0.0 || uv.x + xSize > 1.f)
    {
        dx = dx2;
    }
    
    if (length(dy2) < length(dy) && uv.y - ySize >= 0.0 || uv.y + ySize > 1.f)
    {
        dy = dy2;
    }
    
    return normalize(-cross(dx, dy).xyz);
}


float3 lightSample(float2 coord, float4x4 ipm, float2 lightcoord, float3 normal, float3 position, float n, float2 texsize)
{
    // https://github.com/demonixis/SSGI-URP/blob/master/Shaders/ssgi.shader
    
    
    float2 random = float2(1.0, 1.0);

    if (true)
    {
        random = (mod_dither3((coord * texsize) + float2(n * 82.294, n * 127.721))) * 0.01 * 2;
    }
    else
    {
        random = dither(coord, 1.0, texsize) * 0.1 * 2;
    }

    lightcoord *= float2(0.7, 0.7);

                //light absolute data
    float3 lightcolor = u_ColorTexture.SampleLevel(WrapSampler, ((lightcoord) + random), 0).rgb;
    float3 lightnormal = GetNormal(frac(lightcoord) + random, ipm).rgb;
    float3 lightposition = ReconstructPositionFromDepth(frac(lightcoord) + random, ipm, float2(0,0)).xyz;

                //light variable data
    float3 lightpath = lightposition - position;
    float3 lightdir = normalize(lightpath);

                //falloff calculations
    float cosemit = clamp(dot(lightdir, -lightnormal), 0.0, 1.0); //emit only in one direction
    float coscatch = clamp(dot(lightdir, normal) * 0.5 + 0.5, 0.0, 1.0); //recieve light from one direction
    float distfall = pow(lenSq(lightpath), 0.1) + 1.0; //fall off with distance

    return (lightcolor * cosemit * coscatch / distfall) * (length(lightposition) / 20.0);
}

float3 CalculateViewPositions(float2 texCoords)
{
    float depth = u_DepthTexture.SampleLevel(ClampSampler, texCoords, 0.f).x;
    
    float x = texCoords.x * 2.f - 1.f;
    float y = (1.f - texCoords.y) * 2.f - 1.f;

    float4 screenPos = float4(x, y, depth, 1.f);
    float4 viewSpace = mul(inverse(toProjection), screenPos);
    
    viewSpace /= viewSpace.w;
    return viewSpace.xyz;
}
float AspectRatio()
{
    return resolution.x / resolution.y;
}
float2 CalculateViewUV(float3 viewPosition)
{
    float3 norm = viewPosition / viewPosition.z;
    float2 view = float2((norm.x + 1.0) / 2.f, ((norm.y * -1.f) * AspectRatio() + 1.f) / 2.f);
    return view;
}

float LinearizeDepth01(const float screenDepth)
{
    return (nearPlane * farPlane / (farPlane + screenDepth * (nearPlane - farPlane))) / farPlane;
}

// will return the uv coord the ray march hit.
float2 RayMarchDepth(in float3 startPos, in float3 direction, in float2 startUV, in float stepSize, in int stepCount, out int steps)
{
    const float epilon = 0.01f;
    float3 currentPosition = startPos;
    float2 hitUv = float2(-1, -1);
    steps = 0;
    for (int i = 0; i < stepCount; ++i)
    {
        const float3 newStepPos = (currentPosition + direction) * stepSize; // view space
        
        float2 newUV = CalculateViewUV(newStepPos);
       
        float newDepth = u_DepthTexture.SampleLevel(ClampSampler, newUV, 0).r;
        
        steps++;
        float linearStep = newStepPos.z / farPlane;
        float delta = (newDepth) - newStepPos.z;
        if ((newDepth) < newStepPos.z)
        {
            hitUv = newUV;
            break;
        }
        //if(delta > 0 && delta < 1.f)
        //{
        //    hitUv = newUV;
        //    break;
        //}
       
      
        currentPosition = newStepPos;
        
    }
    return hitUv;
}


float4 main(VertextoPixel input) : SV_Target
{
    float3 directColor = u_ColorTexture.SampleLevel(ClampSampler, input.uv, 0).rgb;
    float3 color = normalize(directColor).rgb;
    float3 indirectColor = 0.f;
    
    float depth = u_DepthTexture.SampleLevel(ClampSampler, input.uv, 0).r;
    if ((depth < 1.f) == false)
    {
        return float4(directColor, 1.f);
    }
    
    const int samplecount = 64;
    const float pi = 3.14159;
    
    float4x4 inverseProj = inverse(toProjection);
    
    float3 viewPos = ReconstructPositionFromDepth(input.uv, inverseProj, float2(0, 0));
    float3 viewNormal = GetNormal(input.uv, inverseProj);
    
    float dlong = pi * (3.f - sqrt(5.f));
    float dz = 1.f / float(samplecount);
    float l = 0.f;
    float z = 1.f - dz / 2.0f;
    
    
    
    //[unroll(samplecount)]
    //for (int i = 0; i < samplecount; ++i)
    //{
    //    float radius = sqrt(1.f - z);
    //    float xPoint = (cos(l) * radius) * 0.5f + 0.5f;
    //    float yPoint = (sin(l) * radius) * 0.5f + 0.5f;

    //    z = z - dz;
    //    l = l + dlong;
        
    //    indirectColor += lightSample(input.uv, inverseProj, float2(xPoint, yPoint), viewNormal, viewPos, float(i), resolution);
    //}

    return float4(directColor + (indirectColor / float(samplecount) * 2.f), 1.f);

}