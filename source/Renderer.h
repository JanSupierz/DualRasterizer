#pragma once
#include "DataTypes.h"
struct SDL_Window;
struct SDL_Surface;
struct Vertex;

class Sampler;
class Rasterizer;
class MeshOpaque;
class MeshTransparent;
class Texture;


namespace dae
{
	struct Camera;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		void ToggleFilteringMethods();
		void ToggleRotation();
		void ToggleVersion();
		void ToggleCullMode();
		void ToggleUniformClearColor();
		void ToggleFireMesh();
		void ToggleUseNormalMap();
		void ToggleBoundingBoxVisualization();
		void ToggleDepthBufferVisualization();
		void ToggleRenderMode();

	private:

		////////////////////////////////////////////////////
		//	Helper Functions
		////////////////////////////////////////////////////

		void PrintStartInfo();
		void SetBackColor();

		////////////////////////////////////////////////////
		//	Variables
		////////////////////////////////////////////////////

		//Window
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		//Software
		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		//DIRECTX
		HRESULT InitializeDirectX();
		bool m_IsInitialized{ false };

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;

		IDXGISwapChain* m_pSwapChain;

		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;

		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		//Camera
		std::unique_ptr<Camera> m_pCamera;

		//Textures
		std::unique_ptr<Texture> m_pDiffuseMap;
		std::unique_ptr<Texture> m_pNormalMap;
		std::unique_ptr<Texture> m_pSpecularMap;
		std::unique_ptr<Texture> m_pGlossinessMap;

		std::unique_ptr<Texture> m_pFireDiffuseMap;

		////////////////////////////////////////////////////
		//	Demonstration variables
		////////////////////////////////////////////////////

		bool m_IsSoftware{ false };
		bool m_ShouldRotate{ true };
		bool m_ShowFireMesh{ true };
		bool m_UseNormalMap{ true };
		bool m_ShowBoundingBox{ false };
		bool m_ShowDepth{ false };
		
		//Cull Mode
		CullMode m_CullMode{ CullMode::BackFaceCulling };
		std::unique_ptr<Rasterizer> m_pRasterizer;

		//Render Mode
		RenderMode m_RenderMode{ RenderMode::Combined };

		//Color
		ColorRGB m_BackColor{};
		const ColorRGB m_DarkGray{ 0.1f,0.1f,0.1f };
		const ColorRGB m_CornFlowerBlue{ 0.39f, 0.59f, 0.93f };
		const ColorRGB m_LightGray{ 0.39f, 0.39f, 0.39f };
		bool m_IsUniformBackground{ false };

		//Sampling
		std::unique_ptr<Sampler> m_pSampler;

		enum class FilteringMethod { Point, Linear, Anisotropic };
		FilteringMethod m_FilteringMethod{ FilteringMethod::Point };

		//Meshes
		std::unique_ptr<MeshOpaque> m_pVehicleMesh;
		std::unique_ptr<MeshTransparent> m_pFireMesh;
		const float m_AngularSpeed{ 45.f * static_cast<float>(M_PI) / 180.f };
	};
}
