#include <Includes/ConstStructs.hlsli>
#include <Includes/Math.hlsli>
#include <Includes/VolumetricFogCommon.hlsli>
#include <includes/ShadowFunctions.hlsli>

struct ComputeInputs
{
    uint3 ThreadId : SV_DispatchThreadID;
};

RWTexture3D<float4> Result : register(u0);

Texture2D u_BlueNoise : register(t90);
TextureCube u_PointShadow[8] : register(t21);
Texture2DArray u_CascadeArray : register(t35);
Texture3D<float4> BufferedFroxelVolume : register(t36);

sampler WrapSampler : register(s0);
sampler BorderSampler : register(s1);
sampler MirrorSampler : register(s2);
sampler PointSampler : register(s3);
sampler ClampSampler : register(s4);

float GetNoise(uint3 coords)
{
    int2 noiseCoord = (coords.xy + int2(0, 1) * coords.z * FROXEL_GRID_SIZE_Z) % FROXEL_GRID_SIZE_Z;
    return u_BlueNoise.Load(int3(noiseCoord, 0)).x;
}


void ApplyHeightFog(float3 wpos, inout float density)
{
    density *= exp(-(wpos.y + 1) * 0.0);
}

float exponentialLerp(float x, float y, float s, float a)
{
    float t = pow(a, s);
    return x * (1 - t) + y * t;
}

