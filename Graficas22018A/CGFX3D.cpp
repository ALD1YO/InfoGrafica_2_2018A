#include "stdafx.h"
#include "CGFX3D.h"

D3D11_INPUT_ELEMENT_DESC InputLayout[4] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

CGFX3D::CGFX3D(CDXManager * pOwner)
{
	m_pDXM = pOwner;
	memset(&m_Params, 0, sizeof(PARAMS));
	m_Params.Options.lx = LIGHTING_DIFFUSE; //LIGHTING_DIFFUSE|LIGHTING_ESPECULAR
	m_Params.Projection = Identity();
	m_Params.View = Identity();
	m_Params.World = Identity();
	m_Params.TexVelocity = { 0,0,0,0 };
	m_Params.t = 0.0f;
}

CGFX3D::~CGFX3D()
{
}

bool CGFX3D::Initialize()
{
	ID3DBlob * pVSDXIL = nullptr;
	m_pVS =
		m_pDXM->CompileVS(L"..\\Shaders\\GFX3D.hlsl", "VSMain", &pVSDXIL);
	if (!m_pVS)
		return false;
	m_pDXM->GetDevice()->CreateInputLayout(InputLayout,4,pVSDXIL->GetBufferPointer(),pVSDXIL->GetBufferSize(), &m_pIL);
	pVSDXIL->Release();
	m_pPS = m_pDXM->CompilePS(L"..\\Shaders\\GFX3D.hlsl", "PSMain");
	if (!m_pPS) 
	{
		m_pVS->Release();
		m_pIL->Release();
		return false;
	}
	//Inicializa buffer de constantes para los shaders
	D3D11_BUFFER_DESC dbd;
	memset(&dbd, 0, sizeof(dbd));
	dbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;//Para que quieres usar el buffer
	dbd.ByteWidth = 16 * ((sizeof(PARAMS) + 15) / 16);
	//tamano del constant buffer 16 bytes por comando o multiplos osea que el tamano tiene que ser 16 bytes o multiplos
	//1 float = 4 bytes
	dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dbd.Usage = D3D11_USAGE_DYNAMIC;//Como interactua el canal de comunicacion GPU:READ, CPU:WRITE
	m_pDXM->GetDevice()->CreateBuffer(&dbd, 0, &m_pCB);//Ya se crea el constan buffer falta conectarlo 
	return true;
}

void CGFX3D::Uninitialize()
{
	m_pIL->Release(); m_pIL = nullptr;
	m_pVS->Release(); m_pVS = nullptr;
	m_pPS->Release(); m_pPS = nullptr;
	m_pCB->Release(); m_pCB = nullptr;
}

void CGFX3D::Configure()
{
	auto pCtx = m_pDXM->GetContext();
	pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCtx->IASetInputLayout(m_pIL);
	pCtx->VSSetShader(m_pVS, 0, 0);
	pCtx->VSSetConstantBuffers(0, 1, &m_pCB);
	pCtx->PSSetShader(m_pPS, 0, 0);
	pCtx->PSSetConstantBuffers(0, 1, &m_pCB);
}

void CGFX3D::UpdateCB()
{
	PARAMS Temp = m_Params;
	Temp.World = Transpose(m_Params.World);
	Temp.View = Transpose(m_Params.View);
	Temp.Projection = Transpose(m_Params.Projection);
	Temp.WV = Transpose(m_Params.World*m_Params.View);
	Temp.WVP = Transpose(m_Params.World*m_Params.View*m_Params.Projection);
	for (auto &L : Temp.Lights)
	{
		L.Position = L.Position*m_Params.View;	//Pl Posicion de la luz
		L.Direction = L.Direction*m_Params.View;	//Ld Direccion de la luz cuando es direccional

	}
	D3D11_MAPPED_SUBRESOURCE ms;
	m_pDXM->GetContext()->Map(m_pCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	memcpy(ms.pData,&Temp,sizeof(PARAMS));
	m_pDXM->GetContext()->Unmap(m_pCB, 0);
}

void CGFX3D::Draw(const CMesh * pMesh)
{
	ID3D11Buffer *pIB = nullptr, *pVB = nullptr;
	D3D11_BUFFER_DESC dbd;
	memset(&dbd, 0, sizeof(dbd));
	dbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	dbd.ByteWidth = pMesh->m_V.size() * sizeof(CMesh::VERTEX);
	dbd.Usage = D3D11_USAGE_IMMUTABLE;
	D3D11_SUBRESOURCE_DATA dsd;
	dsd.pSysMem = &pMesh->m_V[0];
	dsd.SysMemPitch = 0;
	dsd.SysMemSlicePitch = 0;
	m_pDXM->GetDevice()->CreateBuffer(&dbd, &dsd, &pVB);
	dbd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	dbd.ByteWidth = sizeof(unsigned long)*pMesh->m_I.size();
	dsd.pSysMem = &pMesh->m_I[0];
	m_pDXM->GetDevice()->CreateBuffer(&dbd, &dsd, &pIB);
	unsigned int Stride = sizeof(CMesh::VERTEX), Offset = 0;
	m_pDXM->GetContext()->IASetVertexBuffers(0, 1, &pVB, &Stride, &Offset);
	m_pDXM->GetContext()->IASetIndexBuffer(pIB, DXGI_FORMAT_R32_UINT, 0);
	m_pDXM->GetContext()->DrawIndexed(pMesh->m_I.size(), 0, 0);
	pIB->Release();
	pVB->Release();
}


