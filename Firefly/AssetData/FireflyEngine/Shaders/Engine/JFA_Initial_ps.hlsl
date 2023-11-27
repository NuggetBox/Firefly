#include <Includes/PBRFunctions.hlsli>
#include <Includes/ConstStructs.hlsli>
#include <Includes/Math.hlsli>
#include <Includes/ShadowFunctions.hlsli>

struct PixelInput
{
    float4 position : SV_POSITION;
};

struct Output
{
    float colorResult : SV_Target0;
};

Output main(PixelInput pInput)
{
    Output outer;

// use this as the color return.
    outer.colorResult = 1.f;

// leave this to be.
    return outer;
}