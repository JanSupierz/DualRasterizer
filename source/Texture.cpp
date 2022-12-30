//---------------------------
// Includes
//---------------------------

#include "pch.h"
#include "Texture.h"
#include "assert.h"

//---------------------------
// Constructor & Destructor
//---------------------------

Texture::Texture(ID3D11Device* pDevice, const std::string& path)
	:m_pSurface{ IMG_Load(path.c_str()) }
{
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = m_pSurface->w;
	desc.Height = m_pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);

	HRESULT result = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);
	if (FAILED(result)) assert(false);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	result = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);
	if (FAILED(result)) assert(false);

	m_pSurfacePixels = (uint32_t*)m_pSurface->pixels;
}

Texture::~Texture()
{
	m_pSRV->Release();
	m_pResource->Release();

	SDL_FreeSurface(m_pSurface);
}

//---------------------------
// Member functions
//---------------------------

ID3D11ShaderResourceView* Texture::GetResource() const
{
	return m_pSRV;
}

dae::ColorRGB Texture::SampleRGB(const dae::Vector2& uv) const
{
	Uint8 red{}, green{}, blue{};

	//clamp between 0 and 1
	const float u{ std::clamp(uv.x,0.f,1.f) };
	const float v{ std::clamp(uv.y,0.f,1.f) };

	//Sample the correct texel for the given uv
	SDL_GetRGB(m_pSurfacePixels[static_cast<Uint32>(int(u * m_pSurface->w) + int(v * m_pSurface->h) * m_pSurface->w)], m_pSurface->format, &red, &green, &blue);

	constexpr float division{ 1.f / 255.f };

	return { red * division, green * division, blue * division };
}

dae::Vector4 Texture::SampleRGBA(const dae::Vector2& uv) const
{
	Uint8 red{}, green{}, blue{}, alpha{};

	//clamp between 0 and 1
	const float u{ std::clamp(uv.x,0.f,1.f) };
	const float v{ std::clamp(uv.y,0.f,1.f) };

	//Sample the correct texel for the given uv
	SDL_GetRGBA(m_pSurfacePixels[static_cast<Uint32>(int(u * m_pSurface->w) + int(v * m_pSurface->h) * m_pSurface->w)], m_pSurface->format, &red, &green, &blue, &alpha);

	constexpr float division{ 1.f / 255.f };

	return { red * division, green * division, blue * division, alpha * division };
}