#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "Mesh.h"
class Effect;
class Texture;


//-----------------------------------------------------
// Mesh Class									
//-----------------------------------------------------
class MeshOpaque final : public Mesh
{
public:
	MeshOpaque(ID3D11Device* pDevice, const std::string& filename, Texture* pDiffuseMap, Texture* pNormalMap, Texture* pSpecularMap, Texture* pGlossinessMap);
	~MeshOpaque() = default;

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	MeshOpaque(const MeshOpaque& other) = delete;
	MeshOpaque(MeshOpaque&& other) noexcept = delete;
	MeshOpaque& operator=(const MeshOpaque& other) = delete;
	MeshOpaque& operator=(MeshOpaque&& other)	noexcept = delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	virtual void SoftwareRender(int width, int height, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels, float* pDepthBufferPixels) override;
	virtual void PrintTypeName() override;

	void SetDiffuseMap(Texture* pDiffuseMap);
	void SetNormalMap(Texture* pNormalMap);
	void SetSpecularMap(Texture* pSpecularMap);
	void SetGlossinessMap(Texture* pGlossinessMap);
};