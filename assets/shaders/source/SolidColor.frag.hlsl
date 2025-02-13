/*
cbuffer UniformBlock : register(b0, space3)
{
    float4 AlbedoColor : packoffset(c0);
};
*/

float4 main(float4 Position : SV_Position, float4 Normal : NORMAL, float2 UV: TEXCOORD0) : SV_Target0
{
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}