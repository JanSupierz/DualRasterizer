//---------------------------------------------------------------------------------
//      Globals
//---------------------------------------------------------------------------------

float gPI = 3.14159f;
float gLightIntensity = 7.f;
float gShininess = 25.f;
float3 gAmbient = float3(0.025f, 0.025f, 0.025f);
float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);

float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorld         : World;
float4x4 gViewInverse   : ViewInverse;

Texture2D gDiffuseMap    : DiffuseMap;
Texture2D gNormalMap     : NormalMap;
Texture2D gSpecularMap   : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

//---------------------------------------------------------------------------------
//      Sampler States
//---------------------------------------------------------------------------------

SamplerState gSamplerState : Sampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};

RasterizerState gRasterizerState
{
    CullMode = back;
    FrontCounterClockwise = false; //default
};

BlendState gBlendState
{
};

DepthStencilState gDepthStencilState
{
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
	float4 WorldPosition : WORLD_POSITION;
    float2 UV			 : TEXCOORD;
    float3 Normal		 : NORMAL;
    float3 Tangent		 : TANGENT;
};

//---------------------------------------------------------------------------------
//      Vertex Shader
//---------------------------------------------------------------------------------

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorld);

	output.UV = input.UV;

    output.Normal = normalize(mul(normalize(input.Normal), (float3x3)gWorld));
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorld);

    return output;
}

//---------------------------------------------------------------------------------
//      Pixel Shader
//---------------------------------------------------------------------------------

float4 Diffuse(float kd, float4 sampledDiffuse)
{
	return kd * sampledDiffuse / gPI;
}

float Phong(float ks, float exp, float3 l, float3 v, float3 n)
{
	return ks * pow(saturate(dot(reflect(l, n), v)), exp);
}

float4 PS(VS_OUTPUT input) : SV_TARGET
{
    float3 binormal = cross(input.Normal, input.Tangent);
	float4x4 tangentSpaceAxis = float4x4(float4(input.Tangent, 0.0f), float4(binormal, 0.0f), float4(input.Normal, 0.f), float4(0.f, 0.f, 0.f, 1.f));

	float3 sampledNormal = (2.f * gNormalMap.Sample(gSamplerState, input.UV).rgb) - float3(1.f, 1.f, 1.f);
    sampledNormal = mul(normalize(float4(sampledNormal, 0.f)), tangentSpaceAxis);

	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);

    float observedArea = saturate(dot(sampledNormal, -gLightDirection));

    float3 diffuse = gLightIntensity * Diffuse(1.f, gDiffuseMap.Sample(gSamplerState, input.UV));
    float3 specular = gSpecularMap.Sample(gSamplerState, input.UV) * Phong(1.f, gShininess * gGlossinessMap.Sample(gSamplerState, input.UV).r, -gLightDirection, viewDirection, sampledNormal);
    
    return float4(observedArea * (diffuse + specular + gAmbient), 1.f);
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