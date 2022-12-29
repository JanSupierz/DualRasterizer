//---------------------------
// Includes
//---------------------------
#include "pch.h"
#include "Mesh.h"
#include "Texture.h"
#include <iostream>
#include "Utils.h"
#include "Effect.h"
#include "Camera.h"

//---------------------------
// Constructor & Destructor
//---------------------------

Mesh::Mesh(ID3D11Device* pDevice, const std::string& filename)
{
	dae::Utils::ParseOBJ(filename, m_Vertices, m_Indices);
	m_VerticesOut.resize(m_Vertices.size());

	//Create Vertex Buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(m_Vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_Vertices.data();

	HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result)) return;

	//Create Index Buffer
	m_NumIndices = static_cast<uint32_t>(m_Indices.size());

	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	initData.pSysMem = m_Indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result)) return;
}

Mesh::~Mesh()
{
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
	}

	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
	}
}

//---------------------------
// Member functions
//---------------------------

void Mesh::Render(ID3D11DeviceContext* pDeviceContext)
{
	//1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pEffect->GetInputLayout());

	//3. Set VertexBuffer
	constexpr UINT stride{ sizeof(Vertex) };
	constexpr UINT offset{ 0 };
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//4. Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pEffect->GetTechnique()->GetDesc(&techDesc);

	for (UINT p{}; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

void Mesh::SetSamplerState(ID3D11SamplerState* pSamplerState)
{
	m_pEffect->SetSamplerState(pSamplerState);
}

void Mesh::SetMatrices(dae::Camera* pCamera)
{
	m_pEffect->SetMatrices(pCamera, m_WorldMatrix);
}

void Mesh::Translate(const dae::Vector3& translation)
{
	m_WorldMatrix = dae::Matrix::CreateTranslation(m_WorldMatrix.GetTranslation() + translation);
}

void Mesh::RotateY(float angle)
{
	m_WorldMatrix = dae::Matrix::CreateRotationY(angle) * m_WorldMatrix;
}


