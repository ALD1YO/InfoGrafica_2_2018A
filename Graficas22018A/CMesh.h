#pragma once
#include "Matrix4D.h"
#include <vector>
using namespace std;

class CMesh
{
public:
	struct VERTEX
	{
		VECTOR4D Position;
		VECTOR4D Normal;
		VECTOR4D Color;
		VECTOR4D TexCoord;
	};
	//Topologia lista de triangugos indexados
	vector<VERTEX> m_V;
	vector<unsigned long> m_I;

public:
	void LoadSuzzane();
	CMesh();
	~CMesh();
};