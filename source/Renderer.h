#pragma once
struct SDL_Window;
struct SDL_Surface;
struct Vertex;

class Sampler;
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

	private:
		void SoftwareRender() const;

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

		//Meshes
		std::unique_ptr<MeshOpaque> m_pVehicleMesh;
		std::unique_ptr<MeshTransparent> m_pFireMesh;
		bool m_ShouldRotate{ true };

		//Camera
		std::unique_ptr<Camera> m_pCamera;

		//Textures
		std::unique_ptr<Texture> m_pDiffuseMap;
		std::unique_ptr<Texture> m_pNormalMap;
		std::unique_ptr<Texture> m_pSpecularMap;
		std::unique_ptr<Texture> m_pGlossinessMap;

		std::unique_ptr<Texture> m_pFireDiffuseMap;

		//Sampling
		std::unique_ptr<Sampler> m_pSampler;

		enum class FilteringMethod { Point, Linear, Anisotropic };
		FilteringMethod m_FilteringMethod{ FilteringMethod::Point };

		//Key-bindings
		bool m_IsSoftware{ true };
	};
}
