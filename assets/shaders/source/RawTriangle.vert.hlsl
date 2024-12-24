cbuffer UniformBlock : register(b0, space1)
{
    float4x4 MatrixTransform : packoffset(c0);
};

struct Input
{
    uint VertexIndex : SV_VertexID;
};

struct Output
{
    float4 Color : TEXCOORD0;
    float4 Position : SV_Position;
};

Output main(Input input)
{
    Output output;
    float3 pos;

    switch (input.VertexIndex)
    {
        // Front face
        case 0:
        case 4:
        case 8:
            pos = float3(0.0f, 0.6f, 0.0f);  // Top vertex
            output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        case 1:
        case 5:
        case 9:
            pos = float3(-0.5f, -0.3f, 0.3f);  // Bottom left
            output.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
            break;
        case 2:
        case 6:
        case 10:
            pos = float3(0.5f, -0.3f, 0.3f);  // Bottom right
            output.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
            break;
        case 3:
        case 7:
        case 11:
            pos = float3(0.0f, -0.3f, -0.5f);  // Back vertex
            output.Color = float4(0.0f, 0.0f, 1.0f, 1.0f);
            break;
        default:
            pos = float3(0.0f, 0.0f, 0.0f);
            output.Color = float4(0.0f, 0.0f, 0.0f, 1.0f);
            break;
    }

    output.Position = mul(MatrixTransform, float4(pos, 1.0f));
    return output;
}