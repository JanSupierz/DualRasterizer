#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "Effect.h"
struct Vertex;
struct VertexOut;

//-----------------------------------------------------
// EffectUV Class									
//-----------------------------------------------------

class EffectTransparent final : public Effect
{
public:
	EffectTransparent(ID3D11Device* pDevice, const std::wstring& assetFile);
	~EffectTransparent();

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	EffectTransparent(const EffectTransparent& other) = delete;
	EffectTransparent(EffectTransparent&& other) noexcept = delete;
	EffectTransparent& operator=(const EffectTransparent& other) = delete;
	EffectTransparent& operator=(EffectTransparent&& other)	noexcept = delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void VertexTransformationFunction(const std::vector<Vertex>& vertices, std::vector<VertexOut>& verticesOut, const std::vector<uint32_t>& indices);
	void PixelShading(const VertexOut& v, int width, int height, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const;

	void SetDiffuseMap(Texture* pDiffuseTexture);

private:

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;

	Texture* m_pDiffuseMap{};

	//Transparency

	Uint8* m_pRed{};
	Uint8* m_pGreen{};
	Uint8* m_pBlue{};

	const float m_BlendFactor{ 0.9f };
};


