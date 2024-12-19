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
    float2 pos;
    if (input.VertexIndex == 0 || input.VertexIndex == 4)
    {
        pos = float2(-0.5f, -0.5f);
        output.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        if (input.VertexIndex == 1)
        {
            pos = float2(0.5f, -0.5f);
            output.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
        }
        else
        {
            if (input.VertexIndex == 2 || input.VertexIndex == 3)
            {
                pos = float2(0.5f, 0.5f);
                output.Color = float4(0.0f, 0.0f, 1.0f, 1.0f);
            }
            else {
                pos = float2(-0.5f, 0.5f);
                output.Color = float4(1.0f, 1.0f, 0.0f, 1.0f);
            }
        }
    }
    output.Position = float4(pos, 0.0f, 1.0f);
    return output;
}