//// BRDF Globals
#define FLT_EPSILON 1.192092896e-07f
#define nMipOffset 3
#define PI 3.14159265358979323846f

float2 polarToCartesian(float polar, float2 vec)
{
	float2 vecOut;
	vecOut.x = length(vec)*sin(polar);
	vecOut.y = length(vec)*cos(polar);
	return vecOut;
}

float cartesianToPolar(float2 vec)
{
	return atan2(vec.x,vec.y);
}

float bias(float value, float b)
{
    return (b > 0.0) ? pow(abs(value), log(b) / log(0.5)) : 0.0f;
}

float gain(float value, float g)
{
    return 0.5 * ((value < 0.5) ? bias(2.0 * value, 1.0 - g) : (2.0 - bias(2.0 - 2.0 * value, 1.0 - g)));
}

float RoughnessFromPerceptualRoughness(float perceptualRoughness)
{
    return perceptualRoughness * perceptualRoughness;
}

float PerceptualRougnessFromRoughness(float roughness)
{
    return sqrt(max(0.0, roughness));
}

float SpecularPowerFromPerceptualRoughness(float perceptualRoughness)
{
    float roughness = RoughnessFromPerceptualRoughness(perceptualRoughness);
    return (2.0 / max(FLT_EPSILON, roughness * roughness)) - 2.0;
}

float PerceptualRougnessFromSpecularPower(float specularPower)
{
    float roughness = sqrt(2.0 / (specularPower + 2.0));
    return PerceptualRougnessFromRoughness(roughness);
}

float BurleyToMip(float fPerceptualRoughness, int nMips, float NdotR)
{
    float specPower = SpecularPowerFromPerceptualRoughness(fPerceptualRoughness);
    specPower /= (4 * max(NdotR, FLT_EPSILON));
    float scale = PerceptualRougnessFromSpecularPower(specPower);
    return scale * (nMips - 1 - nMipOffset);
}

float3 GetSpecularDominantDir(float3 vN, float3 vR, float roughness)
{
    float invRough = saturate(1 - roughness);
    float alpha = invRough * (sqrt(invRough) + roughness);

    return lerp(vN, vR, alpha);
}

float GetReductionInMicrofacets(float perceptualRoughness)
{
    float roughness = RoughnessFromPerceptualRoughness(perceptualRoughness);

    return 1.0 / (roughness * roughness + 1.0);
}

float EmpiricalSpecularAO(float ao, float perceptualRoughness)
{
    float smooth = 1 - perceptualRoughness;
    float specAO = gain(ao, 0.5 + max(0.0, smooth * 0.4));

    return min(1.0, specAO + lerp(0.0, 0.5, smooth * smooth * smooth * smooth));
}

float ApproximateSpecularSelfOcclusion(float3 vR, float3 vertNormalNormalized)
{
    const float fadeParam = 1.3;
    float rimmask = clamp(1 + fadeParam * dot(vR, vertNormalNormalized), 0.0, 1.0);
    rimmask *= rimmask;

    return rimmask;
}

float3 Diffuse(float3 pAlbedo)
{
    return pAlbedo / PI;
}

float NormalDistribution_GGX(float a, float NdH)
{
    // Isotropic ggx
    float a2 = a * a;
    float NdH2 = NdH * NdH;

    float denominator = NdH2 * (a2 - 1.0f) + 1.0f;
    denominator *= denominator;
    denominator *= PI;

    return a2 / denominator;
}

float Geometric_Smith_Schlick_GGX(float a, float NdV, float NdL)
{
    // Smith Schlick-GGX
    float k = a * 0.5f;
    float GV = NdV / (NdV * (1 - k) + k);
    float GL = NdL / (NdL * (1 - k) + k);

    return GV * GL;
}

float3 Fresnel_Schlick(float3 specularColor, float3 h, float3 v)
{
    return (specularColor + (1.0f - specularColor) * pow((1.0f - saturate(dot(v, h))), 5));
}

float3 Specular(float3 specularColor, float3 h, float3 v, float a, float NdL, float NdV, float NdH)
{
    return ((NormalDistribution_GGX(a, NdH) * Geometric_Smith_Schlick_GGX(a, NdV, NdL)) * Fresnel_Schlick(specularColor, h, v)) / (4.0f * NdL * NdV + 0.0001f);
}

int GetNumMips(TextureCube map)
{
    int numMips = 0;
    int width = 0;
    int height = 0;
    map.GetDimensions(0, width, height, numMips);
    return numMips;
}

