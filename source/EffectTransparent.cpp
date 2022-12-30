//---------------------------
// Includes
//---------------------------

#include "pch.h"
#include "EffectTransparent.h"
#include <assert.h>
#include "Camera.h"
#include "Texture.h"
#include "Mesh.h"

//---------------------------
// Constructor & Destructor
//---------------------------

EffectTransparent::EffectTransparent(ID3D11Device* pDevice, const std::wstring& assetFile)
	:Effect(pDevice, assetFile)
{
	//-----------------------------------------------------
	// Transparency								
	//-----------------------------------------------------

	m_pRed = new Uint8;
	m_pGreen = new Uint8;
	m_pBlue = new Uint8;

	//-----------------------------------------------------
	// Maps								
	//-----------------------------------------------------

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();

	if (!m_pDiffuseMapVariable->IsValid())
	{
		std::wcout << L"Diffuse map not valid\n";
	}
}

EffectTransparent::~EffectTransparent()
{
	//Maps

	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
	}

	//Transparency

	delete m_pRed;
	delete m_pGreen;
	delete m_pBlue;
}

void EffectTransparent::VertexTransformationFunction(const std::vector<Vertex>& vertices, std::vector<VertexOut>& verticesOut, const std::vector<uint32_t>& indices)
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
	}
}

void EffectTransparent::PixelShading(const VertexOut& v, int width, int height, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const
{
	//Sample the cololr from texture
	dae::Vector4 sample{ m_pDiffuseMap->SampleRGBA(v.uv) };

	//Sample the color from screen
	SDL_GetRGB(pBackBufferPixels[static_cast<int>(v.position.x) + (static_cast<int>(v.position.y) * width)], pBackBuffer->format, m_pRed, m_pGreen, m_pBlue);

	//Calculate blended color
	const float inverseAlpha{ 1.f - sample.w };
	const float division{ 1 / 255.f };

	dae::ColorRGB finalColor
	{ 
		sample.x * sample.w + *m_pRed * inverseAlpha * division,
		sample.y * sample.w + *m_pGreen * inverseAlpha * division,
		sample.z * sample.w + *m_pBlue * inverseAlpha * division
	};

	//Set color
	finalColor.MaxToOne();

	pBackBufferPixels[static_cast<int>(v.position.x) + (static_cast<int>(v.position.y) * width)] = SDL_MapRGB(pBackBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255.f),
		static_cast<uint8_t>(finalColor.g * 255.f),
		static_cast<uint8_t>(finalColor.b * 255.f));
}

void EffectTransparent::SetDiffuseMap(Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable)
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetResource());

	m_pDiffuseMap = pDiffuseTexture;

	std::wcout << L"Diffuse map: OK\n";
}

