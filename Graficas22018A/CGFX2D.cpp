#include "stdafx.h"
#include "CGFX2D.h"

D3D11_INPUT_ELEMENT_DESC CGFX2D::VERTEX2D::InputLayout[3] =
{
	//tipo,indice,formato,slot,desplazamiento, weas iguales,0
	{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD",0, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

CGFX2D::CGFX2D(CDXManager * pOwner)
{
	m_pManager = pOwner;
	m_pIL = nullptr;
	m_pVS = nullptr;
	m_pPS = nullptr;
}

bool CGFX2D::Initialize()
{
	ID3DBlob * pVSDXIL = nullptr;
	m_pVS = 
		m_pManager->CompileVS((wchar_t*)L"..\\Shaders\\GFX2D.hlsl", (char*)"VSMain", &pVSDXIL);
	
	if(!m_pVS) return false;
		
	m_pPS = m_pManager->CompilePS((wchar_t*)L"..\\Shaders\\GFX2D.hlsl", (char*)"PSMain");
	if (!m_pPS)
	{
		m_pVS->Release();
		m_pPS->Release();
		return false;
	}
	HRESULT hr = m_pManager->GetDevice()->CreateInputLayout(VERTEX2D::InputLayout, 3,
		pVSDXIL->GetBufferPointer(), pVSDXIL->GetBufferSize(), &m_pIL
	);
	if(FAILED(hr))
	{
		m_pVS->Release();
		m_pPS->Release();
		return false;
	}
	pVSDXIL->Release();
	//Inicializa buffer de constantes para los shaders
	D3D11_BUFFER_DESC dbd;
	memset(&dbd, 0, sizeof(dbd));
	dbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;//Para que quieres usar el buffer
	dbd.ByteWidth = 16 * ((sizeof(PARAMS) + 15) / 16);
	//tamano del constant buffer 16 bytes por comando o multiplos osea que el tamano tiene que ser 16 bytes o multiplos
	//1 float = 4 bytes
	dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; 
	dbd.Usage = D3D11_USAGE_DYNAMIC;//Como interactua el canal de comunicacion GPU:READ, CPU:WRITE
	m_pManager->GetDevice()->CreateBuffer(&dbd, NULL, &m_pCB);//Ya se crea el constan buffer falta conectarlo 
	return true;
}

void CGFX2D::Uninitialize()
{
	m_pIL->Release();
	m_pPS->Release();
	m_pVS->Release();
}

void CGFX2D::Draw(VERTEX2D * pVertices, int nVertices)
{
	//1. Transferir vertices al gpu/cpu
	ID3D11Buffer* pVB = nullptr;//Buffer de vertices
	D3D11_BUFFER_DESC dbd;
	memset(&dbd,0, sizeof(dbd));
	dbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	dbd.ByteWidth = sizeof(VERTEX2D)*nVertices;
	//immutable porque solo voy a cargar la info una vez 
	//(no quiero que el cpu altere el contenido del buffer)
	dbd.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA dsd;
	dsd.pSysMem = pVertices;
	dsd.SysMemPitch = 0;
	dsd.SysMemSlicePitch = 0;

	m_pManager->GetDevice()->CreateBuffer(&dbd, &dsd, &pVB);

	//2. Configurar tuberia grafica
	//crea el tipo de primitivas (cada tres de forma un triangulo)
	m_pManager->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//stride distancia que separa cada vertice
	UINT Stride = sizeof(VERTEX2D);
	UINT Offset = 0; //desde el primer byte
	//instala vertex buffer
	m_pManager->GetContext()->IASetVertexBuffers(0, 1, &pVB, &Stride, &Offset);
	m_pManager->GetContext()->IASetInputLayout(m_pIL);
	m_pManager->GetContext()->VSSetShader(m_pVS, 0, 0);
	m_pManager->GetContext()->PSSetShader(m_pPS, 0, 0);

	//Configurar render target
	ID3D11Texture2D* pBackBuffer = nullptr;
	m_pManager->GetSwapChain()->GetBuffer(0,IID_ID3D11Texture2D, (void**)&pBackBuffer);
	D3D11_TEXTURE2D_DESC dtd;
	pBackBuffer->GetDesc(&dtd);
	D3D11_VIEWPORT VP;//acotar regiones de dibujo para el shader
	VP.TopLeftX = 0;
	VP.TopLeftY = 0;
	VP.Width = dtd.Width;
	VP.Height = dtd.Height;
	VP.MinDepth = 0.0f;//no puede ser menor que 0
	VP.MaxDepth = 1.0f;//no puede ser mayor que 1
	m_pManager->GetContext()->RSSetViewports(1, &VP);
	//vista de un recurso como render target
	ID3D11RenderTargetView* pRTV = nullptr;
	m_pManager->GetDevice()->CreateRenderTargetView(pBackBuffer, NULL, &pRTV);
	m_pManager->GetContext()->OMSetRenderTargets(1, &pRTV, NULL);
	//Actualizar el buffer de constantes y atar buffer de constantes
	D3D11_MAPPED_SUBRESOURCE ms;//estructura que hace una zona de memoria en el kernel para poner los datos en el constant buffer
	//el kernel es el unico que puede comunicarse con el hardware, se abre una memory window
	m_pManager->GetContext()->Map(m_pCB,//el constant buffer
		0,//capa por si hay texturas multicapa
		D3D11_MAP_WRITE_DISCARD,//le dices al kernel que haga lo que quiera cuando termine 
		0,
		&ms);
	memcpy(ms.pData, &m_Params, sizeof(PARAMS));
	//cerramos el memory window
	m_pManager->GetContext()->Unmap(m_pCB, 0);
	m_pManager->GetContext()->VSSetConstantBuffers(0,1,&m_pCB);
	m_pManager->GetContext()->PSSetConstantBuffers(0, 1, &m_pCB);
	//3. Dibuja (por finnnn!!!111)
	m_pManager->GetContext()->Draw(nVertices, 0);
	//4. Liberar recursos temporales
	pBackBuffer->Release();
	pVB->Release();
	pRTV->Release();
}

CGFX2D::~CGFX2D()
{
}
