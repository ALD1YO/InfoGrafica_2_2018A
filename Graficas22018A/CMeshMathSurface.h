#pragma once
#include "CMesh.h"
class CMeshMathSurface :
	public CMesh
{
protected:
	unsigned long m_nVx;
	unsigned long m_nVy;

public:
	void BuildAnalyticSurface(const int nVx, const int nVy, 
		const float x0, const float y0,
		const float dx, const float dy,
		float(*pF)(const float x,const float y), 
		VECTOR4D (*pDivergent)(const VECTOR4D& P));
	void BuildParametricSurface(const int nVx, const int nVy,
		const float p0, const float q0,
		const float dp, const float dq,
		VECTOR4D(*pF)(const float p, const float q),
		VECTOR4D(*pDivergent)(const VECTOR4D& P));
	void Tesselate();
	void SetColor(
		const VECTOR4D& A,
		const VECTOR4D& B,
		const VECTOR4D& C,
		const VECTOR4D& D
		);
	void BuildTextureCoords(float u0, float v0, float du, float dv);
	CMeshMathSurface();
	~CMeshMathSurface();
};

