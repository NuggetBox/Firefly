#include <includes/ConstStructs.hlsli>
#include <includes/Math.hlsli>
#include <includes/ShadowFunctions.hlsli>
#include <Engine/AtmosphericSky/SkyAtmosphereCommon.hlsl>
#include <Includes/VolumetricFogCommon.hlsli>
#include <includes/PBRFunctions.hlsli>

struct PixelInput
{
    float4 position : SV_POSITION;
    float4 worldPosition : POSITION;
    float3 RotatedNormal : RotNormal;
    float2 texcoord0 : UV0;
    float2 texcoord1 : UV1;
    float2 texcoord2 : UV2;
    float2 texcoord3 : UV3;
    float4 color0 : COLOR0;
    float4 color1 : COLOR1;
    float4 color2 : COLOR2;
    float4 color3 : COLOR3;
    float3 normal : NORMAL;
    float3x3 tangentBias : TBASIS;
    float entityTime : EntityTime;
};

Texture2DArray DirShadow_Engine : register(t20);

Texture2D SpotShadow_Engine[8] : register(t21);

TextureCube env_Engine : register(t30);

TextureCube PointShadow_Engine[8] : register(t31);

Texture2D TransmittanceTexture_Engine : register(t40);
Texture3D<float4> indirectLight_Engine : register(t50);


float3 InscatteredLight(float3 color, float3 worldPos, float3 atmosphereCol)
{
    float3 uv = WorldToUv(worldPos, nearPlane, 500000, FogSettings.z, mul(toProjection, toView));

    if (uv.z < 0.1)
    {
        uv.z = 0.1;
    }
    
    float4 scatteredLight = tex3DTricubic(indirectLight_Engine, ClampSampler, uv, (float3(FROXEL_GRID_SIZE_X, FROXEL_GRID_SIZE_Y, FROXEL_GRID_SIZE_Z)));

    //scatteredLight = indirectLight_Engine.Load(int4(float3(uv.x * FROXEL_GRID_SIZE_X, uv.y * FROXEL_GRID_SIZE_Y, uv.z * FROXEL_GRID_SIZE_Z), 0));
    
    float transmittance = scatteredLight.a;
    
    return color * transmittance + scatteredLight.rgb * atmosphereCol;
}

float3 CalcPointlight(uint j, float3 worldPosition, float3 diffuseColor, float3 specularColor, float3 normal, float roughness, float3 toEye)
{
    float3 pixelToLight = worldPosition.xyz - pointLight[j].position;
        
        
        
    float shadow = 1.f;
    if (pointLight[j].customValues.w > 0)
    {
            
        switch (int(pointLight[j].customValues.z))
        {
            case 0:
                shadow = PointHardShadows(PointShadow_Engine[0], PointSampler, pixelToLight, pointLight[j].radius);
                break;
            case 1:
                shadow = PointHardShadows(PointShadow_Engine[1], PointSampler, pixelToLight, pointLight[j].radius);
                break;
            case 2:
                shadow = PointHardShadows(PointShadow_Engine[2], PointSampler, pixelToLight, pointLight[j].radius);
                break;
            case 3:
                shadow = PointHardShadows(PointShadow_Engine[3], PointSampler, pixelToLight, pointLight[j].radius);
                break;
            case 4:
                shadow = PointHardShadows(PointShadow_Engine[4], PointSampler, pixelToLight, pointLight[j].radius);
                break;
            case 5:
                shadow = PointHardShadows(PointShadow_Engine[5], PointSampler, pixelToLight, pointLight[j].radius);
                break;
            case 6:
                shadow = PointHardShadows(PointShadow_Engine[6], PointSampler, pixelToLight, pointLight[j].radius);
                break;
            case 7:
                shadow = PointHardShadows(PointShadow_Engine[7], PointSampler, pixelToLight, pointLight[j].radius);
                break;
        }
    }
        
        
    return EvaluatePointLight(
        diffuseColor,
        specularColor,
        normal,
        roughness,
        pointLight[j].colorAndIntensity.rgb,
        pointLight[j].colorAndIntensity.a,
        pointLight[j].radius,
        pointLight[j].position,
        worldPosition.xyz,
        toEye) * shadow;
}

