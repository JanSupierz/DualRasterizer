//---------------------------
// Includes
//---------------------------

#include "pch.h"
#include "EffectOpaque.h"
#include "Texture.h"
#include "Camera.h"
#include "Mesh.h"

//---------------------------
// Constructor & Destructor
//---------------------------

EffectOpaque::EffectOpaque(ID3D11Device* pDevice, const std::wstring& assetFile)
	:Effect(pDevice, assetFile)
{
	//-----------------------------------------------------
	// Matrices								
	//-----------------------------------------------------

	m_pWorldMatrixVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();

	if (!m_pWorldMatrixVariable->IsValid())
	{
		std::wcout << L"World matrix not valid\n";
	}

	m_pViewInverseMatrixVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();

	if (!m_pViewInverseMatrixVariable->IsValid())
	{
		std::wcout << L"View inverse matrix not valid\n";
	}

	//-----------------------------------------------------
	// Maps								
	//-----------------------------------------------------

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();

	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"Diffuse map not valid\n";
	}

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();

	if (!m_pNormalMapVariable->IsValid())
	{
		std::wcout << L"Normal map not valid\n";
	}

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();

	if (!m_pSpecularMapVariable->IsValid())
	{
		std::wcout << L"Specular map not valid\n";
	}

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();

	if (!m_pGlossinessMapVariable->IsValid())
	{
		std::wcout << L"Glossiness map not valid\n";
	}
}

EffectOpaque::~EffectOpaque()
{
	//Maps

	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->Release();
	}

	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->Release();
	}

	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->Release();
	}

	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
	}

	//Matrices

	if (m_pViewInverseMatrixVariable)
	{
		m_pViewInverseMatrixVariable->Release();
	}

	if (m_pWorldMatrixVariable)
	{
		m_pWorldMatrixVariable->Release();
	}
}

void EffectOpaque::VertexTransformationFunction(const std::vector<Vertex>& vertices, std::vector<VertexOut>& verticesOut, const std::vector<uint32_t>& indices)
{
	dae::Vector4 position{};

	for (size_t index{}; index < vertices.size(); ++index)
	{
		position = { vertices[index].position.x,vertices[index].position.y,vertices[index].position.z,0.f };
		verticesOut[index].position = m_WorldViewProjectionMatrix.TransformPoint(position);

		const float inverseW{ 1.f / verticesOut[index].position.w };

		//Positions
		verticesOut[index].position.x *= inverseW;
		verticesOut[index].position.y *= inverseW;
		verticesOut[index].position.z *= inverseW;
		verticesOut[index].position.w = inverseW;

		//Normals
		verticesOut[index].normal = m_WorldMatrix.TransformVector(vertices[index].normal).Normalized();
		verticesOut[index].tangent = m_WorldMatrix.TransformVector(vertices[index].tangent).Normalized();

		//View
		verticesOut[index].viewDirection = m_WorldMatrix.TransformPoint(vertices[index].position) - m_ViewInverseMatrix.GetTranslation();
	}
}

void EffectOpaque::PixelShading(const VertexOut& v, int width, int height, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels, bool useNormalMap, RenderMode renderMode) const
{
	dae::ColorRGB finalColor{};

	const dae::Vector3 binominal{ dae::Vector3::Cross(v.normal,v.tangent) };
	const dae::Matrix tangentSpaceAxis{ v.tangent,binominal,v.normal,dae::Vector3::Zero };

	const dae::ColorRGB normalMapSample{ m_pNormalMap->SampleRGB(v.uv) };

	dae::Vector3 sampledNormal{ 2.f * normalMapSample.r - 1.f,2.f * normalMapSample.g - 1.f,2.f * normalMapSample.b - 1.f };
	sampledNormal = (useNormalMap ? tangentSpaceAxis.TransformVector(sampledNormal).Normalized() : v.normal);

	const float observedArea{ dae::Vector3::Dot(sampledNormal,-m_LightDirection) };

	if (observedArea > 0.f)
	{
		const dae::ColorRGB diffuse{ (m_LightIntensity * m_pDiffuseMap->SampleRGB(v.uv)) / static_cast<float>(M_PI) };

		dae::ColorRGB specular{ m_pSpecularMap->SampleRGB(v.uv) * Phong(1.f,m_Shininess * m_pGlossinessMap->SampleRGB(v.uv).r,-m_LightDirection,v.viewDirection,sampledNormal) };

		specular.r = std::clamp(specular.r, 0.f, 1.f);
		specular.g = std::clamp(specular.g, 0.f, 1.f);
		specular.b = std::clamp(specular.b, 0.f, 1.f);

		switch (renderMode)
		{
		case RenderMode::Combined:
			finalColor = observedArea * (diffuse + specular + m_Ambient);
			break;
		case RenderMode::ObservedArea:
			finalColor = { observedArea,observedArea,observedArea };
			break;
		case RenderMode::Diffuse:
			finalColor = observedArea * diffuse;
			break;
		case RenderMode::Specular:
			finalColor = observedArea * specular;
			break;
		}
	}

	//Update Color in Buffer
	finalColor.MaxToOne();

	pBackBufferPixels[static_cast<int>(v.position.x) + (static_cast<int>(v.position.y) * width)] = SDL_MapRGB(pBackBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}

void EffectOpaque::SetMatrices(dae::Camera* pCamera, const dae::Matrix& worldMatrix)
{
	Effect::SetMatrices(pCamera, worldMatrix);

	m_ViewInverseMatrix = pCamera->invViewMatrix;
	m_WorldMatrix = worldMatrix;

	m_pWorldMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&(m_WorldMatrix)));
	m_pViewInverseMatrixVariable->SetMatrix(reinterpret_cast<const float*>(&(m_ViewInverseMatrix)));
}

void EffectOpaque::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable)
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetResource());

	m_pDiffuseMap = pDiffuseTexture;

	std::wcout << L"Diffuse map: OK\n";
}

void EffectOpaque::SetNormalMap(Texture* pNormalTexture)
{
	if (m_pNormalMapVariable)
		m_pNormalMapVariable->SetResource(pNormalTexture->GetResource());

	m_pNormalMap = pNormalTexture;

	std::wcout << L"Normal map: OK\n";
}

void EffectOpaque::SetSpecularMap(Texture* pSpecularTexture)
{
	if (m_pSpecularMapVariable)
		m_pSpecularMapVariable->SetResource(pSpecularTexture->GetResource());

	m_pSpecularMap = pSpecularTexture;

	std::wcout << L"Specular map: OK\n";
}

void EffectOpaque::SetGlossinessMap(Texture* pGlossinessTexture)
{
	if (m_pGlossinessMapVariable)
		m_pGlossinessMapVariable->SetResource(pGlossinessTexture->GetResource());

	m_pGlossinessMap = pGlossinessTexture;

	std::wcout << L"Glossines map: OK\n";
}

float EffectOpaque::Phong(float ks, float exp, const dae::Vector3& l, const dae::Vector3& v, const dae::Vector3 n) const
{		
	return powf(std::max(dae::Vector3::Dot(l - (2.f * std::max(dae::Vector3::Dot(n, l), 0.f) * n), v), 0.f), exp);
}

