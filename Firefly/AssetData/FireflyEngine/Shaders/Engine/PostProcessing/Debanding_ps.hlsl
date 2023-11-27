#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/PostProcessStructs.hlsli>

// thanks to Zoe Thysell Tga21

//Book of shaders on randomness https://thebookofshaders.com/10/
//Function from https://simonharris.co/making-a-noise-film-grain-post-processing-effect-from-scratch-in-threejs/
float Noise(float2 p)
{
    float2 K1 = float2(
        23.14069263277926, // e^pi (Gelfond's constant)
        2.665144142690225 // 2^sqrt(2) (Gelfond–Schneider constant)
        );
    return frac(cos(dot(p, K1)) * 12345.6789);
}

float RandomNoise(float2 aTexCoord, float aAmount, float aStrength)
{
    float2 random = Noise(aTexCoord);
    random.y *= Noise(float2(random.y, aAmount));
    return Noise(random) * aStrength;
}


Texture2D framebuffer : register(t0);
float4 main(VertextoPixel input) : SV_Target
{
    float4 Color = framebuffer.Sample(ClampSampler, input.uv);

    float noiseStrength = 0.005;
    Color.rgb += RandomNoise(input.uv, input.position.x, noiseStrength);
    return Color;
}