float3 CalcSpotlight(uint k, float3 worldPosition, float3 diffuseColor, float3 specularColor, float3 normal, float roughness, float3 toEye)
{
    float4 pos = mul(spotData[k].transform, float4(worldPosition, 1.f));
    float shadow = 1.f;
    if (spotData[k].spotInfo.w > 0)
    {
        switch (int(spotData[k].direction.w))
        {
            case 0:
                shadow = SpotSoftShadows(SpotShadow_Engine[0], PointSampler, pos, normal, spotData[k].direction.xyz, spotData[k].spotInfo.x);
                break;
            case 1:
                shadow = SpotSoftShadows(SpotShadow_Engine[1], PointSampler, pos, normal, spotData[k].direction.xyz, spotData[k].spotInfo.x);
                break;
            case 2:
                shadow = SpotSoftShadows(SpotShadow_Engine[2], PointSampler, pos, normal, spotData[k].direction.xyz, spotData[k].spotInfo.x);
                break;
            case 3:
                shadow = SpotSoftShadows(SpotShadow_Engine[3], PointSampler, pos, normal, spotData[k].direction.xyz, spotData[k].spotInfo.x);
                break;
            case 4:
                shadow = SpotSoftShadows(SpotShadow_Engine[4], PointSampler, pos, normal, spotData[k].direction.xyz, spotData[k].spotInfo.x);
                break;
            case 5:
                shadow = SpotSoftShadows(SpotShadow_Engine[5], PointSampler, pos, normal, spotData[k].direction.xyz, spotData[k].spotInfo.x);
                break;
            case 6:
                shadow = SpotSoftShadows(SpotShadow_Engine[6], PointSampler, pos, normal, spotData[k].direction.xyz, spotData[k].spotInfo.x);
                break;
            case 7:
                shadow = SpotSoftShadows(SpotShadow_Engine[7], PointSampler, pos, normal, spotData[k].direction.xyz, spotData[k].spotInfo.x);
                break;
        }
    }
        
    return EvaluateSpotLight(
        diffuseColor,
        specularColor,
        normal,
        roughness,
        spotData[k].colorAndIntensity.rgb,
        spotData[k].colorAndIntensity.a,
        spotData[k].spotInfo.x,
        spotData[k].position.xyz,
        spotData[k].direction.xyz,
        spotData[k].spotInfo.z,
        spotData[k].spotInfo.y,
        toEye,
        worldPosition.xyz) * shadow;
}

