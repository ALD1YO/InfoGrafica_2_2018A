#pragma once
#include "CDXManager.h"
#include "Matrix4D.h"
class CGFX2D
{
protected:
	CDXManager * m_pManager;
	ID3D11InputLayout * m_pIL;
	ID3D11VertexShader * m_pVS;
	ID3D11PixelShader * m_pPS;
	ID3D11Buffer * m_pCB;
public:
	
	struct VERTEX2D
	{
		float x, y;
		float r, g, b;
		float u, v;//Texcoord
		static D3D11_INPUT_ELEMENT_DESC InputLayout[];
	};
	
	struct PARAMS
	{
		MATRIX4D World;
		VECTOR4D Brightness;
		float Time;
	}m_Params;

	CGFX2D(CDXManager* pOwner);
	bool Initialize();
	void Uninitialize();
	void Draw(VERTEX2D* pVertices, int nVertices);
	~CGFX2D();
};

