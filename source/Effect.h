#pragma once

//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
class Texture;
namespace dae
{
	struct Camera;
}

//-----------------------------------------------------
// Effect Class									
//-----------------------------------------------------
class Effect
{
public:
	Effect() = default;
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~Effect();

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	Effect(const Effect& other) = delete;
	Effect(Effect&& other) noexcept = delete;
	Effect& operator=(const Effect& other) = delete;
	Effect& operator=(Effect&& other)	noexcept = delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	ID3DX11Effect* GetEffect() const;
	ID3DX11EffectTechnique* GetTechnique() const;
	ID3D11InputLayout* GetInputLayout() const;

	virtual void SetMatrices(dae::Camera* pCamera, const dae::Matrix& worldMatrix);
	void SetSamplerState(ID3D11SamplerState* pSamplerState);

protected:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;
	ID3D11InputLayout* m_pInputLayout;

	ID3DX11EffectMatrixVariable* m_pWorldViewProjectionMatrixVariable;

	ID3DX11EffectSamplerVariable* m_pSamplerStateVariable;

	dae::Matrix m_WorldViewProjectionMatrix{};

	//Shading
	const dae::Vector3 m_LightDirection{ 0.577f,-0.577f,0.577f };
	const float m_LightIntensity{ 7.f };
	const float m_Shininess{ 25.f };
	const dae::ColorRGB m_Ambient{ 0.025f,0.025f,0.025f };
};

