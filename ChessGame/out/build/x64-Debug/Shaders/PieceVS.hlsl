struct VSInput {
    float2 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 color : COLOR;
};

struct VSOutput {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 color : COLOR;
};

VSOutput main(VSInput input) {
    VSOutput output;
    float2 ndc = float2(input.pos.x / 4.0f - 1.0f, 1.0f - input.pos.y / 4.0f);
    output.pos = float4(ndc, 0.0f, 1.0f);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}