int GetNumMips(Texture2D map)
{
    int numMips = 0;
    int width = 0;
    int height = 0;
    map.GetDimensions(0, width, height, numMips);
    return numMips;
}

float3 EvaluateAmbience(TextureCube lysBurleyCube, float3 pixelNormal, float3 vertexNormal, float3 toEye, float perceptualRoughness, float ao, float3 diffuseColor, float3 specularColor, SamplerState lysBurleyCubeSampler)
{
    float3 specular = 0.0f;
    // Extract the number of Mipmaps that exist in this texture.
    const int numMips = GetNumMips(lysBurleyCube);
    const int nrBrdMips = numMips - nMipOffset;

    // o component in slide 44
    const float VdotN = saturate(dot(toEye, pixelNormal));

    // Original reflection vector half angle.
    const float3 vRorg = 2 * pixelNormal * VdotN - toEye;
    // Modified reflection half angle, h in slide 44.
    const float3 vR = GetSpecularDominantDir(pixelNormal, vRorg, RoughnessFromPerceptualRoughness(perceptualRoughness));
    // Clamp the resulting specular direction to 0 - 1
	const float RdotNsat = saturate(dot(pixelNormal, vR));

    // Obtain the exact mip level for this roughness to fake blurry reflections.
    const float mipLevel = BurleyToMip(perceptualRoughness, numMips, RdotNsat);

    // Specular Radiance from the cubemap
    const float3 specRad = lysBurleyCube.SampleLevel(lysBurleyCubeSampler, vR, mipLevel).xyz;
    // Diffuse Irradiance from the cubemap.
    const float3 diffRad = lysBurleyCube.SampleLevel(lysBurleyCubeSampler, pixelNormal, (float) (nrBrdMips - 1)).xyz;

    // The specular color temper.
    const float fT = 1.0 - RdotNsat;
    float fT5 = fT * fT;
    fT5 = fT5 * fT5 * fT;
    const float3 specCol = lerp(specularColor, (float3) 1.0, fT5);

    // GGX evaluation to find microgeometry scatter in the surface.
    float fFade = GetReductionInMicrofacets(perceptualRoughness);
    fFade *= EmpiricalSpecularAO(ao, perceptualRoughness);
    fFade *= ApproximateSpecularSelfOcclusion(vR, vertexNormal);

    // Assemble.
    const float3 ambientDiffuse = ao * diffuseColor * diffRad;
    const float3 ambientSpecular = fFade * specCol * specRad;
    // IBL BRDF product of Diffuse and Specular light. Slide 9 - 11.
	return (ambientDiffuse + ambientSpecular);
}

float HalfLambert(float3 normal, float3 lightDir)
{
    return sqrt(((dot(normal, lightDir))) * 0.5f + 0.5f);;
}

float3 EvaluateDirectionalLight(float3 albedoColor, float3 specularColor, float3 normal, float roughness, float3 lightColor, float lightIntensity, float3 lightDir, float3 viewDir)
{
    // Computer N dot L, the Lambert Attenuation.
    const float NdL = HalfLambert(normal, lightDir);
    // Compute N dot V, the View Angle.
    const float NdV = HalfLambert(normal, viewDir);
    const float3 h = normalize(lightDir + viewDir);
    const float NdH = HalfLambert(normal, h);
    const float a = max(0.001f, roughness * roughness);

    const float3 cDiff = Diffuse(albedoColor);
    const float3 cSpec = Specular(specularColor, h, viewDir, a, NdL, NdV, NdH);
    
    return saturate(lightColor * NdL * (cDiff * (1.0 - cSpec) + cSpec) * PI) * lightIntensity;
}

