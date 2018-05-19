#include "stdafx.h"
#include "CDXManager.h"

bool CDXManager::Initialize(HWND hWnd, IDXGIAdapter * pAdapter, int nSizeX, int nSizeY)
{
	D3D_FEATURE_LEVEL Requested = D3D_FEATURE_LEVEL_11_0, Created;
	DXGI_SWAP_CHAIN_DESC dscd;
	memset(&dscd, 0, sizeof(dscd));
	dscd.OutputWindow = hWnd;
	dscd.BufferCount = 2;
	dscd.Windowed = true;
	dscd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT|DXGI_USAGE_UNORDERED_ACCESS;
	dscd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dscd.SampleDesc.Count = 1;
	dscd.SampleDesc.Quality = 0;
	dscd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dscd.BufferDesc.Width = nSizeX;
	dscd.BufferDesc.Height = nSizeY;
	dscd.BufferDesc.RefreshRate.Denominator = 0;//0 es decir que use el maximo posible
	dscd.BufferDesc.RefreshRate.Numerator = 0;
	dscd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	dscd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	dscd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	HRESULT hr =
	D3D11CreateDeviceAndSwapChain(pAdapter,
			D3D_DRIVER_TYPE_UNKNOWN,
			NULL,
			0,
			&Requested,
			1,
			D3D11_SDK_VERSION,
			&dscd,
			&m_pSwapChain,
			&m_pDevice,
			&Created,
			&m_pContext
			);
	if (FAILED(hr)) return false;
	//Creacion de los samplers states (configuraciones de samplers)
	D3D11_SAMPLER_DESC dsd;
	memset(&dsd, 0, sizeof(dsd));
	dsd.AddressU = dsd.AddressV = dsd.AddressW =
		D3D11_TEXTURE_ADDRESS_WRAP;
	dsd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; //Trilinear sampler
	dsd.MaxAnisotropy = 1;
	m_pDevice->CreateSamplerState(&dsd, &m_pSamplers[0]);
	
	dsd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	m_pDevice->CreateSamplerState(&dsd, &m_pSamplers[1]);
	return true;
	
}

IDXGIAdapter * CDXManager::EnumAndChooseAdapter(HWND hWnd)
{
	IDXGIFactory* pFactory = nullptr;
	CreateDXGIFactory(IID_IDXGIFactory, (void**)&pFactory);
	int iAdapter = 0;
	IDXGIAdapter* pAdapter = nullptr;
	do {
		pFactory->EnumAdapters(iAdapter, &pAdapter);
		if (!pAdapter) break;
		DXGI_ADAPTER_DESC dad;//El papu de papus
		wchar_t szMessage[512];
		pAdapter->GetDesc(&dad);
		wsprintf(szMessage, 
			L"Descripción:%s\r\nMemoria Dedicada:%d MB\r\nMemoria Compartida:%d MB\r\nMemoria de Sistema:%d MB\r\nID del dispositivo:%d\r\nRevisión:%d",
			dad.Description, dad.DedicatedVideoMemory / (1024 * 1024),
			dad.SharedSystemMemory / (1024 * 1024),
			dad.DedicatedSystemMemory / (1024 * 1024),
			dad.DeviceId,
			dad.Revision
			);
		switch (MessageBox(hWnd, szMessage, L"Selección de adaptador", MB_YESNO | MB_ICONQUESTION))
		{
		case IDYES:
			pFactory->Release();
			return pAdapter;
		case IDNO:
			pAdapter->Release();
			iAdapter++;
			break;
		}
	} while (1);
	
	return pAdapter;
}

#include<d3dcompiler.h>
ID3D11PixelShader * CDXManager::CompilePS(const wchar_t * pszFileName,const char * pszEntryPoint)
{
	//Blob en informatica significa Binary Large Object == Buffer Binario (bytes)
	ID3DBlob* pDXIL = nullptr; //DirectX Intermediate Languaje
	ID3DBlob* pErrors = nullptr;
	HRESULT hr = D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszEntryPoint, "ps_5_0",
		D3DCOMPILE_OPTIMIZATION_LEVEL3|D3DCOMPILE_ENABLE_STRICTNESS, 0, &pDXIL, &pErrors);
	if (pErrors) 
	{
		MessageBoxA(NULL, (char*)pErrors->GetBufferPointer(), "Errores y/o Avisos", MB_ICONERROR);
		pErrors->Release();

	}
	if (pDXIL) 
	{
		ID3D11PixelShader*	pPS = 0;
		//DXIL -> Native Code
		m_pDevice->CreatePixelShader(pDXIL->GetBufferPointer(), 
			pDXIL->GetBufferSize(), 
			NULL,
			&pPS
			);
		pDXIL->Release();
		return pPS;
	}
	return nullptr;
}

