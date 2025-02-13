struct Input
{
    uint VertexIndex : SV_VertexID;
};

struct Output
{
    float4 Position : SV_Position;
    float4 Normal : NORMAL;
    float2 UV : TEXCOORD0;
};

cbuffer CameraData : register(b0, space1)
{
    float4x4 CameraView       : packoffset(c0);
    float4x4 CameraProjection : packoffset(c4);
};

Output main(Input input)
{
    Output output;
    float3 pos;
    float3 normal;
    float2 uv;

    switch (input.VertexIndex)
    {
        case 0:
            pos = float3(-1.0f, 1.0f, 0.0f);  // Top left
            normal = float3(0.0f, 0.0f, 1.0f);
            uv = float2(-1.0f, 1.0f);
            break;
        case 1:
            pos = float3(-1.0f, -1.0f, 0.0f);  // Bottom left
            normal = float3(0.0f, 0.0f, 1.0f);
            uv = float2(-1.0f, -1.0f);
            break;
        case 2:
            pos = float3(1.0f, 1.0f, 0.0f);  // Top right
            normal = float3(0.0f, 0.0f, 1.0f);
            uv = float2(1.0f, 1.0f);
            break;
        case 3:
            pos = float3(1.0f, -1.0f, 0.0f);  // Bottom right
            normal = float3(0.0f, 0.0f, 1.0f);
            uv = float2(1.0f, -1.0f);
            break;
        default:
            pos = float3(0.0f, 0.0f, 0.0f);
            normal = float3(0.0f, 0.0f, 1.0f);
            uv = float2(0.0f, 0.0f);
            break;
    }

    output.Position = float4(pos, 1.0f);
    output.Position = mul(output.Position, CameraView);  // Move and rotate to the camera space
    output.Position = mul(output.Position, CameraProjection);  // Project to clip space
    output.Normal = float4(normal, 1.0f);
    output.UV = uv;
    return output;
}