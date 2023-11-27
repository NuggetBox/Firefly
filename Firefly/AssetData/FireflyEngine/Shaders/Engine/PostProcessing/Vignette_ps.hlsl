#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>
#include <Includes/PostProcessStructs.hlsli>

Texture2D colorBuffer : register(t1);


float4 main(VertextoPixel input) : SV_Target
{
    float3 color = outlineColor.xyz;
    float maskRadiusParameter = padding.x;
    float opacityParameter = padding.y;
    float maskGradientIntensity = padding.y;
    //Rendered content on screen
    float3 resource = colorBuffer.SampleLevel(ClampSampler, input.uv, 0).rgb;
    

    float2 UV = (input.uv) * 2 - 1;
    float circle = distance(0, UV);
    float circleAdj = saturate(pow(circle, maskGradientIntensity) * 2 - maskRadiusParameter) * opacityParameter;
    //float circleAdj = saturate(pow(circle, .5) * 2 - 1.5) * .5;
    float3 colour = (circleAdj) * color.xyz;

    //float3 bruh = colour; // * saturate(tex * tex2 * 3);
    colour *= .2; // * middle;

// use this as the color return.
    float4 vignette = float4(colour, circleAdj) + (opacityParameter * maskRadiusParameter * maskGradientIntensity * 0);
    float3 outColour = lerp(resource, vignette.xyz, vignette.a);
    return float4(outColour, 1);

}