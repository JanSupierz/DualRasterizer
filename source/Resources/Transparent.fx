//---------------------------------------------------------------------------------
//      Globals
//---------------------------------------------------------------------------------

float gPI = 3.14159f;
float gLightIntensity = 7.f;
float gShininess = 25.f;
float3 gAmbient = float3(0.025f, 0.025f, 0.025f);
float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);

float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap    : DiffuseMap;

//---------------------------------------------------------------------------------
//      States
//---------------------------------------------------------------------------------

SamplerState gSamplerState : Sampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};

RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockwise = false; //default
};

BlendState gBlendState
{
    BlendEnable[0] = true;
    SrcBlend = src_alpha;
    DestBlend = inv_src_alpha;
    BlendOp = add;
    SrcBlendAlpha = zero;
    DestBlendAlpha = zero;
    BlendOpAlpha = add;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;

    //Others are redundant, because StencilEnable is FALSE
    StencilReadMask = 0x0F;
    StencilWriteMask = 0x0F;

    FrontFaceStencilFunc = always;
    BackFaceStencilFunc = always;

    FrontFaceStencilDepthFail = keep;
    BackFaceStencilDepthFail = keep;

    FrontFaceStencilPass = keep;
    BackFaceStencilPass = keep;

    FrontFaceStencilFail = keep;
    BackFaceStencilFail = keep;
};

//---------------------------------------------------------------------------------
//      Input/Output Structs
//---------------------------------------------------------------------------------

struct VS_INPUT
{
    float3 Position : POSITION;
    float2 UV		: TEXCOORD;
    float3 Normal	: NORMAL;
    float3 Tangent	: TANGENT;
};

struct VS_OUTPUT
{
    float4 Position		 : SV_POSITION;
    float2 UV			 : TEXCOORD;
};

//---------------------------------------------------------------------------------
//      Vertex Shader
//---------------------------------------------------------------------------------

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.UV = input.UV;

    return output;
}

//---------------------------------------------------------------------------------
//      Pixel Shader
//---------------------------------------------------------------------------------

float4 Diffuse(float kd, float4 sampledDiffuse)
{
	return kd * sampledDiffuse / gPI;
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    return gDiffuseMap.Sample(gSamplerState, input.UV);
}

//---------------------------------------------------------------------------------
//      Technique
//---------------------------------------------------------------------------------

technique11 DefaultTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);

        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}