#include "stdafx.h"
#include "CMesh.h"

#include <fstream>
using namespace std;

void CMesh::LoadSuzzane()
{
	fstream Positions;
	fstream Normals;
	fstream Indices;
	Positions.open("..\\Data\\Monkey.position", ios::in|ios::binary); //el ios blabla es para que lea los archivos binarios
	Normals.open("..\\Data\\Monkey.normal", ios::in|ios::binary);
	Indices.open("..\\Data\\Monkey.index",ios::in|ios::binary);
	unsigned long nVertices;
	Positions.read((char*)&nVertices, sizeof(unsigned long));
	m_V.resize(nVertices);
	for (int i = 0; i < nVertices; i++)
	{
		Positions.read((char*)&m_V[i].Position, sizeof(VECTOR4D));
		Normals.read((char*)&m_V[i].Normal, sizeof(VECTOR4D));
		m_V[i].TexCoord = m_V[i].Position;
		m_V[i].TexCoord.z = 0;
		m_V[i].TexCoord.w = 0;
	}
	unsigned long nIndices;
	Indices.read((char*)&nIndices, sizeof(unsigned long));
	m_I.resize(nIndices);
	Indices.read((char*)&m_I[0], sizeof(unsigned long)*nIndices);
}

CMesh::CMesh()
{
}


CMesh::~CMesh()
{
}
