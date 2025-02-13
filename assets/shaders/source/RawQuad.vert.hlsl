struct Input
{
    uint VertexIndex : SV_VertexID;
};

struct Output
{
    float2 UV : TEXCOORD0;
    float4 Position : SV_Position;
};

Output main(Input input)
{
    Output output;
    float3 pos;
    float2 uv;

    switch (input.VertexIndex)
    {
        case 0:
            pos = float3(-1.0f, 1.0f, 0.0f);  // Top left
            uv = float2(-1.0f, 1.0f);
            break;
        case 1:
            pos = float3(-1.0f, -1.0f, 0.0f);  // Bottom left
            uv = float2(-1.0f, -1.0f);
            break;
        case 2:
            pos = float3(1.0f, 1.0f, 0.0f);  // Top right
            uv = float2(1.0f, 1.0f);
            break;
        case 3:
            pos = float3(1.0f, -1.0f, 0.0f);  // Bottom right
            uv = float2(1.0f, -1.0f);
            break;
        default:
            pos = float3(0.0f, 0.0f, 0.0f);
            uv = float2(0.0f, 0.0f);
            break;
    }

    output.Position = float4(pos, 1.0f);
    output.UV = uv;
    return output;
}