#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
#include "DataTypes.h"
class Effect;
class Texture;
namespace dae
{
	struct Camera;
}

struct Vertex
{
	dae::Vector3 position{};
	dae::Vector2 uv{};
	dae::Vector3 normal{};
	dae::Vector3 tangent{};
};

struct VertexOut
{
	dae::Vector4 position{};
	dae::Vector2 uv{};
	dae::Vector3 normal{};
	dae::Vector3 tangent{};
	dae::Vector3 viewDirection{};
};

//-----------------------------------------------------
// Mesh Class									
//-----------------------------------------------------
class Mesh
{
public:
	Mesh(ID3D11Device* pDevice, const std::string& filePath );
	virtual ~Mesh();

	// -------------------------
	// Copy/move constructors and assignment operators
	// -------------------------    
	Mesh(const Mesh& other) = delete;
	Mesh(Mesh&& other) noexcept = delete;
	Mesh& operator=(const Mesh& other) = delete;
	Mesh& operator=(Mesh&& other)	noexcept = delete;

	//-------------------------------------------------
	// Member functions						
	//-------------------------------------------------
	void Render(ID3D11DeviceContext* pDeviceContext);
	virtual void SoftwareRender(int width, int height, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels, float* pDepthBufferPixels) = 0;

	void SetSamplerState(ID3D11SamplerState* pSamplerState);
	void SetRasterizerState(ID3D11RasterizerState* pRasterizerState);
	void SetMatrices(dae::Camera* pCamera);

	void Translate(const dae::Vector3& translation);
	void RotateY(float angle);

	virtual void PrintTypeName() = 0;

	void SetBoundingBoxVisibitily(bool showBoundingBox);
	void SetDepthVisibility(bool showDepth);
protected:
	//-------------------------------------------------
	// Private member functions								
	//-------------------------------------------------
	
	//-------------------------------------------------
	// Datamembers								
	//-------------------------------------------------

	//Software
	std::vector<Vertex> m_Vertices{};
	std::vector<VertexOut> m_VerticesOut{};
	std::vector<uint32_t> m_Indices{};

	//Direct X
	std::unique_ptr<Effect> m_pEffect;

	uint32_t m_NumIndices;

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	//Other
	dae::Matrix m_WorldMatrix{};
	bool m_ShowBoundingbox{ false };
	bool m_ShowDepth{ false };

	CullMode m_CullMode{CullMode::BackFaceCulling};

	enum class PrimitiveTopology{TriangleList, TriangleStrip};
	PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TriangleList };

	bool m_IsTriangleList{ true };
	int m_Increment{ 1 };
	int m_MaxCount{};
};