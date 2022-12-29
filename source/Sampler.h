#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------


//-----------------------------------------------------
// Texture Class									
//-----------------------------------------------------

class Sampler final
{
public:
	Sampler(ID3D11Device* pDevice);
	virtual ~Sampler();

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	Sampler(const Sampler& other) = delete;
	Sampler(Sampler&& other) noexcept = delete;
	Sampler& operator=(const Sampler& other) = delete;
	Sampler& operator=(Sampler&& other)	noexcept = delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	ID3D11SamplerState* GetSamplerState(D3D11_FILTER filter) const;
	
private:
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	ID3D11SamplerState* m_pPointSampler;
	ID3D11SamplerState* m_pLinearSampler;
	ID3D11SamplerState* m_pAnisotropicSampler;
};