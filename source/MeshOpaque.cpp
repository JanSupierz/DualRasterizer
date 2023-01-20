//---------------------------
// Includes
//---------------------------
#include "pch.h"
#include "MeshOpaque.h"
#include "Texture.h"
#include "EffectOpaque.h"
#include "Utils.h"

//---------------------------
// Constructor & Destructor
//---------------------------

MeshOpaque::MeshOpaque(ID3D11Device* pDevice, const std::string& filename, Texture* pDiffuseMap, Texture* pNormalMap, Texture* pSpecularMap, Texture* pGlossinessMap)
	:Mesh(pDevice, filename)
{
	PrintTypeName();

	//Effect
	m_pEffect = std::make_unique<EffectOpaque>(pDevice, L"Resources/Opaque.fx");

	SetDiffuseMap(pDiffuseMap);
	SetNormalMap(pNormalMap);
	SetSpecularMap(pSpecularMap);
	SetGlossinessMap(pGlossinessMap);
}

//---------------------------
// Member functions
//---------------------------

void MeshOpaque::SoftwareRender(int width, int height, SDL_Surface* pBackBuffer, uint32_t* pBackBufferPixels, float* pDepthBufferPixels)
{
	EffectOpaque* pEffect{ static_cast<EffectOpaque*>(m_pEffect.get()) };
	
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

		//Discard triangles where two indices are the same
		if (m_Indices[index] == m_Indices[index + 1] || m_Indices[index] == m_Indices[index + 2] || m_Indices[index + 1] == m_Indices[index + 2]) continue;

		//Vertices
		const dae::Vector2 v0{ m_VerticesOut[m_Indices[index]].position.GetXY() };
		const dae::Vector2 v1{ m_VerticesOut[m_Indices[index + 1]].position.GetXY() };
		const dae::Vector2 v2{ m_VerticesOut[m_Indices[index + 2]].position.GetXY() };

		//Cullmode
		const bool shouldSwap{ !m_IsTriangleList && index & 0x01 };
		const float area{ (shouldSwap ? -1 : 1) * dae::Vector2::Cross(v1 - v0, v2 - v0) };

		if ((m_CullMode == CullMode::FrontFaceCulling && area > 0.f) || (m_CullMode == CullMode::BackFaceCulling && area < 0.f)) continue;

		//Get values for boundingbox
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
				// Boundingbox visualization
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
				if (!dae::Utils::IsPixelInTriangle(dae::Vector2{ static_cast<float>(px),static_cast<float>(py) }, v0, v1, v2, vertexRatio, shouldSwap)) continue;

				//Attribute Interpolation
				const float currentDepth{ 1.f / ((vertexRatio.x / m_VerticesOut[m_Indices[index]].position.z) + (vertexRatio.y / m_VerticesOut[m_Indices[index + 1]].position.z) + (vertexRatio.z / m_VerticesOut[m_Indices[index + 2]].position.z)) };

				if (currentDepth < pDepthBufferPixels[px + (py * width)])
				{
					pDepthBufferPixels[px + (py * width)] = currentDepth;

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
						},
						dae::Vector3 //normal
						{
							((m_VerticesOut[m_Indices[index]].normal * vertexRatio.x * m_VerticesOut[m_Indices[index]].position.w + m_VerticesOut[m_Indices[index + 1]].normal * vertexRatio.y * m_VerticesOut[m_Indices[index + 1]].position.w + m_VerticesOut[m_Indices[index + 2]].normal * vertexRatio.z * m_VerticesOut[m_Indices[index + 2]].position.w) * wInterpolated).Normalized()
						},
						dae::Vector3 //tangent
						{
							((m_VerticesOut[m_Indices[index]].tangent * vertexRatio.x * m_VerticesOut[m_Indices[index]].position.w + m_VerticesOut[m_Indices[index + 1]].tangent * vertexRatio.y * m_VerticesOut[m_Indices[index + 1]].position.w + m_VerticesOut[m_Indices[index + 2]].tangent * vertexRatio.z * m_VerticesOut[m_Indices[index + 2]].position.w) * wInterpolated).Normalized()
						},
						dae::Vector3 //viewDirection
						{
							((m_VerticesOut[m_Indices[index]].viewDirection * vertexRatio.x * m_VerticesOut[m_Indices[index]].position.w + m_VerticesOut[m_Indices[index + 1]].viewDirection * vertexRatio.y * m_VerticesOut[m_Indices[index + 1]].position.w + m_VerticesOut[m_Indices[index + 2]].viewDirection * vertexRatio.z * m_VerticesOut[m_Indices[index + 2]].position.w) * wInterpolated).Normalized()
						}
					};

					pEffect->PixelShading(pixel, width, height, pBackBuffer, pBackBufferPixels, m_UseNormalMap, m_RenderMode);
				}
			}
		}
	}
}

void MeshOpaque::PrintTypeName()
{
	std::cout << "----------------------------\n";
	std::cout << "Opaque Mesh\n";
	std::cout << "----------------------------\n";
}

void MeshOpaque::SetDiffuseMap(Texture* pDiffuseMap)
{
	static_cast<EffectOpaque*>(m_pEffect.get())->SetDiffuseMap(pDiffuseMap);
}

void MeshOpaque::SetNormalMap(Texture* pNormalMap)
{
	static_cast<EffectOpaque*>(m_pEffect.get())->SetNormalMap(pNormalMap);
}

void MeshOpaque::SetSpecularMap(Texture* pSpecularMap)
{
	static_cast<EffectOpaque*>(m_pEffect.get())->SetSpecularMap(pSpecularMap);
}

void MeshOpaque::SetGlossinessMap(Texture* pGlossinessMap)
{
	static_cast<EffectOpaque*>(m_pEffect.get())->SetGlossinessMap(pGlossinessMap);
}

void MeshOpaque::SetCullMode(CullMode cullMode, ID3D11RasterizerState* pRasterizerState )
{
	m_CullMode = cullMode;
	m_pEffect->SetRasterizerState(pRasterizerState);
}

void MeshOpaque::SetUseNormalMap(bool useNormalMap)
{
	m_UseNormalMap = useNormalMap;
}

void MeshOpaque::SetRenderMode(RenderMode renderMode)
{
	m_RenderMode = renderMode;
}


