#include <Includes/FullscreenPassShader.hlsli>
#include <Includes/ConstStructs.hlsli>


Texture2D colorBuffer : register(t1);

#define ITERATIONS 24
#define QUALITY(x) 4.0 * x
#define SUBPIXEL_QUALITY 1.0
#define TURNOFF 0
float RGBToLuminance(float3 col)
{
    return sqrt(dot(col, float3(0.299, 0.587, 0.114)));
}

float4 main(VertextoPixel input) : SV_Target
{
    float4 color = colorBuffer.Sample(ClampSampler, input.uv);
    if(TURNOFF)
    {
        return color;
    }
    const float lumaCenter = RGBToLuminance(color.rgb);
    
    float2 inverseScreenSize = float2(1.0 / resolution.x, 1.0 / resolution.y);
    
    float lumaDown = RGBToLuminance(colorBuffer.SampleLevel(ClampSampler, input.uv, 0, int2(0, -1)).rgb);
    float lumaUp = RGBToLuminance(colorBuffer.SampleLevel(ClampSampler, input.uv, 0, int2(0, 1)).rgb);
    float lumaLeft = RGBToLuminance(colorBuffer.SampleLevel(ClampSampler, input.uv, 0, int2(-1, 0)).rgb);
    float lumaRight = RGBToLuminance(colorBuffer.SampleLevel(ClampSampler, input.uv, 0, int2(1, 0)).rgb);
    
    float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
    float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));
    
    float lumaRange = lumaMax - lumaMin;
    if (lumaRange < max(0.0312, lumaMax * 0.125))
    {
        return color;
    }
    
    float lumaDownLeft = RGBToLuminance(colorBuffer.SampleLevel(ClampSampler, input.uv, 0, int2(-1, -1)).rgb);
    float lumaUpRight = RGBToLuminance(colorBuffer.SampleLevel(ClampSampler, input.uv, 0, int2(1, 1)).rgb);
    float lumaUpLeft = RGBToLuminance(colorBuffer.SampleLevel(ClampSampler, input.uv, 0, int2(-1, 1)).rgb);
    float lumaDownRight = RGBToLuminance(colorBuffer.SampleLevel(ClampSampler, input.uv, 0, int2(1, -1)).rgb);
    
    
    float lumaDownUp = lumaDown + lumaUp;
    float lumaLeftRight = lumaLeft + lumaRight;
    
    float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
    float lumaDownCorners = lumaDownLeft + lumaDownRight;
    float lumaRightCorners = lumaDownRight + lumaUpRight;
    float lumaUpCorners = lumaUpRight + lumaUpLeft;
    
    float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs(-2.0 * lumaCenter + lumaDownUp) * 2.0 + abs(-2.0 * lumaRight + lumaRightCorners);
    float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 + abs(-2.0 * lumaDown + lumaDownCorners);
    
    bool isHorizontal = (edgeHorizontal >= edgeVertical);
    
    float luma1 = isHorizontal ? lumaDown : lumaLeft;
    float luma2 = isHorizontal ? lumaUp : lumaRight;
    
    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;
    
    bool is1Steepest = abs(gradient1) >= abs(gradient2);
    
    float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));
    
    float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;
    
    float lumaLocalAverage = 0.0f;
    
    if (is1Steepest)
    {
    // Switch the direction
        stepLength = -stepLength;
        lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
    }
    else
    {
        lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
    }
    
    float2 currentUv = input.uv;
    
    if (isHorizontal)
    {
        currentUv.y -= stepLength * 0.5;
    }
    else
    {
        currentUv.x += stepLength * 0.5;
    }
    
    float2 offset = isHorizontal ? float2(inverseScreenSize.x, 0.0) : float2(0.0, inverseScreenSize.y);
    
    float2 uv1 = currentUv - offset;
    float2 uv2 = currentUv + offset;
    
    float lumaEnd1 = RGBToLuminance(colorBuffer.Sample(WrapSampler, uv1).rgb);
    float lumaEnd2 = RGBToLuminance(colorBuffer.Sample(WrapSampler, uv2).rgb);
    
    lumaEnd1 -= lumaLocalAverage;
    lumaEnd2 -= lumaLocalAverage;
    
    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;
    
    bool reachedBoth = reached1 && reached2;
    
    if (!reached1)
    {
        uv1 -= offset;
    }
    else
    {
        uv2 += offset;
    }
    
    if (!reachedBoth)
    {

        [unroll]
        for (int i = 2; i < ITERATIONS; i++)
        {
            if (!reached1)
            {
                lumaEnd1 = RGBToLuminance(colorBuffer.Sample(WrapSampler, uv1).rgb);
                lumaEnd1 = lumaEnd1 - lumaLocalAverage;
            }
            if (!reached2)
            {
                lumaEnd2 = RGBToLuminance(colorBuffer.Sample(WrapSampler, uv2).rgb);
                lumaEnd2 = lumaEnd2 - lumaLocalAverage;
            }
            reached1 = abs(lumaEnd1) >= gradientScaled;
            reached2 = abs(lumaEnd2) >= gradientScaled;
            reachedBoth = reached1 && reached2;

            if (!reached1)
            {
                uv1 -= offset * QUALITY(i);
            }
            if (!reached2)
            {
                uv2 += offset * QUALITY(i);
            }

            if (reachedBoth)
            {
                break;
            }
        }
    }
    float distance1 = isHorizontal ? (input.uv.x - uv1.x) : (input.uv.y - uv1.y);
    float distance2 = isHorizontal ? (uv2.x - input.uv.x) : (uv2.y - input.uv.y);

// In which direction is the extremity of the edge closer ?
    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);

    float edgeThickness = (distance1 + distance2);

    float pixelOffset = -distanceFinal / edgeThickness + 0.5;
    
        
    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;


    bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0) != isLumaCenterSmaller;

    float finalOffset = correctVariation ? pixelOffset : 0.0;
        
    float lumaAverage = (1.0 / 12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
    float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
    float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
    float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

    finalOffset = max(finalOffset, subPixelOffsetFinal);
    float2 finalUv = input.uv;
    if (isHorizontal)
    {
        finalUv.y += finalOffset * stepLength;
    }
    else
    {
        finalUv.x += finalOffset * stepLength;
    }

    
    
// Read the color at the new UV coordinates, and use it.
    float3 finalColor = colorBuffer.Sample(WrapSampler, finalUv).rgb;
    return float4(finalColor.xyz, 1);
}