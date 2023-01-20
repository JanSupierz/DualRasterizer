//---------------------------
// Includes
//---------------------------

#include "pch.h"
#include "Effect.h"
#include "Texture.h"
#include <assert.h>
#include "Camera.h"

//---------------------------
// Constructor & Destructor
//---------------------------

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile);

	//-----------------------------------------------------
	// Techniques								
	//-----------------------------------------------------

	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");

	if (!m_pTechnique->IsValid())
	{
		std::wcout << L"Technique not valid\n";
	}

	//-----------------------------------------------------
	// Matrices								
	//-----------------------------------------------------

	m_pWorldViewProjectionMatrixVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();

	if (!m_pWorldViewProjectionMatrixVariable->IsValid())
	{
		std::wcout << L"WorldViewProjection matrix not valid\n";
	}

	m_pSamplerStateVariable = m_pEffect->GetVariableByName("gSamplerState")->AsSampler();

	if (!m_pSamplerStateVariable->IsValid())
	{
		std::wcout << L"Sampler state not valid\n";
	}

	m_pRasterizerStateVariable = m_pEffect->GetVariableByName("gRasterizerState")->AsRasterizer();

	if (!m_pRasterizerStateVariable->IsValid())
	{
		std::wcout << L"Rasterizer state not valid\n";
	}

	//-----------------------------------------------------
	// Vertex Layout								
	//-----------------------------------------------------

	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TEXCOORD";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12; //3 x float --- float = 4 bytes
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 20;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 32;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//-----------------------------------------------------
	// Input Layout								
	//-----------------------------------------------------

	D3DX11_PASS_DESC passDesc{};
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	const HRESULT result = pDevice->CreateInputLayout(vertexDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout);
	if (FAILED(result)) assert(false);

}

Effect::~Effect()
{
	if (m_pInputLayout)
	{
		m_pInputLayout->Release();
	}
	
	if (m_pRasterizerStateVariable)
	{
		m_pRasterizerStateVariable->Release();
	}

	if (m_pSamplerStateVariable)
	{
		m_pSamplerStateVariable->Release();
	}
	
	//Matrices

	if (m_pWorldViewProjectionMatrixVariable)
	{
		m_pWorldViewProjectionMatrixVariable->Release();
	}

	if (m_pTechnique)
	{
		m_pTechnique->Release();
	}

	if (m_pEffect)
	{
		m_pEffect->Release();
	}
}

//---------------------------
// Member functions
//---------------------------

ID3DX11Effect* Effect::GetEffect() const
{
	return m_pEffect;
}

ID3DX11EffectTechnique* Effect::GetTechnique() const
{
	return m_pTechnique;
}

ID3D11InputLayout* Effect::GetInputLayout() const
{
	return m_pInputLayout;
}

void Effect::SetMatrices(dae::Camera* pCamera, const dae::Matrix& worldMatrix)
{
	m_WorldViewProjectionMatrix = worldMatrix * pCamera->viewMatrix * pCamera->projectionMatrix;

	m_pWorldViewProjectionMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&(m_WorldViewProjectionMatrix)));
}

void Effect::SetSamplerState(ID3D11SamplerState* pSamplerState)
{
	HRESULT hr{ m_pSamplerStateVariable->SetSampler(0, pSamplerState) };
	if (FAILED(hr)) std::wcout << L"Failed to change sampler state\n";
}

void Effect::SetRasterizerState(ID3D11RasterizerState* pRasterizerState)
{
	HRESULT hr{ m_pRasterizerStateVariable->SetRasterizerState(0,pRasterizerState) };
	if (FAILED(hr)) std::wcout << L"Failed to change rasterizer state\n";
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;

	DWORD shaderFlags{ 0 };

#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(), nullptr, nullptr, shaderFlags, 0, pDevice, &pEffect, &pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for (unsigned int i{}; i < pErrorBlob->GetBufferSize(); ++i)
			{
				ss << pErrors[i];
			}

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << std::endl;

			return nullptr;
		}
	}

	return pEffect;
}
