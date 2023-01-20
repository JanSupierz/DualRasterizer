#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------


//-----------------------------------------------------
// Texture Class									
//-----------------------------------------------------

class Rasterizer final
{
public:
	Rasterizer(ID3D11Device* pDevice);
	virtual ~Rasterizer();

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	Rasterizer(const Rasterizer& other) = delete;
	Rasterizer(Rasterizer&& other) noexcept = delete;
	Rasterizer& operator=(const Rasterizer& other) = delete;
	Rasterizer& operator=(Rasterizer&& other)	noexcept = delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	ID3D11RasterizerState* GetRasterizerState(D3D11_CULL_MODE cullMode) const;

private:
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	ID3D11RasterizerState* m_pFrontFaceCullingRasterizer;
	ID3D11RasterizerState* m_pBackFaceCullingRasterizer;
	ID3D11RasterizerState* m_pNoCullingRasterizer;
};