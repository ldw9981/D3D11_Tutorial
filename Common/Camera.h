#pragma once

#include <directxtk/SimpleMath.h>
#include "InputSystem.h"

using namespace DirectX::SimpleMath;

class Camera : public InputProcesser
{
public:
	Camera();
	Vector3 m_Rotation;	//  z�� Roll �ش�ǹǷ� ������.
	Vector3 m_PositionInitial = { 0,0,-30 };
	Vector3 m_Position;
	Matrix m_World;		// �ӽ�ī�޶�� �θ� ������ ������ķ� �ٷ� ���� 
	Vector3 m_InputVector;

	float m_MoveSpeed = 20.0f;		
	float m_RotationSpeed = 0.004f;	// rad per sec

	Vector3 GetForward();
	Vector3 GetRight();
	
	void Reset();
	void Update(float elapsedTime);
	void GetViewMatrix(Matrix& out);
	void AddInputVector(const Vector3& input);
	void SetSpeed(float speed) { m_MoveSpeed = speed; }
	void AddPitch(float value);
	void AddYaw(float value);

	virtual void OnInputProcess(const Keyboard::State& KeyState, const Keyboard::KeyboardStateTracker& KeyTracker,
		const Mouse::State& MouseState, const Mouse::ButtonStateTracker& MouseTracker);
};