float3 CalculatePBR(float3 albedo, float3 normal, float roughness, float metalness, float4 worldPosition, float3 vertexNormals, float AO, float3 emission)
{
    float3 directColor = 0;
    const float3 toEye = normalize(cameraPosition.xyz - worldPosition.xyz);
    const float3 specularColor = lerp((float3) 0.04, albedo.xyz, metalness);
    const float3 diffuseColor = lerp((float3) 0.0, albedo.xyz, 1.0f - metalness);
    AtmosphereParameters Atmosphere = GetAtmosphereParameters();
    float4 ueWorldPos = float4(worldPosition.z / 1000.f, worldPosition.x / 1000.f, worldPosition.y / 1000.f, worldPosition.w);
    float3 P0 = ueWorldPos.xyz / ueWorldPos.w + float3(0, 0, Atmosphere.BottomRadius);
    float viewHeight = length(P0);
    const float3 UpVector = P0 / viewHeight;
    float viewZenithCosAngle = dot(sun_direction, UpVector);
    float2 uv;
    LutTransmittanceParamsToUv(Atmosphere, viewHeight, viewZenithCosAngle, uv);
    const float3 newLightCol = TransmittanceTexture_Engine.SampleLevel(ClampSampler, uv, 0).xyz;
    
    float3 newLightDir = normalize(-dirLight[0].direction.xyz);
       

        //float light_occlusion = 1 - saturate(dot(float4(-newLightDir, 1), occ));
    directColor += EvaluateDirectionalLight(
	    diffuseColor,
	    specularColor,
	    normal,
	    roughness,
	    dirLight[0].colorAndIntensity.rgb * newLightCol /** light_occlusion*/,
	    dirLight[0].colorAndIntensity.a,
		

		-newLightDir, toEye);
        
    float shadow;
    float4 pos;
    if (dirLight[0].dirLightInfo.x > 0)
    {
        float4 viewPos = mul(toView, worldPosition);
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
            layer = 4;
        }
            
        pos = mul(dirLight[layer].projMatrix, mul(dirLight[layer].viewMatrix, worldPosition));
        float3 col = 0;
        if (dirLight[0].dirLightInfo.y > 0)
        {
            shadow = DirectionalSoftShadows(DirShadow_Engine, PointSampler, pos, normal, -newLightDir, layer, dirLight[layer].direction.w);
        }
        else
        {
            shadow = 1 - DirectionalHardShadows(DirShadow_Engine, PointSampler, pos, normal, -newLightDir, layer, dirLight[layer].direction.w);
        }
            
        directColor *= shadow;
    }

    float debugShadow = 0;
    for (int j = 0; j < pointCount.x; j += 4)
    {
      
        directColor += CalcPointlight(j + 0, worldPosition.xyz, diffuseColor, specularColor, normal, roughness, toEye);
        directColor += CalcPointlight(j + 1, worldPosition.xyz, diffuseColor, specularColor, normal, roughness, toEye);
        directColor += CalcPointlight(j + 2, worldPosition.xyz, diffuseColor, specularColor, normal, roughness, toEye);
        directColor += CalcPointlight(j + 3, worldPosition.xyz, diffuseColor, specularColor, normal, roughness, toEye);

    }
    
    for (int k = 0; k < spotCount.x; k ++)
    {
        directColor += CalcSpotlight(k + 0, worldPosition.xyz, diffuseColor, specularColor, normal, roughness, toEye);
    }

    float3 ambientLighting = EvaluateAmbience(
	env_Engine,
	normal,
	vertexNormals,
	toEye,
	roughness,
	AO,
	diffuseColor,
	specularColor,
	WrapSampler
    ) * EnvironmentIntensity;
    
    // Second evaluate transmittance due to participating media

    
    float3 emissiveCol = albedo.xyz * emission;
    float3 final = directColor + (ambientLighting * newLightCol) /* * EnvironmentIntensity*/;
    if (renPadd.x > 0)
    {
        final = InscatteredLight(final, worldPosition.xyz, newLightCol);
    }
    return final + emissiveCol;
}


float3 CalculatePixelNormals(float3 normal, PixelInput input)
{
    normal.z = 0;
    normal = 2.0f * normal - 1.0f;
    normal.z = sqrt(1 - saturate(normal.x * normal.x + normal.y * normal.y));

    return normalize(mul(normalize(normal), input.tangentBias));
}

float3 ExtractNormals(Texture2D normalTexture, SamplerState samplertype, float2 UV)
{
    return normalTexture.Sample(samplertype, UV).agb;
}

void ExtractMaterialInfo(float4 materialInfo, out float roughness, out float metalness, out float emissive, out float emissiveStrength)
{
    metalness = materialInfo.r;
    roughness = materialInfo.g;
    emissive = materialInfo.b;
    emissiveStrength = materialInfo.a;
}


float3x3 inverse3x3(float3x3 M)
{
    
    float3x3 M_t = transpose(M);
    float det = dot(cross(M_t[0], M_t[1]), M_t[2]);
    float3x3 adjugate = float3x3(cross(M_t[1], M_t[2]),
                          cross(M_t[2], M_t[0]),
                          cross(M_t[0], M_t[1]));
    return adjugate / det;
}


float3x3 CalculateDecalTBN(float3 worldPosition, float3 viewVector, float2 uv)
{
    const float3 ddxWp = ddx(worldPosition);
    const float3 ddyWp = ddy(worldPosition);

    const float3 normal = normalize(cross(ddyWp, ddxWp));

    const float3 dp1 = ddx(viewVector);
    const float3 dp2 = ddy(viewVector);

    const float2 duv1 = ddx(uv);
    const float2 duv2 = ddy(uv);

    const float3 dp2perp = cross(dp2, normal);
    const float3 dp1perp = cross(normal, dp1);

    const float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    const float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    const float invMax = rsqrt(max(dot(T, T), dot(B, B)));
    const float3x3 TBN = (float3x3(T * invMax, -B * invMax, -1.f * normal));

    return TBN;
}