//---------------------------
// Includes
//---------------------------
#include "pch.h"
#include "MeshTransparent.h"
#include "Texture.h"
#include "EffectTransparent.h"
#include "Utils.h"

//---------------------------
// Constructor & Destructor
//---------------------------

MeshTransparent::MeshTransparent(ID3D11Device* pDevice, const std::string& filename, Texture* pDiffuseMap)
	:Mesh(pDevice, filename)
{
	PrintTypeName();

	//Effect
	m_pEffect = std::make_unique<EffectTransparent>(pDevice, L"Resources/Transparent.fx");
	m_CullMode = CullMode::NoCulling;

	SetDiffuseMap(pDiffuseMap);
}

//---------------------------
// Member functions
//---------------------------
void MeshTransparent::SoftwareRender(int width, int height, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels, float* pDepthBufferPixels)
{
	EffectTransparent* pEffect{ static_cast<EffectTransparent*>(m_pEffect.get()) };

	pEffect->VertexTransformationFunction(m_Vertices, m_VerticesOut, m_Indices);

	for (size_t index{}; index < m_Vertices.size(); ++index)
	{
		//NDC space -> Raster space
		m_VerticesOut[index].position.x = 0.5f * (m_VerticesOut[index].position.x + 1.f) * width;
		m_VerticesOut[index].position.y = 0.5f * (1.f - m_VerticesOut[index].position.y) * height;
	}

	for (size_t index{}; index < m_MaxCount; index += m_Increment)
	{
		//Frustrum culling
		if (m_VerticesOut[m_Indices[index]].position.z < 0.f || m_VerticesOut[m_Indices[index]].position.z > 1.f ||
			m_VerticesOut[m_Indices[index + 1]].position.z < 0.f || m_VerticesOut[m_Indices[index + 1]].position.z > 1.f ||
			m_VerticesOut[m_Indices[index + 2]].position.z < 0.f || m_VerticesOut[m_Indices[index + 2]].position.z > 1.f) continue;

		if (m_Indices[index] == m_Indices[index + 1] || m_Indices[index] == m_Indices[index + 2] || m_Indices[index + 1] == m_Indices[index + 2]) continue;

		const dae::Vector2 v0{ m_VerticesOut[m_Indices[index]].position.GetXY() };
		const dae::Vector2 v1{ m_VerticesOut[m_Indices[index + 1]].position.GetXY() };
		const dae::Vector2 v2{ m_VerticesOut[m_Indices[index + 2]].position.GetXY() };

		dae::Vector2 min{ std::min(v0.x, v1.x),std::min(v0.y, v1.y) };
		min.x = std::min(min.x, v2.x);
		min.y = std::min(min.y, v2.y);

		dae::Vector2 max{ std::max(v0.x, v1.x),std::max(v0.y, v1.y) };
		max.x = std::max(max.x, v2.x);
		max.y = std::max(max.y, v2.y);


		//RENDER LOGIC
		for (int px{ std::max(0,static_cast<int>(min.x)) }; px <= std::min(width - 1, static_cast<int>(max.x)); ++px)
		{
			for (int py{ std::max(0,static_cast<int>(min.y)) }; py <= std::min(height - 1, static_cast<int>(max.y)); ++py)
			{
				if (m_ShowBoundingbox)
				{
					pBackBufferPixels[static_cast<int>(px) + (static_cast<int>(py) * width)] = SDL_MapRGB(pBackBuffer->format,
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255));
					continue;
				}

				//Rasterization
				dae::Vector3 vertexRatio{};
				const bool shouldSwap{ !m_IsTriangleList && index & 0x01 };

				if (!dae::Utils::IsPixelInTriangle(dae::Vector2{ static_cast<float>(px),static_cast<float>(py) }, v0, v1, v2, vertexRatio, shouldSwap, m_CullMode)) continue;

				//Attribute Interpolation
				const float currentDepth{ 1.f / ((vertexRatio.x / m_VerticesOut[m_Indices[index]].position.z) + (vertexRatio.y / m_VerticesOut[m_Indices[index + 1]].position.z) + (vertexRatio.z / m_VerticesOut[m_Indices[index + 2]].position.z)) };

				if (currentDepth < pDepthBufferPixels[px + (py * width)])
				{
					if (m_ShowDepth)
					{
						continue;
					}

					const float wInterpolated{ 1.f / ((vertexRatio.x * m_VerticesOut[m_Indices[index]].position.w) + (vertexRatio.y * m_VerticesOut[m_Indices[index + 1]].position.w) + (vertexRatio.z * m_VerticesOut[m_Indices[index + 2]].position.w)) };

					VertexOut pixel
					{
						dae::Vector4//position
						{
							static_cast<float>(px),
							static_cast<float>(py),
							currentDepth,
							wInterpolated
						},
						dae::Vector2 //uv
						{
							(m_Vertices[m_Indices[index]].uv * vertexRatio.x * m_VerticesOut[m_Indices[index]].position.w + m_Vertices[m_Indices[index + 1]].uv * vertexRatio.y * m_VerticesOut[m_Indices[index + 1]].position.w + m_Vertices[m_Indices[index + 2]].uv * vertexRatio.z * m_VerticesOut[m_Indices[index + 2]].position.w) * wInterpolated
						}
					};

					pEffect->PixelShading(pixel, width, height, pBackBuffer, pBackBufferPixels);
				}
			}
		}
	}
}
	

void MeshTransparent::PrintTypeName()
{
	std::cout << "----------------------------\n";
	std::cout << "Transparent Mesh\n";
	std::cout << "----------------------------\n";
}

void MeshTransparent::SetDiffuseMap(Texture* pDiffuseMap)
{
	static_cast<EffectTransparent*>(m_pEffect.get())->SetDiffuseMap(pDiffuseMap);
}