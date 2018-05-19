#pragma once
#include "CDXManager.h"
#include "CMesh.h"
#include "Matrix4D.h"
class CGFX3D
{
protected:
	CDXManager * m_pDXM;
	ID3D11InputLayout * m_pIL;
	ID3D11VertexShader * m_pVS;
	ID3D11PixelShader * m_pPS;
	ID3D11Buffer * m_pCB;//Constant buffer
	
public:
	struct MATERIAL
	{
		VECTOR4D Ambient;
		VECTOR4D Diffuse;
		VECTOR4D Specular;
		VECTOR4D Emissive;
		VECTOR4D Params;	//x:Specular Power
	};
#define LIGHT_OFF 0
#define LIGHT_ON 1
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

	struct LIGHT
	{
		//En HLSL los enteros son de 32 bits
		VECTOR4D SwitchAndType; //x: On/Off, y: 0:Directional, 1: Point  2:Spot 
		VECTOR4D Ambient;
		VECTOR4D Diffuse;
		VECTOR4D Specular;
		VECTOR4D Position;	//Position Light, Spot Light Only
		VECTOR4D Direction;	//Directional Light Only
		VECTOR4D Attenuation;
		VECTOR4D SpotParams;	//Parámetros de Luz de Spot ...
	};

#define LIGHTING_AMBIENT  0x00000001
#define LIGHTING_DIFFUSE  0x00000002
#define LIGHTING_SPECULAR 0x00000004
#define LIGHTING_EMISSIVE 0x00000008
#define MAPPING_DIFFUSE	  0x00000010
#define MAPPING_SPECULAR  0x00000020
#define MAPPING_EMISSIVE  0x00000040
#define SHADER_TOON		  0x00000080

	struct PARAMS 
	{
		VECTOR4D Options;
		MATRIX4D World;
		MATRIX4D View;
		MATRIX4D Projection;
		MATRIX4D WVP;
		MATRIX4D WV;
		MATERIAL Material;	//x:specular Power
		LIGHT Lights[8];
		VECTOR4D TexVelocity;
		float t;
	}m_Params;
	CGFX3D(CDXManager * pOwner);
	bool Initialize();
	void Uninitialize();
	void Configure();//Configura la tuberia grafica
	void UpdateCB();//cosas para actualizar posicion rotacion y escala de los objetos
	void Draw(const CMesh * pMesh);
	~CGFX3D();
};
