#pragma once
//-----------------------------------------------------
// Include Files
//-----------------------------------------------------
class Effect;
class Texture;
namespace dae
{
	struct Camera;
}

struct Vertex
{
	dae::Vector3 position;
	dae::Vector2 uv;
	dae::Vector3 normal;
	dae::Vector3 tangent;
};

struct VertexOut
{
	dae::Vector4 position;
	dae::Vector2 uv;
	dae::Vector3 normal;
	dae::Vector3 tangent;
	dae::Vector3 viewDirection;
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
	void SetMatrices(dae::Camera* pCamera);

	void Translate(const dae::Vector3& translation);
	void RotateY(float angle);

	void VertexTransformationFunction(dae::Camera* pCamera);

	virtual void PrintTypeName() = 0;
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

	//Common
	dae::Matrix m_WorldMatrix{};

};