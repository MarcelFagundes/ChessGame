struct VSInput {
    float2 pos : POSITION;
    float4 color : COLOR;
};

struct VSOutput {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VSOutput main(VSInput input) {
    VSOutput output;
    // O espaco do tabuleiro vai de 0 a 8 em X e Y. Convertemos para NDC
    // (-1 a 1), invertendo Y porque em NDC ele cresce para cima.
    float2 ndc = float2(input.pos.x / 4.0f - 1.0f, 1.0f - input.pos.y / 4.0f);
    output.pos = float4(ndc, 0.0f, 1.0f);
    output.color = input.color;
    return output;
}
