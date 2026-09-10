Texture2D atlasTexture : register(t0);
SamplerState atlasSampler : register(s0);

struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 color : COLOR;
};

float4 main(PSInput input) : SV_TARGET {
    // O canal alfa do atlas guarda a "cobertura" da letra (0 = fundo,
    // 1 = traco). A cor vem do vertice (branco ou preto, definida no
    // PieceRenderer), permitindo reusar o mesmo atlas para as duas cores.
    float alpha = atlasTexture.Sample(atlasSampler, input.uv).a;
    return float4(input.color, alpha);
}
