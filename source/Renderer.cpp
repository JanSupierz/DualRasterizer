#include "pch.h"
#include "Renderer.h"
#include "MeshOpaque.h"
#include "MeshTransparent.h"
#include "Texture.h"
#include "Sampler.h"
#include "Camera.h"
#include "Utils.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Create Buffers (Software)
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		m_pDepthBufferPixels = new float[m_Width * m_Height];

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();

		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		//Initialize Camera
		m_pCamera = std::make_unique<Camera>();
		m_pCamera->Initialize(45.f, { 0.f,0.f,-50.f }, m_Width / static_cast<float>(m_Height));

		m_pDiffuseMap = std::make_unique<Texture>(m_pDevice, "Resources/vehicle_diffuse.png");
		m_pNormalMap = std::make_unique<Texture>(m_pDevice, "Resources/vehicle_normal.png");
		m_pSpecularMap = std::make_unique<Texture>(m_pDevice, "Resources/vehicle_specular.png");
		m_pGlossinessMap = std::make_unique<Texture>(m_pDevice, "Resources/vehicle_gloss.png");

		m_pFireDiffuseMap = std::make_unique<Texture>(m_pDevice, "Resources/fireFX_diffuse.png");

		m_pSampler = std::make_unique<Sampler>(m_pDevice);

		//Initialize Meshes

		//Opaque
		m_pVehicleMesh = std::make_unique<MeshOpaque>(m_pDevice, "Resources/vehicle.obj", m_pDiffuseMap.get(), m_pNormalMap.get(), m_pSpecularMap.get(), m_pGlossinessMap.get());

		m_pVehicleMesh->SetMatrices(m_pCamera.get());
		m_pVehicleMesh->SetSamplerState(m_pSampler->GetSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT));

		//Transparent
		m_pFireMesh = std::make_unique<MeshTransparent>(m_pDevice, "Resources/fireFX.obj", m_pFireDiffuseMap.get());

		m_pFireMesh->SetMatrices(m_pCamera.get());
		m_pFireMesh->SetSamplerState(m_pSampler->GetSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT));

		SetBackColor();
	}

	Renderer::~Renderer()
	{
		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
		}

		if (m_pRenderTargetBuffer)
		{
			m_pRenderTargetBuffer->Release();
		}

		if (m_pDepthStencilView)
		{
			m_pDepthStencilView->Release();
		}

		if (m_pDepthStencilBuffer)
		{
			m_pDepthStencilBuffer->Release();
		}

		if (m_pSwapChain)
		{
			m_pSwapChain->Release();
		}

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		if (m_pDevice)
		{
			m_pDevice->Release();
		}

		delete[] m_pDepthBufferPixels;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_pCamera->Update(pTimer);

		if (m_ShouldRotate)
		{
			const float angle{ pTimer->GetElapsed() };

			m_pVehicleMesh->RotateY(angle);
			m_pFireMesh->RotateY(angle);
		}

		m_pVehicleMesh->SetMatrices(m_pCamera.get());
		m_pFireMesh->SetMatrices(m_pCamera.get());
	}


	void Renderer::Render() const
	{
		if (m_IsSoftware)
		{
			//Lock BackBuffer
			SDL_LockSurface(m_pBackBuffer);
			
			//Clear Depth Buffer
			const int nrPixels{ m_Width * m_Height };
			std::fill_n(m_pDepthBufferPixels, nrPixels, INFINITY);

			SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, static_cast<Uint8>(m_BackColor.r * 255.f), static_cast<Uint8>(m_BackColor.g * 255.f), static_cast<Uint8>(m_BackColor.b * 255.f)));

			//DrawCalls
			m_pVehicleMesh->SoftwareRender(m_Width, m_Height, m_pBackBuffer, m_pBackBufferPixels, m_pDepthBufferPixels);

			if (m_ShowFireMesh)
			{
				m_pFireMesh->SoftwareRender(m_Width, m_Height, m_pBackBuffer, m_pBackBufferPixels, m_pDepthBufferPixels);
			}

			if (m_ShowDepth)
			{
				for (int px{ 0 }; px <= m_Width - 1; ++px)
				{
					for (int py{ 0 }; py <= m_Height - 1; ++py)
					{
						const float remappedDepth{ 255.f * dae::Remap(m_pDepthBufferPixels[static_cast<int>(px) + (static_cast<int>(py) * m_Width)],0.995f) };

						m_pBackBufferPixels[static_cast<int>(px) + (static_cast<int>(py) * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(remappedDepth),
							static_cast<uint8_t>(remappedDepth),
							static_cast<uint8_t>(remappedDepth));
					}
				}
			}

			//Update SDL Surface
			SDL_UnlockSurface(m_pBackBuffer);
			SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);

			SDL_UpdateWindowSurface(m_pWindow);
		}
		else if(m_IsInitialized)
		{
			//1. Clear RTV & DSV
			m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &m_BackColor.r);
			m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

			//2. Set Pipeline + Invoke DrawCalls
			m_pVehicleMesh->Render(m_pDeviceContext);

			if (m_ShowFireMesh)
			{
				m_pFireMesh->Render(m_pDeviceContext);
			}

			//3. Present Backbuffer (Swap)
			m_pSwapChain->Present(0, 0);
		}
		else
		{
			std::cout << "DirectX not initialized!\n";
		}
	}

	void Renderer::ToggleFilteringMethods()
	{


		if (m_FilteringMethod == FilteringMethod::Anisotropic)
		{
			m_FilteringMethod = FilteringMethod::Point;
		}
		else
		{
			m_FilteringMethod = static_cast<FilteringMethod>(static_cast<int>(m_FilteringMethod) + 1);
		}

		ID3D11SamplerState* samplerState{};

		std::cout << "----------------------------\n";
		std::cout << "HARDWARE: ";

		switch (m_FilteringMethod)
		{
		case FilteringMethod::Point:
			std::cout << "POINT FILTERING\n";
			samplerState = m_pSampler->GetSamplerState(D3D11_FILTER_MIN_MAG_MIP_POINT);
			break;
		case FilteringMethod::Linear:
			std::cout << "LINEAR FILTERING\n";
			samplerState = m_pSampler->GetSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR);
			break;
		default:
		case FilteringMethod::Anisotropic:
			std::cout << "ANISOTROPIC FILTERING\n";
			samplerState = m_pSampler->GetSamplerState(D3D11_FILTER_ANISOTROPIC);
			break;
		}

		std::cout << "----------------------------\n";

		m_pVehicleMesh->SetSamplerState(samplerState);
		m_pFireMesh->SetSamplerState(samplerState);
	}

	void Renderer::ToggleRotation()
	{
		m_ShouldRotate = !m_ShouldRotate;

		std::cout << "----------------------------\n";
		std::cout << "ROTATION: " << (m_ShouldRotate ? "ON" : "OFF") << '\n';
		std::cout << "----------------------------\n";
	}

	void Renderer::ToggleVersion()
	{
		m_IsSoftware = !m_IsSoftware;

		std::cout << "----------------------------\n";
		std::cout << "VERSION: " << (m_IsSoftware ? "SOFTWARE" : "HARDWARE") << '\n';
		std::cout << "----------------------------\n";

		SetBackColor();
	}

	void Renderer::ToggleCullMode()
	{
		if (m_CullMode == CullMode::NoCulling)
		{
			m_CullMode = CullMode::BackFaceCulling;
		}
		else
		{
			m_CullMode = static_cast<CullMode>(static_cast<int>(m_CullMode) + 1);
		}

		std::cout << "----------------------------\n";

		switch (m_CullMode)
		{
		case CullMode::FrontFaceCulling:
			std::cout << "FRONT FACE CULLING\n";
			break;
		case CullMode::BackFaceCulling:
			std::cout << "BACK FACE CULLING\n";
			break;
		case CullMode::NoCulling:
			std::cout << "NO CULLING\n";
			break;
		}

		std::cout << "----------------------------\n";

		//Only the vehicle has a cull mode that can be changed
		m_pVehicleMesh->SetCullMode(m_CullMode);
	}

	void Renderer::ToggleUniformClearColor()
	{
		m_IsUniformBackground = !m_IsUniformBackground;

		std::cout << "----------------------------\n";
		std::cout << "UNIFORM BACKGROUND: " << (m_IsUniformBackground ? "ON" : "OFF") << '\n';
		std::cout << "----------------------------\n";

		SetBackColor();
	}

	void Renderer::ToggleFireMesh()
	{
		m_ShowFireMesh = !m_ShowFireMesh;

		std::cout << "----------------------------\n";
		std::cout << "FIRE FX: " << (m_ShowFireMesh ? "ON" : "OFF") << '\n';
		std::cout << "----------------------------\n";
	}

	void Renderer::ToggleUseNormalMap()
	{
		m_UseNormalMap = !m_UseNormalMap;

		std::cout << "----------------------------\n";
		std::cout << "SOFTWARE: NORMAL MAP: " << (m_UseNormalMap ? "ON" : "OFF") << '\n';
		std::cout << "----------------------------\n";

		m_pVehicleMesh->SetUseNormalMap(m_UseNormalMap);
	}

	void Renderer::ToggleBoundingBoxVisualization()
	{
		m_ShowBoundingBox = !m_ShowBoundingBox;

		std::cout << "----------------------------\n";
		std::cout << "SOFTWARE: BOUNDING BOX VISIBLE: " << (m_ShowBoundingBox ? "ON" : "OFF") << '\n';
		std::cout << "----------------------------\n";

		m_pVehicleMesh->SetBoundingBoxVisibitily(m_ShowBoundingBox);
		m_pFireMesh->SetBoundingBoxVisibitily(m_ShowBoundingBox);
	}

	void Renderer::ToggleDepthBufferVisualization()
	{
		m_ShowDepth = !m_ShowDepth;

		std::cout << "----------------------------\n";
		std::cout << "SOFTWARE: DEPTH BUFFER VISIBLE: " << (m_ShowDepth ? "ON" : "OFF") << '\n';
		std::cout << "----------------------------\n";

		m_pVehicleMesh->SetDepthVisibility(m_ShowDepth);
		m_pFireMesh->SetDepthVisibility(m_ShowDepth);
	}

	void Renderer::ToggleRenderMode()
	{
		if (m_RenderMode == RenderMode::Specular)
		{
			m_RenderMode = RenderMode::Combined;
		}
		else
		{
			m_RenderMode = static_cast<RenderMode>(static_cast<int>(m_RenderMode) + 1);
		}

		std::cout << "----------------------------\n";
		std::cout << "SOFTWARE: SHADING: ";

		switch (m_RenderMode)
		{
		case RenderMode::Combined:
			std::cout << "COMBINED\n";
			break;
		case RenderMode::ObservedArea:
			std::cout << "OBSERVED AREA\n";
			break;
		case RenderMode::Diffuse:
			std::cout << "DIFFUSE\n";
			break;
		case RenderMode::Specular:
			std::cout << "SPECULAR\n";
			break;
		}

		std::cout << "----------------------------\n";

		//Only the vehicle has a render mode that can be changed
		m_pVehicleMesh->SetRenderMode(m_RenderMode);
	}

	void Renderer::SetBackColor()
	{
		if (m_IsUniformBackground)
		{
			m_BackColor = m_DarkGray;
		}
		else if (m_IsSoftware)
		{
			m_BackColor = m_LightGray;
		}
		else
		{
			m_BackColor = m_CornFlowerBlue;
		}
	}

	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device & DeviceContext
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;

	#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
		if(FAILED(result)) return S_FALSE;

		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result)) return S_FALSE;

		//2. Create SwapChain
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);

		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc,&m_pSwapChain);
		if (FAILED(result)) return result;

		//3. Create DepthStencil (DS) & DepthStencilView (DSV)
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		
		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result)) return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result)) return result;
		
		//4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result)) return result;

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result)) return result;

		//5. Bind RTV & DSV to Output Merger Stage
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set Viewport --- Shared screen possible with multiple viewports
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		if (pDxgiFactory)
		{
			pDxgiFactory->Release();
		}

		return result;
	}
}
