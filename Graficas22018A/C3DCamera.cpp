#include "stdafx.h"
#include "C3DCamera.h"


C3DCamera::C3DCamera()
{
}


C3DCamera::~C3DCamera()
{
}

void C3DCamera::RotateXAxis(float dtheta)
{
	MATRIX4D O;
	VECTOR4D P = GetPosition();
	O = Camera;
	O.m30 = O.m31 = O.m32 = 0.0f;
	O = O * RotationAxis(dtheta, GetXAxis());
	Camera = O;
	Camera.m30 = P.x;
	Camera.m31 = P.y;
	Camera.m32 = P.z;
}

void C3DCamera::RotateYAxis(float dtheta)
{
	MATRIX4D O;
	VECTOR4D P = GetPosition();
	O = Camera;
	O.m30 = O.m31 = O.m32 = 0.0f;
	O = O * RotationAxis(dtheta, GetYAxis());
	Camera = O;
	Camera.m30 = P.x;
	Camera.m31 = P.y;
	Camera.m32 = P.z;
}

void C3DCamera::RotateZAxis(float dtheta)
{
	MATRIX4D O;
	VECTOR4D P = GetPosition();
	O = Camera;
	O.m30 = O.m31 = O.m32 = 0.0f;
	O = O * RotationAxis(dtheta, GetZAxis());
	Camera = O;
	Camera.m30 = P.x;
	Camera.m31 = P.y;
	Camera.m32 = P.z;
}

void C3DCamera::MoveZAxis(float dz)
{
	VECTOR4D P = GetPosition();
	VECTOR4D Z = GetZAxis();
	P = {P.x+Z.x*dz, P.y+Z.y*dz, P.z+Z.z*dz, 0};
	Camera.m30 = P.x;
	Camera.m31 = P.y;
	Camera.m32 = P.z;
}
////////////////////////////////////////////
/*************JALA PORFAVOR*****************/
void C3DCamera::MoveYAxis(float dy)
{
	VECTOR4D P = GetPosition();
	VECTOR4D Y = GetYAxis();
	P = { P.x + Y.x*dy, P.y + Y.y*dy, P.z + Y.z*dy, 0 };
	Camera.m30 = P.x;
	Camera.m31 = P.y;
	Camera.m32 = P.z;
}

void C3DCamera::MoveXAxis(float dx)
{
	VECTOR4D P = GetPosition();
	VECTOR4D X = GetXAxis();
	P = { P.x + X.x*dx, P.y + X.y*dx, P.z + X.z*dx, 0 };
	Camera.m30 = P.x;
	Camera.m31 = P.y;
	Camera.m32 = P.z;
}
/**************************************************/