ID3D11VertexShader * CDXManager::CompileVS(const wchar_t * pszFileName,const char * pszEntryPoint, ID3DBlob** ppDXIL)
{
	ID3DBlob* pErrors = nullptr;
	HRESULT hr = D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, pszEntryPoint, "vs_5_0",
		D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ENABLE_STRICTNESS, 0, ppDXIL, &pErrors);
	if (pErrors)
	{
		MessageBoxA(NULL, (char*)pErrors->GetBufferPointer(), "Errores y/o Avisos", MB_ICONERROR);
		pErrors->Release();

	}
	if (*ppDXIL)
	{
		ID3D11VertexShader* pVS = nullptr;
		m_pDevice->CreateVertexShader(
			(*ppDXIL)->GetBufferPointer(),
			(*ppDXIL)->GetBufferSize(),
			NULL,
			&pVS );
		return pVS;
	}
	return nullptr;
}

CDXManager::CDXManager()
{
		m_pSwapChain = nullptr;
		m_pDevice = nullptr;
		m_pContext = nullptr;
		m_pt2dBufferZ = nullptr;
		m_pDSV = nullptr;
}


CDXManager::~CDXManager()
{
}
#include<stdio.h>
#include<fstream>
using namespace std;
//LoadTexture by Cornejo: returns a ID3D11Texture2D bindable as Render Target View and Shader Resorce View
ID3D11Texture2D* CDXManager::LoadTexture(
	const char* pszFileName,		//The file name
	unsigned long MipMapLevels, //Number of mipmaps to generate, -1 automatic (int)log2(min(sizeX,sizeY))+1 levels
	ID3D11ShaderResourceView** ppSRV, //The Shader Resource View
	float(*pAlphaFunction)(float r, float g, float b),  //Operates pixel's rgb channels to build an alpha channel (normalized), can be null
	VECTOR4D(*pColorFunction)(const VECTOR4D& Color))
{
	ID3D11Device* pDev = GetDevice();
	ID3D11DeviceContext* pCtx = GetContext();
	printf("Loading %s...\n", pszFileName);
	fstream bitmap(pszFileName, ios::in | ios::binary);
	if (!bitmap.is_open())
	{
		printf("Error: Unable to open file %s\n", pszFileName);
		return NULL;
	}
	//Verificar el numeo magico de un bmp
	BITMAPFILEHEADER bfh;
	bitmap.read((char*)&bfh.bfType, sizeof(bfh.bfType));
	if (!(bfh.bfType == 'MB'))
	{
		printf("Error: Not a DIB File\n");
		return NULL;
	}
	//Leer el resto de los datos
	bitmap.read((char*)&bfh.bfSize, sizeof(bfh) - sizeof(bfh.bfType));

	BITMAPINFOHEADER bih;
	bitmap.read((char*)&bih.biSize, sizeof(bih.biSize));
	if (bih.biSize != sizeof(BITMAPINFOHEADER))
	{
		printf("Error: Unsupported DIB file format.");
		return NULL;
	}
	bitmap.read((char*)&bih.biWidth, sizeof(bih) - sizeof(bih.biSize));
	RGBQUAD Palette[256];
	unsigned long ulRowlenght = 0;
	unsigned char* pBuffer = NULL;

	ID3D11Texture2D* pTemp; //CPU memory
	D3D11_TEXTURE2D_DESC dtd;
	memset(&dtd, 0, sizeof(D3D11_TEXTURE2D_DESC));
	dtd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dtd.ArraySize = 1;
	dtd.BindFlags = 0;
	dtd.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	dtd.Height = bih.biHeight;
	dtd.Width = bih.biWidth;
	dtd.Usage = D3D11_USAGE_STAGING;
	dtd.MipLevels = min(MipMapLevels, 1 + (unsigned long)floor(log(min(dtd.Width, dtd.Height)) / log(2)));
	dtd.SampleDesc.Count = 1;
	dtd.SampleDesc.Quality = 0;

	printf("Width %d, Height:%d, %dbpp\n", bih.biWidth, bih.biHeight, bih.biBitCount);
	fflush(stdout);
	pDev->CreateTexture2D(&dtd, NULL, &pTemp);
	struct PIXEL
	{
		unsigned char r, g, b, a;
	};
	D3D11_MAPPED_SUBRESOURCE ms;
	pCtx->Map(pTemp, 0, D3D11_MAP_READ_WRITE, 0, &ms);
	char *pDestStart = (char*)ms.pData + (bih.biHeight - 1)*ms.RowPitch;
	PIXEL *pDest = (PIXEL*)pDestStart;
	switch (bih.biBitCount)
	{
	case 1: //Tarea 1bpp 2 colores
		if (bih.biClrUsed == 0)//Si se usan todos los colores, ese miembro es cero
			bitmap.read((char*)Palette, 2 * sizeof(RGBQUAD));
		else
			bitmap.read((char*)Palette, bih.biClrUsed * sizeof(RGBQUAD));
		ulRowlenght = 4 * ((bih.biBitCount*bih.biWidth + 31) / 32);
		pBuffer = new unsigned char[ulRowlenght];
		for (int y = (bih.biHeight - 1); y >= 0; y--)
		{
			bitmap.read((char*)pBuffer, ulRowlenght);
			int x = 0;
			for (unsigned long iByte = 0; iByte<ulRowlenght; iByte++)
			{
				unsigned long iColorIndex;
				unsigned char c = pBuffer[iByte];
				for (int iBit = 0; iBit<8; iBit++)
				{
					iColorIndex = ((c & 0x80) != 0);
					c <<= 1;
					pDest->r = Palette[iColorIndex].rgbRed;
					pDest->g = Palette[iColorIndex].rgbGreen;
					pDest->b = Palette[iColorIndex].rgbBlue;
					if (pAlphaFunction)
						pDest->a = (unsigned char)max(0, min(255, (int)(255 * pAlphaFunction(pDest->r / 255.0f, pDest->g / 255.0f, pDest->b / 255.0f))));
					else
						pDest->a = 0xff;
					if (pColorFunction)
					{
						VECTOR4D Color = { pDest->r*(1.0f / 255), pDest->g*(1.0f / 255), pDest->b*(1.0f / 255), pDest->a*(1.0f / 255) };
						VECTOR4D Result = pColorFunction(Color);
						pDest->r = (unsigned char)max(0, min(255, (int)(Result.r * 255)));
						pDest->g = (unsigned char)max(0, min(255, (int)(Result.g * 255)));
						pDest->b = (unsigned char)max(0, min(255, (int)(Result.b * 255)));
						pDest->a = (unsigned char)max(0, min(255, (int)(Result.a * 255)));
					}
					x++;
					pDest++;
					if (x<bih.biWidth)
						continue;
				}
			}
			pDestStart -= ms.RowPitch;
			pDest = (PIXEL*)pDestStart;
		}
		delete[] pBuffer;
		break;
	case 4: //Aquí 4 bpp 16 colores
		if (bih.biClrUsed == 0)//Si se usan todos los colores, ese miembro es cero
			bitmap.read((char*)Palette, 16 * sizeof(RGBQUAD));
		else
			bitmap.read((char*)Palette, bih.biClrUsed * sizeof(RGBQUAD));
		//Leer el bitmap
		ulRowlenght = 4 * ((bih.biBitCount*bih.biWidth + 31) / 32);
		pBuffer = new unsigned char[ulRowlenght];
		for (int y = (bih.biHeight - 1); y >= 0; y--)
		{
			bitmap.read((char*)pBuffer, ulRowlenght);
			for (int x = 0; x<bih.biWidth; x++)
			{
				//Desempacar pixeles así
				unsigned char nibble = (x & 1) ? (pBuffer[x >> 1] & 0x0f) : (pBuffer[x >> 1] >> 4);
				pDest->r = Palette[nibble].rgbRed;
				pDest->b = Palette[nibble].rgbBlue;
				pDest->g = Palette[nibble].rgbGreen;
				if (pAlphaFunction)
					pDest->a = (unsigned char)max(0, min(255, (int)(255 * pAlphaFunction(pDest->r / 255.0f, pDest->g / 255.0f, pDest->b / 255.0f))));
				else
					pDest->a = 0xff;
				if (pColorFunction)
				{
					VECTOR4D Color = { pDest->r*(1.0f / 255), pDest->g*(1.0f / 255), pDest->b*(1.0f / 255), pDest->a*(1.0f / 255) };
					VECTOR4D Result = pColorFunction(Color);
					pDest->r = (unsigned char)max(0, min(255, (int)(Result.r * 255)));
					pDest->g = (unsigned char)max(0, min(255, (int)(Result.g * 255)));
					pDest->b = (unsigned char)max(0, min(255, (int)(Result.b * 255)));
					pDest->a = (unsigned char)max(0, min(255, (int)(Result.a * 255)));
				}
				pDest++;
			}
			pDestStart -= ms.RowPitch;
			pDest = (PIXEL*)pDestStart;
		}
		delete[] pBuffer;
		break;
	case 8: //Tarea 8 bpp 256 colores
		if (bih.biClrUsed == 0)//Si se usan todos los colores, ese miembro es cero
			bitmap.read((char*)Palette, 256 * sizeof(RGBQUAD));
		else
			bitmap.read((char*)Palette, bih.biClrUsed * sizeof(RGBQUAD));

		ulRowlenght = 4 * ((bih.biBitCount*bih.biWidth + 31) / 32);
		pBuffer = new unsigned char[ulRowlenght];

		for (int y = (bih.biHeight - 1); y >= 0; y--)
		{
			bitmap.read((char*)pBuffer, ulRowlenght);
			for (int x = 0; x<bih.biWidth; x++)
			{
				//Desempacar pixeles así
				unsigned char nibble = (x & 1) ? (pBuffer[x] & 0xff) : (pBuffer[x]);
				pDest->r = Palette[nibble].rgbRed;
				pDest->b = Palette[nibble].rgbBlue;
				pDest->g = Palette[nibble].rgbGreen;
				if (pAlphaFunction)
					pDest->a = (unsigned char)max(0, min(255, (int)(255 * pAlphaFunction(pDest->r / 255.0f, pDest->g / 255.0f, pDest->b / 255.0f))));
				else
					pDest->a = 0xff;
				if (pColorFunction)
				{
					VECTOR4D Color = { pDest->r*(1.0f / 255), pDest->g*(1.0f / 255), pDest->b*(1.0f / 255), pDest->a*(1.0f / 255) };
					VECTOR4D Result = pColorFunction(Color);
					pDest->r = (unsigned char)max(0, min(255, (int)(Result.r * 255)));
					pDest->g = (unsigned char)max(0, min(255, (int)(Result.g * 255)));
					pDest->b = (unsigned char)max(0, min(255, (int)(Result.b * 255)));
					pDest->a = (unsigned char)max(0, min(255, (int)(Result.a * 255)));
				}
				pDest++;
			}
			pDestStart -= ms.RowPitch;
			pDest = (PIXEL*)pDestStart;
		}
		delete[] pBuffer;
		break;
	case 24://Aquí 16777216 colores (True Color)
			//Leer el bitmap
		ulRowlenght = 4 * ((bih.biBitCount*bih.biWidth + 31) / 32);
		pBuffer = new unsigned char[ulRowlenght];
		for (int y = (bih.biHeight - 1); y >= 0; y--)
		{
			bitmap.read((char*)pBuffer, ulRowlenght);
			for (int x = 0; x<bih.biWidth; x++)
			{
				//Desempacar pixeles así
				pDest->b = pBuffer[3 * x + 0];
				pDest->g = pBuffer[3 * x + 1];
				pDest->r = pBuffer[3 * x + 2];
				if (pAlphaFunction)
					pDest->a = (unsigned char)max(0, min(255, (int)(255 * pAlphaFunction(pDest->r / 255.0f, pDest->g / 255.0f, pDest->b / 255.0f))));
				else
					pDest->a = 0xff;
				if (pColorFunction)
				{
					VECTOR4D Color = { pDest->r*(1.0f / 255), pDest->g*(1.0f / 255), pDest->b*(1.0f / 255), pDest->a*(1.0f / 255) };
					VECTOR4D Result = pColorFunction(Color);
					pDest->r = (unsigned char)max(0, min(255, (int)(Result.r * 255)));
					pDest->g = (unsigned char)max(0, min(255, (int)(Result.g * 255)));
					pDest->b = (unsigned char)max(0, min(255, (int)(Result.b * 255)));
					pDest->a = (unsigned char)max(0, min(255, (int)(Result.a * 255)));
				}
				pDest++;
			}
			pDestStart -= ms.RowPitch;
			pDest = (PIXEL*)pDestStart;
		}
		delete[] pBuffer;
		break;
	case 32:
		ulRowlenght = 4 * ((bih.biBitCount*bih.biWidth + 31) / 32);
		pBuffer = new unsigned char[ulRowlenght];
		for (int y = (bih.biHeight - 1); y >= 0; y--)
		{
			bitmap.read((char*)pBuffer, ulRowlenght);
			for (int x = 0; x<bih.biWidth; x++)
			{
				//Desempacar pixeles así
				pDest->b = pBuffer[4 * x + 0];
				pDest->g = pBuffer[4 * x + 1];
				pDest->r = pBuffer[4 * x + 2];
				if (pAlphaFunction)
					pDest->a = (unsigned char)max(0, min(255, (int)(255 * pAlphaFunction(pDest->r / 255.0f, pDest->g / 255.0f, pDest->b / 255.0f))));
				else
					pDest->a = 0xff;
				if (pColorFunction)
				{
					VECTOR4D Color = { pDest->r*(1.0f / 255), pDest->g*(1.0f / 255), pDest->b*(1.0f / 255), pDest->a*(1.0f / 255) };
					VECTOR4D Result = pColorFunction(Color);
					pDest->r = (unsigned char)max(0, min(255, (int)(Result.r * 255)));
					pDest->g = (unsigned char)max(0, min(255, (int)(Result.g * 255)));
					pDest->b = (unsigned char)max(0, min(255, (int)(Result.b * 255)));
					pDest->a = (unsigned char)max(0, min(255, (int)(Result.a * 255)));
				}
				pDest++;
			}
			pDestStart -= ms.RowPitch;
			pDest = (PIXEL*)pDestStart;
		}
		delete[] pBuffer;
		break;
	}
	//transfer cpu mem to gpu memory
	pCtx->Unmap(pTemp, 0);
	//Crear buffer en GPU
	dtd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	dtd.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.CPUAccessFlags = 0;
	ID3D11Texture2D* pTexture;
	pDev->CreateTexture2D(&dtd, NULL, &pTexture);
	//copy gpu mem to gpu mem for RW capable surface
	pCtx->CopyResource(pTexture, pTemp);
	if (ppSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
		srvd.Texture2D.MipLevels = dtd.MipLevels;
		srvd.Texture2D.MostDetailedMip = 0;
		srvd.Format = dtd.Format;
		srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		pDev->CreateShaderResourceView(pTexture, &srvd, ppSRV);
		printf("Generating %d mipmaps levels... ", dtd.MipLevels);
		fflush(stdout);
		pCtx->GenerateMips(*ppSRV);
		printf("done.\n");
		fflush(stdout);
	}
	pTemp->Release();
	printf("Load success.\n");
	return pTexture;
}

