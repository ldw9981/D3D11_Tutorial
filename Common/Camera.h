#pragma once

#include <directxtk/SimpleMath.h>


using namespace DirectX::SimpleMath;

class Camera
{
public:

	Vector3 m_Rotation;
	Vector3 m_Position = { 0,0,-30 } ;
	Matrix m_World;
	Vector3 m_InputVector;
	float m_Speed = 20.0f;

	Vector3 GetForward();
	Vector3 GetRight();
	
	void Update(float elapsedTime);
	void GetViewMatrix(Matrix& out);
	void AddInputVector(const Vector3& input);
	void SetSpeed(float speed) { m_Speed = speed; }
};

