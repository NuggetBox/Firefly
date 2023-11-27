
SamplerComparisonState shadowSampler : register(s5);

static const float biasValue = 0.00005f;
static const float biasValueSpot = 0.00001f;
static const float biasValuePoint = 0.005f;
static const float Half = 0.5f;
float DirectionalHardShadows(Texture2DArray shadowTexture, SamplerState pointSampler, float4 pixelPosLightSpace, float3 normal, float3 dir, int layer, float CascadeFar)
{
    float3 projCoords = pixelPosLightSpace.xyz / pixelPosLightSpace.w;
    projCoords.x = projCoords.x * Half + Half;
    projCoords.y = -projCoords.y * Half + Half;
    float shadow = 0.0f;
    if ((saturate(projCoords.x) == projCoords.x) && (saturate(projCoords.y) == projCoords.y))
    {
        float closetDepth = shadowTexture.SampleLevel(pointSampler, float3(projCoords.xy, float(layer)), 0).r;
        float currentDepth = projCoords.z;
        float biases[5] =
        {
            0.0005f,
            0.0005f,
            0.0005f,
            0.00005f,
            0.00003f
        };
        float bias = max(biases[layer] * (1.0 - dot(normal, dir.xyz)), biases[layer]);
        
        shadow = currentDepth - biasValue > closetDepth ? 1.0 : 0.0;
    }
    if (projCoords.z > 1.f)
    {
        shadow = 1.f;
    }
    
    return shadow;
}


float DirectionalHardShadows(Texture2D shadowTexture, SamplerState pointSampler, float4 pixelPosLightSpace, float3 normal, float3 dir)
{
    float3 projCoords = pixelPosLightSpace.xyz / pixelPosLightSpace.w;
    projCoords.x = projCoords.x * Half + Half;
    projCoords.y = -projCoords.y * Half + Half;
    float shadow = 0.0f;
    if ((saturate(projCoords.x) == projCoords.x) && (saturate(projCoords.y) == projCoords.y))
    {
        float closetDepth = shadowTexture.SampleLevel(pointSampler, projCoords.xy, 0).r;
        float currentDepth = projCoords.z;

        float bias = max(biasValue * (1.0 - dot(normal, dir.xyz)), biasValue);
        shadow = currentDepth - bias > closetDepth ? 1.0 : 0.0;
    }
    
    if(projCoords.z > 1.f)
    {
        shadow = 1.f;
    }
    return shadow;
}

float DirectionalSoftShadows(Texture2DArray shadowTexture, SamplerState pointSampler, float4 pixelPosLightSpace, float3 normal, float3 dir, int layer, float CascadeFar)
{
    float3 projCoords = pixelPosLightSpace.xyz / pixelPosLightSpace.w;
    projCoords.x = projCoords.x * Half + Half;
    projCoords.y = -projCoords.y * Half + Half;
    float shadow = 1.0;
    if ((saturate(projCoords.x) == projCoords.x) && (saturate(projCoords.y) == projCoords.y))
    {
        float biases[5] =
        {
            0.00005f,
            0.00005f,
            0.00005f,
            0.00005f,
            0.00005f
        };
        
        float currentDepth = projCoords.z;

        float bias = max(biases[layer] * (1.f - dot(normal, -dir.xyz)), biases[layer]);
        shadow = 0.0f;
        uint x = 0;
        uint y = 0;
        uint elements = 0;
        shadowTexture.GetDimensions(x, y, elements);
        float2 texelSize = 1.0f / float2(x, y);
        for (float i = -1.5f; i <= 1.5f; i++)
        {
            for (float j = -1.5f; j <= 1.5f; ++j)
            {
                float pcfDepth = shadowTexture.SampleCmpLevelZero(shadowSampler, float3(projCoords.xy + float2(i, j) * texelSize, float(layer)), projCoords.z - bias).r;
               
                shadow += pcfDepth;

            }
        }
        shadow /= 16.f;
    }
    return shadow;
}

float SpotSoftShadows(Texture2D shadowTexture, SamplerState pointSampler, float4 pixelPosLightSpace, float3 normal, float3 dir, float farplane)
{
    float3 projCoords = pixelPosLightSpace.xyz / pixelPosLightSpace.w;
    projCoords.x = projCoords.x * Half + Half;
    projCoords.y = -projCoords.y * Half + Half;
    float shadow = 1.0;
    if ((saturate(projCoords.x) == projCoords.x) && (saturate(projCoords.y) == projCoords.y))
    {
        float currentDepth = projCoords.z;

        float bias = max(biasValueSpot * (1.0 - dot(-normal, dir.xyz)), biasValueSpot);
        //bias *= 100.f / (farplane/* * 0.75f*/);
        shadow = 0.0f;
        uint x = 0;
        uint y = 0;
        shadowTexture.GetDimensions(x, y);
        float2 texelSize = 1.0f / float2(x, y);
        for (float i = -1.5f; i <= 1.5f; i++)
        {
            for (float j = -1.5f; j <= 1.5f; ++j)
            {
                float pcfDepth = shadowTexture.SampleCmpLevelZero(shadowSampler, projCoords.xy + float2(i, j) * texelSize, projCoords.z - bias).r;
               
                shadow += pcfDepth;

            }
        }
        shadow /= 16.f;
    }
    return shadow;
}

float PointHardShadows(in TextureCube shadowTexture, in SamplerState pointSampler, float3 PixelToLight, float farplane)
{
    const float currentDepth = length(PixelToLight);

    float closestDepth = shadowTexture.SampleCmpLevelZero(shadowSampler, PixelToLight, (currentDepth / farplane) - biasValuePoint).r;
    
    //closestDepth *= farplane;
    
    
  
    
    return closestDepth;
}