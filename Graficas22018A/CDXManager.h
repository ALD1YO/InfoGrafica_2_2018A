#pragma once
#include<dxgi.h>
#include<d3d11.h>
#include"Matrix4D.h"
class CDXManager
{
protected:
	IDXGISwapChain * m_pSwapChain;//administrador de recursos
	ID3D11Device * m_pDevice;//ordenes y comandos al gpu/cpu
	ID3D11DeviceContext * m_pContext;//memoria visible en la pantalla
	ID3D11Texture2D* m_pt2dBufferZ;//herculeo y poderoso buffer z aunque lo odio
	ID3D11DepthStencilView* m_pDSV;//vista del buffer z
	ID3D11SamplerState* m_pSamplers[8]; //Colección de samplers
	
public:
	ID3D11SamplerState *const* GetSampler(int nSampler)
	{
		return &m_pSamplers[nSampler];
	}//Puedes leer el puntero pero no tocarlo. El const
	IDXGISwapChain * GetSwapChain() { return m_pSwapChain; }
	ID3D11DepthStencilView* GetDepthStencil() { return m_pDSV; }
	ID3D11Device * GetDevice() { return m_pDevice; }
	ID3D11DeviceContext * GetContext(){ return m_pContext; }
	bool Initialize(HWND hWnd, IDXGIAdapter* pAdapter, int nSizeX, int nSizeY);
	IDXGIAdapter * EnumAndChooseAdapter(HWND hWnd);
	ID3D11PixelShader* CompilePS(const wchar_t* pszFileName,const char* pszEntryPoint);
	ID3D11VertexShader* CompileVS(const wchar_t* pszFileName,const char* pszEntryPoint, ID3DBlob** ppDXIL);
	ID3D11Texture2D* LoadTexture(const char* pszFileName,		//The file name
		unsigned long MipMapLevels, //Number of mipmaps to generate, -1 automatic (int)log2(min(sizeX,sizeY))+1 levels
		ID3D11ShaderResourceView** ppSRV, //The Shader Resource View
		float(*pAlphaFunction)(float r, float g, float b),  //Operates pixel's rgb channels to build an alpha channel (normalized), can be null
		VECTOR4D(*pColorFunction)(const VECTOR4D& Color));
	void Resize(int nSizeX, int nSizeY);
	CDXManager();
	~CDXManager();
};

