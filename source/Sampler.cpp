//---------------------------
// Includes
//---------------------------

#include "pch.h"
#include "Sampler.h"
#include "assert.h"

//---------------------------
// Constructor & Destructor
//---------------------------

Sampler::Sampler(ID3D11Device* pDevice)
{
	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 16;

	//Point
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	HRESULT result = pDevice->CreateSamplerState(&samplerDesc, &m_pPointSampler);
	if (FAILED(result)) assert(false);

	//Linear
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	result = pDevice->CreateSamplerState(&samplerDesc, &m_pLinearSampler);
	if (FAILED(result)) assert(false);

	//Anisotropic
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	result = pDevice->CreateSamplerState(&samplerDesc, &m_pAnisotropicSampler);
	if (FAILED(result)) assert(false);
}

Sampler::~Sampler()
{
	if (m_pPointSampler)
	{
		m_pPointSampler->Release();
	}

	if (m_pLinearSampler)
	{
		m_pLinearSampler->Release();
	}

	if (m_pAnisotropicSampler)
	{
		m_pAnisotropicSampler->Release();
	}
}

ID3D11SamplerState* Sampler::GetSamplerState(D3D11_FILTER filter) const
{
	switch (filter)
	{	
	case D3D11_FILTER_MIN_MAG_MIP_LINEAR:
		return m_pLinearSampler;
		break;
	case D3D11_FILTER_ANISOTROPIC:
		return m_pAnisotropicSampler;
		break;
	default:
		return m_pPointSampler;
		break;
	}
}

//---------------------------
// Member functions
//---------------------------
