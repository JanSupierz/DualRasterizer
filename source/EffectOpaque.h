#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "Effect.h"
struct Vertex;
struct VertexOut;

//-----------------------------------------------------
// Effect Class									
//-----------------------------------------------------

class EffectOpaque final : public Effect
{
public:
	EffectOpaque(ID3D11Device* pDevice, const std::wstring& assetFile);
	~EffectOpaque();

	//-------------------------------------------------
	// Copy/move constructors and assignment operators
	//-------------------------------------------------

	EffectOpaque(const EffectOpaque& other) = delete;
	EffectOpaque(EffectOpaque&& other) noexcept = delete;
	EffectOpaque& operator=(const EffectOpaque& other) = delete;
	EffectOpaque& operator=(EffectOpaque&& other)	noexcept = delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void VertexTransformationFunction(const std::vector<Vertex>& vertices, std::vector<VertexOut>& verticesOut, const std::vector<uint32_t>& indices);
	void PixelShading(const VertexOut& v, int width, int height, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels) const;

	virtual void SetMatrices(dae::Camera* pCamera, const dae::Matrix& worldMatrix) override;

	void SetDiffuseMap(Texture* pDiffuseTexture);
	void SetNormalMap(Texture* pNormalTexture);
	void SetSpecularMap(Texture* pSpecularTexture);
	void SetGlossinessMap(Texture* pGlossinessTexture);

private:

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	ID3DX11EffectMatrixVariable* m_pViewInverseMatrixVariable;
	ID3DX11EffectMatrixVariable* m_pWorldMatrixVariable;

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;

	Texture* m_pDiffuseMap{};
	Texture* m_pNormalMap{};
	Texture* m_pSpecularMap{};
	Texture* m_pGlossinessMap{};

	dae::Matrix m_ViewInverseMatrix{};
	dae::Matrix m_WorldMatrix{};
};


