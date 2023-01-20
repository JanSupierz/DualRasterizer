//---------------------------
// Includes
//---------------------------

#include "pch.h"
#include "Rasterizer.h"
#include "assert.h"

//---------------------------
// Constructor & Destructor
//---------------------------

Rasterizer::Rasterizer(ID3D11Device* pDevice)
{
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0.f;
	rasterizerDesc.DepthBiasClamp = 0.f;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	//Back face culling
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	HRESULT result = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pBackFaceCullingRasterizer);
	if (FAILED(result)) assert(false);

	//Front face culling
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	result = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pFrontFaceCullingRasterizer);
	if (FAILED(result)) assert(false);

	//No culling
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	result = pDevice->CreateRasterizerState(&rasterizerDesc, &m_pNoCullingRasterizer);
	if (FAILED(result)) assert(false);
}

Rasterizer::~Rasterizer()
{
	if (m_pNoCullingRasterizer)
	{
		m_pNoCullingRasterizer->Release();
	}

	if (m_pFrontFaceCullingRasterizer)
	{
		m_pFrontFaceCullingRasterizer->Release();
	}

	if (m_pBackFaceCullingRasterizer)
	{
		m_pBackFaceCullingRasterizer->Release();
	}
}

//---------------------------
// Member functions
//---------------------------

ID3D11RasterizerState* Rasterizer::GetRasterizerState(D3D11_CULL_MODE cullMode) const
{
	switch (cullMode)
	{
	case D3D11_CULL_BACK:
		return m_pBackFaceCullingRasterizer;
		break;
	case D3D11_CULL_FRONT:
		return m_pFrontFaceCullingRasterizer;
		break;
	default:
		return m_pNoCullingRasterizer;
		break;
	}
}
