#include "stdafx.h"
#include "CMeshMathSurface.h"


void CMeshMathSurface::BuildAnalyticSurface(const int nVx, const int nVy,
	const float x0, const float y0, 
	const float dx, const float dy, 
	float(*pF)(const float x, const float y),
	VECTOR4D(*pDivergent)(const VECTOR4D &P))
{
	m_nVx = nVx;
	m_nVy = nVy;
	m_V.resize(m_nVx*m_nVy);
	//buffer de indices asignado
	
	//construccion de las coordenadas
	float x = x0;
	float y = y0;
	//Calculo de posiciones y normal de cada vertice	
	for (unsigned long j = 0; j < m_nVy; j++) 
	{
		for (unsigned long i = 0; i < m_nVx; i++)
		{
			float z = pF(x, y);	
			m_V[j * m_nVx + i].Position = { x,y,z,1 };
			m_V[j * m_nVx + i].Normal = pDivergent(m_V[j * m_nVx + i].Position);
			x += dx;
		}
		x = x0;
		y += dy;
	}
	Tesselate();
}

void CMeshMathSurface::BuildParametricSurface(const int nVx, const int nVy, const float p0, const float q0, const float dp, const float dq, VECTOR4D(*pF)(const float p, const float q), VECTOR4D(*pDivergent)(const VECTOR4D &P))
{
	
	m_nVx = nVx;
	m_nVy = nVy;
	m_V.resize(m_nVx*m_nVy);
	float p = p0;
	float q = q0;
	//Calculo de posiciones y normal de cada vertice	
	for (unsigned long j = 0; j < m_nVy; j++)
	{
		for (unsigned long i = 0; i < m_nVx; i++)
		{
			VECTOR4D Position = pF(p, q);
			m_V[j * m_nVx + i].Position = Position;
			m_V[j * m_nVx + i].Normal = pDivergent(m_V[j * m_nVx + i].Position);
			p += dp;
		}
		p = p0;
		q += dq;
	}
	Tesselate();
}

void CMeshMathSurface::Tesselate()
{
	unsigned long quad = 0;
	m_I.resize((m_nVx - 1)*(m_nVy - 1) * 2 * 3);
	for (unsigned long j = 0; j < m_nVy-1; j++)
	{
		for (unsigned long i = 0; i < m_nVx-1; i++)
		{
			m_I[quad + 0] = j * m_nVx + i;
			m_I[quad + 1] = j * m_nVx + i + 1;
			m_I[quad + 2] = (j + 1)*m_nVx + i;
			m_I[quad + 3] = m_I[quad + 2];
			m_I[quad + 4] = m_I[quad + 1];
			m_I[quad + 5] = (j + 1)*m_nVx + i + 1;
			quad+=6;
		}
	}
}

void CMeshMathSurface::SetColor(const VECTOR4D & A, const VECTOR4D & B, const VECTOR4D & C, const VECTOR4D & D )
{
	float dx = 1.0f / (m_nVx - 1), dy = 1.0f / (m_nVy - 1);
	for (int j = 0; j < m_nVy; j++) 
	{
		float y = dy * j;
		for (int i = 0; i < m_nVx; i++) 
		{
			float x = dx * i;
			m_V[j*m_nVx + i].Color = 
				Lerp(Lerp(A, B, x), Lerp(C, D, x), y);	
		}
	}
}

void CMeshMathSurface::BuildTextureCoords(
	float u0, float v0, float du, float dv)
{
	for (int j = 0; j < m_nVy; j++)
		for (int i = 0; i < m_nVx; i++)
			m_V[j*m_nVx + i].TexCoord = { u0 + du * i,v0 + dv * j,0,1 };
											//a + b *i              
											//los vectores son inmunes a la traslacion
	//Tapizar el objeto con los puntos del mapa de textura
	//Esquina derecha??
	//du df = cuantas veces se repite la textura
	//u0 v0 = donde iniciar
}

//Constructor
//Sin guionsito es un constructor
CMeshMathSurface::CMeshMathSurface()
{
}

//Destructor
//Con guionsito es un destructor
CMeshMathSurface::~CMeshMathSurface()
{
}
