

RWBuffer<uint> u_GpuDebugLineDispatcher : register(u0);

struct GpuLine
{
    float4 startPos;
    float4 endPos;
    float4 color;
};

StructuredBuffer<GpuLine> u_GpuDebugLines : register(t150);

void DrawLine(GpuLine line)
{
    const uint maxLineCount = 48 * 2048;
    
    uint newSlot = 0;
    InterlockedAdd(u_GpuDebugLineDispatcher[1], 1, newSlot);
    if (newSlot < maxLineCount)
    {
        u_GpuDebugLines[newSlot] = line;
    }
    else
    {
        InterlockedAdd(u_GpuDebugLineDispatcher[1], -1, newSlot);
    }
}

void DrawLine(float3 start, float3 end, float4 color)
{
    GpuLine gpuLine = (GpuLine)0;
    
    gpuLine.startPos = float4(start, 1.f);
    gpuLine.endPos = float4(end, 1.f);
    gpuLine.color = color;
    
    DrawLine(gpuLine);

}

void DrawCross(float3 position, float3 color, float radius)
{
    GpuLine gdl;
    gdl.color = float4(color, 1.0);
    gdl.startPos = float4(position + float3(-radius, 0, 0), 1.0);
    gdl.endPos = float4(position + float3(radius, 0, 0), 1.0);
    DrawLine(gdl);
    gdl.startPos = float4(position + float3(0, -radius, 0), 1.0);
    gdl.endPos = float4(position + float3(0, radius, 0), 1.0);
    DrawLine(gdl);
    gdl.startPos = float4(position + float3(0, 0, -radius), 1.0);
    gdl.endPos = float4(position + float3(0, 0, radius), 1.0);
    DrawLine(gdl);
}