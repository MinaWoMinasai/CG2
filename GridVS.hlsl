cbuffer CameraBuffer : register(b0)
{
    matrix viewProjection;
}

struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 svPos : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.svPos = mul(viewProjection, float4(input.position, 1.0f));
    output.color = input.color;
    return output;
}