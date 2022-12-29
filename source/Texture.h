#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------


//-----------------------------------------------------
// Texture Class									
//-----------------------------------------------------

class Texture final
{
public:
	Texture() = default;
	Texture(ID3D11Device* pDevice, const std::string& path);
	virtual ~Texture();

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	Texture(const Texture& other) = delete;
	Texture(Texture&& other) noexcept = delete;
	Texture& operator=(const Texture& other) = delete;
	Texture& operator=(Texture&& other)	noexcept = delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	ID3D11ShaderResourceView* GetResource() const;

	dae::ColorRGB Sample(const dae::Vector2& uv) const;
	dae::Vector4 SampleRGBA(const dae::Vector2& uv) const;
private:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	Uint8* m_pRed, * m_pGreen, * m_pBlue, * m_pAlpha;

	ID3D11Texture2D* m_pResource{ nullptr };
	ID3D11ShaderResourceView* m_pSRV{ nullptr };

	SDL_Surface* m_pSurface{ nullptr };
	uint32_t* m_pSurfacePixels{ nullptr };
};

