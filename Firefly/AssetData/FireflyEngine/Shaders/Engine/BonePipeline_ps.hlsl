#include "../../Shaders/includes/FireflyShader.hlsli"

// -------------------------------------- //
// Diffrent varibles that can be accessed //
// -------------------------------------- //

// ----------- Pixel Info --------------- //
// float4 position;                       // This is the pixels position releative to the pivot of the object.
// float4 worldPosition;                  // This is the pixels world position relative to origo.
// float4 color0;                         // This is the vertex color.
// float4 color1;                         // This is the second vertex color.
// float4 color2;                         // This is the third vertex color.
// float4 color3;                         // This is the forth vertex color.
// float2 texcoord0;                      // This is the UV.
// float2 texcoord1;                      // This is the second UV.
// float2 texcoord2;                      // This is the third UV.
// float2 texcoord3;                      // This is the forth UV.
// float3 normal;                         // This is the vertex normal.
// float3x3 tangentBias;                  // This is the normal space matrix, you can use this to get pixel normal.
// float entityTime;      		  // This is the entity time AKA the life time of the object.

// ----------- Camera Info -------------- //
// float4x4 ToView;                       //  this gives you the view matrix of the current camera.
// float4x4 ToProjection;                 //  this gives you the projection matrix of the current camera.
// float4 cameraPosition;                 //  this gives you the position in world space of the current camera.

// ----------- Object Info -------------- //
// int4 entityId;                         //  This is the entity Id of the object this is for the mouse picking just return entityId.x in entityIdResoult.
// float4x4 toWorld;                      //  This gives you the toWorld matrix of the object.
// float4x4 BoneData[128]                 //  This gives you boneData, this is all of the bones that a animated model has.

// ---------- DirLight Info ------------- //
// struct DirData                         //
// {                                      //
//     float4 colorAndIntensity;          //  This is a padded color and intensity of the light (rgb is color, a is intensity)
//     float4 direction;                  //  This is the direction of the directional light. (xyz is direction, a is nothing)
// } dirLight[4];                         //  This struct hold info for 4 directional lights.
// int4 count;                            //  This is the count of lights active (to get count.x)

// ----------- Time Info ---------------- //
// float scaledTotalTime;                 //  This gives you the Ingame TotalTime. (cares about slow-mo)
// float unscaledTotalTime;               //  This gives you the unscaled Ingame TotalTime. (doesn't care about slow-mo)
// float scaledDeltaTime;                 //  This gives you the DeltaTime. (cares about slow-mo)
// float unscaledDeltaTime;               //  This gives you the unscaled DeltaTime. (doesn't care about slow-mo)

// -------- Texture Samplers ------------ //
// sampler WrapSampler;                   //
// sampler BorderSampler;                 //   
// sampler MirrorSampler;                 //  
// sampler PointSampler;                  //    
// sampler ClampSampler;                  // 

struct Output
{
    float4 colorResult : SV_Target0;
    uint2 entityIdResult : SV_Target1;
};


cbuffer MaterialInfo : register(b10)
{
    float4 DEMOCol_Color = float4(1,1,1,1); // With _Color after the name it will be a color variable in the editor.
    float DEMOSlider_Slider = 0.5f; // With _Slider it will have a slider from 0 -> 1 in the editor.
    float DEMO = 1.f; // Nothing special just a value.
}

Output main(PixelInput pInput)
{
    Output outer = (Output)0;
    

// use this as the color return.
     outer.colorResult = DEMOCol_Color * DEMOSlider_Slider + DEMO;

// leave this to be.
    //outer.entityIdResult = entityID.xy;
    return outer;
}