[numthreads(8, 8, 1)]
void main(ComputeInputs input)
{
    uint3 pixelCoord = input.ThreadId;
    if (pixelCoord.x < FROXEL_GRID_SIZE_X && pixelCoord.y < FROXEL_GRID_SIZE_Y && pixelCoord.z < FROXEL_GRID_SIZE_Z)
    {
        
        float4x4 wP = mul(toProjection, toView);
        float4x4 inverseVP = inverse(mul(toProjection, toView));
        float jitter = (GetNoise(pixelCoord) - 0.5f) * 0.99999f;
        float3 froxelWorldPosition = ToWorld(pixelCoord, nearPlane, 500000, FogSettings.z, inverseVP, jitter);
    
        float3 WO = normalize(cameraPosition.xyz - froxelWorldPosition.xyz);
    
        float froxelDepth = zSliceThickness(pixelCoord.z);
    
        float density = FogSettings.x;
        
    
        float3 dirLightColor = (godRaysColorIntensity.rgb * godRaysColorIntensity.a);
        
        float3 lighting = EnvironmentFogColorIntensity.rgb * EnvironmentFogColorIntensity.w;
    
        float4 viewPos = mul(toView, float4(froxelWorldPosition, 1.f));
        float depthValue = abs(viewPos.z);
        int layer = -1.f;
        for (int layerID = 0; layerID < 5; layerID++)
        {
            if (depthValue < dirLight[layerID].direction.w)
            {
                layer = layerID;
                break;
            }
        }
            
        if (layer == -1)
        {
            layer = 5;
        }
    
    
    
        float4 pos = mul(dirLight[layer].projMatrix, mul(dirLight[layer].viewMatrix, float4(froxelWorldPosition, 1.f)));
    
    
        float shadow = DirectionalHardShadows(u_CascadeArray, PointSampler, pos, float3(0, 1, 0), normalize(dirLight[0].direction.xyz), layer, dirLight[layer].direction.w);
    
        shadow = 1.f - shadow;
    
        if (shadow > 0.1f)
        {
            lighting += shadow * dirLightColor * Phase(WO, -dirLight[0].direction.xyz, -FogSettings.y);
        }
    
        
        for (int pointlightIndex = 0; pointlightIndex < pointCount.x; ++pointlightIndex)
        {
            PointData pdata = pointLight[pointlightIndex];
            float3 ToFroxelFromPointLight = pdata.position - froxelWorldPosition;
            float3 dirtoPoint = froxelWorldPosition - pdata.position;
            float lengthOfLight = length(dirtoPoint) / pdata.radius;
            lengthOfLight = saturate(lengthOfLight);
            float shadow = 1.f;
            if (pdata.customValues.w > 0)
                switch (int(pdata.customValues.z))
                {
                    case 0:
                        shadow = PointHardShadows(u_PointShadow[0], PointSampler, dirtoPoint, pdata.radius);
                        break;
                    case 1:
                        shadow = PointHardShadows(u_PointShadow[1], PointSampler, dirtoPoint, pdata.radius);
                        break;
                    case 2:
                        shadow = PointHardShadows(u_PointShadow[2], PointSampler, dirtoPoint, pdata.radius);
                        break;
                    case 3:
                        shadow = PointHardShadows(u_PointShadow[3], PointSampler, dirtoPoint, pdata.radius);
                        break;
                    case 4:
                        shadow = PointHardShadows(u_PointShadow[4], PointSampler, dirtoPoint, pdata.radius);
                        break;
                    case 5:
                        shadow = PointHardShadows(u_PointShadow[5], PointSampler, dirtoPoint, pdata.radius);
                        break;
                    case 6:
                        shadow = PointHardShadows(u_PointShadow[6], PointSampler, dirtoPoint, pdata.radius);
                        break;
                    case 7:
                        shadow = PointHardShadows(u_PointShadow[7], PointSampler, dirtoPoint, pdata.radius);
                        break;
                }
            
            float normLength = pdata.radius - length(ToFroxelFromPointLight);
            normLength = saturate(normLength);
            
            ToFroxelFromPointLight = normalize(ToFroxelFromPointLight);
            
            float distNorm = dot(ToFroxelFromPointLight, ToFroxelFromPointLight) * pdata.radius;
            
            float att = clamp(10.f / distNorm, 0, 1.f);
            
            lighting += (att * pdata.colorAndIntensity.xyz * pdata.colorAndIntensity.a * (normLength * exponentialLerp(2, 0, 2, lengthOfLight))) * shadow;

        }
        
        for (int spotlightIndex = 0; spotlightIndex < spotCount.x; ++spotlightIndex)
        {
            SpotData sData = spotData[spotlightIndex];
            float3 ToFroxelFromspotLight = froxelWorldPosition - sData.position.xyz;
            float lengthOfLight = length(ToFroxelFromspotLight) / sData.spotInfo.x;
            lengthOfLight = saturate(lengthOfLight);
            float normLength = sData.spotInfo.x - length(ToFroxelFromspotLight);
            normLength = saturate(normLength);
            ToFroxelFromspotLight = normalize(ToFroxelFromspotLight);
            float distNorm = dot(ToFroxelFromspotLight, ToFroxelFromspotLight) * sData.spotInfo.x;
            
            float thea = dot(ToFroxelFromspotLight, sData.direction.xyz) / (length(ToFroxelFromspotLight) * length(-sData.direction.xyz));
            float accepableAngle = cos(sData.spotInfo.z);
            if (thea > accepableAngle)
            {
                float att = clamp(5.f / distNorm, 0, 1.f) ;
                att *= att;
                lighting += ((sData.colorAndIntensity.xyz * sData.colorAndIntensity.w) * att) * (normLength * (exponentialLerp(0.0, 2, 2, 1 - lengthOfLight)));
            }
        }
        
        //ApplyHeightFog(froxelWorldPosition, density);
       
        float4 colorAndDensity = float4(lighting * density, density);
    
        float3 worldPos = ToWorld(pixelCoord, nearPlane, 500000, 1.f, inverseVP, 0.f);
        
        float3 oldUVW = WorldToUv(worldPos, nearPlane, 500000, 1.f, oldViewProjection);
        if (oldUVW.x > 0.f && oldUVW.y > 0.f && oldUVW.z > 0.f)
        {
            if (oldUVW.x < 1.f && oldUVW.y < 1.f && oldUVW.z < 1.f)
            {
                float4 historyColDens = BufferedFroxelVolume.SampleLevel(ClampSampler, oldUVW, 0);
        
                colorAndDensity = lerp(historyColDens, colorAndDensity, 0.01f);
            }
        }
        
        
        Result[pixelCoord] = colorAndDensity;
    }
}