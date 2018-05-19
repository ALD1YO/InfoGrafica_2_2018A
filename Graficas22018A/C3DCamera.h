#pragma once
#include "Matrix4D.h"
class C3DCamera
{
protected:
	MATRIX4D Camera; // World space

public:
	VECTOR4D GetXAxis() { return { Camera.m00,Camera.m01,Camera.m02,Camera.m03 }; }
	VECTOR4D GetYAxis() { return { Camera.m10,Camera.m11,Camera.m12,Camera.m13 }; }
	VECTOR4D GetZAxis() { return { Camera.m20,Camera.m21,Camera.m22,Camera.m23 }; }
	VECTOR4D GetPosition() { return { Camera.m30,Camera.m31,Camera.m32,Camera.m33 }; }

	void RotateXAxis(float dtheta);
	void RotateYAxis(float dtheta);
	void RotateZAxis(float dtheta);

	void MoveXAxis(float dx);
	void MoveYAxis(float dy);
	void MoveZAxis(float dz);

	MATRIX4D GetViewMatrix() { return ViewInverse(Camera); }
	void InitializeFromViewMatrix(const MATRIX4D& View)
									{Camera = ViewInverse(View);}
	C3DCamera();
	~C3DCamera();
};