float3 EvaluatePointLight(float3 albedoColor, float3 specularColor, float3 normal, float roughness, float3 lightColor, float lightIntensity, float radius, float3 position, float3 camPos, float3 viewDir)
{
    float3 finalColor = 0;
    
    float distFromPixel = length(position.xyz - camPos);
    if (distFromPixel <= radius)
    {
        float3 dirToLight = normalize(position.xyz - camPos);
        float3 halfVector = normalize(dirToLight + (viewDir));

        float attenuation = clamp(1 - distFromPixel / radius, 0.f, 1.f);
        attenuation *= attenuation;

    // hard coded 0.5 falloff on the point light, BAD BAD BAD needs fixing later
        attenuation *= lerp(attenuation, 1.f, 0.5f);
        float ue4Attenuation = ((pow(saturate(1 - pow(distFromPixel / radius, 4.0f)), 2.0f)) / (pow(distFromPixel, 2.0f) + 1)); // Unreal Engine 4 attenuation

        float3 radiance = (lightColor * lightIntensity) * ue4Attenuation;
    
    // Computer N dot L, the Lambert Attenuation.
        const float NdL = HalfLambert(normal, dirToLight);
    // Compute N dot V, the View Angle.
        const float NdV = HalfLambert(normal, viewDir);
        const float3 h = normalize(dirToLight + viewDir);
        const float NdH = HalfLambert(normal, h);
        const float a = max(0.001f, roughness * roughness);

        const float3 cDiff = Diffuse(albedoColor);
        const float3 cSpec = Specular(specularColor, h, viewDir, a, NdL, NdV, NdH);
        finalColor = saturate(lightColor * NdL * (cDiff * (1.0 - cSpec) + cSpec) * PI) * radiance * lightIntensity;
    }
    
    return finalColor;
    
}

float3 EvaluateSpotLight(float3 albedoColor, float3 specularColor, float3 normal,
    float roughness, float3 lightColor, float lightIntensity, float lightRange,
    float3 lightPos, float3 lightDir, float outerAngle, float innerAngle, float3 viewDir, float3 pixelPos)
{
    float3 toLight = lightPos.xyz - pixelPos.xyz;
    float lightDistance = length(toLight);
    toLight = normalize(toLight);

    float NdL = HalfLambert(normal, toLight);
    float lambert = NdL; // Angle attenuation
    float NdV = HalfLambert(normal, viewDir);
    float3 h = normalize(toLight + viewDir);
    float NdH = HalfLambert(normal, h);
    float a = max(0.001f, roughness * roughness);

    float3 cDiff = Diffuse(albedoColor);
    float3 cSpec = Specular(specularColor, h, viewDir, a, NdL, NdV, NdH);

    float cosOuterAngle = cos(outerAngle);
    float cosInnerAngle = cos(innerAngle);
    float3 lightDirection = lightDir;

    // Determine if pixel is within cone.
    float theta = dot(toLight, normalize(-lightDirection));
	// And if we're in the inner or outer radius.
    float epsilon = cosInnerAngle - cosOuterAngle;
    float intensity = clamp((theta - cosOuterAngle) / epsilon, 0.0f, 1.0f);
    intensity *= intensity;
	
    float ue4Attenuation = ((pow(saturate(1 - pow(lightDistance / lightRange, 4.0f)), 2.0f)) / (pow(lightDistance, 2.0f) + 1)); // Unreal Engine 4 attenuation
    float finalAttenuation = lambert * intensity * ue4Attenuation;

    return saturate(lightColor * lightIntensity * lambert * finalAttenuation * ((cDiff * (1.0 - cSpec) + cSpec) * PI));
}

float3 SRGBToLinear(in float3 color)
{
    float3 linearRGBLo = color / 12.92f;
    float3 linearRGBHi = pow((color + 0.055f) / 1.055f, 2.4f);
    float3 linearRGB = (color <= 0.04045f) ? linearRGBLo : linearRGBHi;

    return linearRGB;
}

float3 LinearToSRGB(in float3 color)
{
    float3 sRGBLo = color * 12.92f;
    float3 sRGBHi = pow(abs(color), 1.f / 2.4f) * 1.055f - 0.055f;
    float3 sRGB = (color <= 0.0031308f) ? sRGBLo : sRGBHi;

    return sRGB;
}

float3 LinearToGamma(float3 aColor)
{
    return pow(abs(aColor), 1.0 / 2.2);
}

float3 GammaToLinear(float3 aColor)
{
    return pow(abs(aColor), 2.2);
}

float3 ACESTonemap(float3 color)
{
    float3x3 m1 = float3x3(
		0.59719, 0.07600, 0.02840,
		0.35458, 0.90834, 0.13383,
		0.04823, 0.01566, 0.83777
	);
    float3x3 m2 = float3x3(
		1.60475, -0.10208, -0.00327,
		-0.53108, 1.10813, -0.07276,
		-0.07367, -0.00605, 1.07602
	);
    float3 v = mul(color, m1);
    float3 a = v * (v + 0.0245786) - 0.000090537;
    float3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return clamp(mul((a / b), m2), 0.0, 1.0);
}