void CDXManager::Resize(int nSizeX, int nSizeY)
{
	if (m_pSwapChain) 
	{
		if (m_pDSV) m_pDSV->Release();
		if (m_pt2dBufferZ) m_pt2dBufferZ->Release();
		m_pContext->ClearState();//Desconecta todos los objetos pero no los borra. Se hace para el back buffer
		//Debido a que si esta en uso, no se puede cambiar su tamano
		m_pSwapChain->ResizeBuffers(2,nSizeX, nSizeY,DXGI_FORMAT_R8G8B8A8_UNORM,0);//Cambias el backbuffer
		//Crear buffer z
		D3D11_TEXTURE2D_DESC dtd;
		memset(&dtd, 0, sizeof(dtd));
		dtd.ArraySize = 1;
		dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		dtd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dtd.Width = nSizeX;
		dtd.Height = nSizeY;
		dtd.MipLevels = 1;
		dtd.Usage = D3D11_USAGE_DEFAULT;
		dtd.SampleDesc.Count = 1;
		dtd.SampleDesc.Quality = 0;
		m_pDevice->CreateTexture2D(&dtd, nullptr, &m_pt2dBufferZ);
		m_pDevice->CreateDepthStencilView(m_pt2dBufferZ, nullptr, &m_pDSV);
	}
}
