#include "pch.h"
#include "Camera.h"



Vector3 Camera::GetForward()
{
	return -m_World.Forward();
}

Vector3 Camera::GetRight()
{
	return m_World.Right();
}

void Camera::Update(float elapsedTime)
{
	if (m_InputVector.Length() > 0.0f)
	{
		m_Position += m_InputVector * m_Speed * elapsedTime;
		m_InputVector = Vector3::Zero;
	}

	m_World = Matrix::CreateFromYawPitchRoll(m_Rotation)*
		Matrix::CreateTranslation(m_Position);
}

void Camera::GetViewMatrix(Matrix& out)
{
	Vector3 eye = m_World.Translation();
	Vector3 target = m_World.Translation() + GetForward();
	Vector3 up = m_World.Up();
	out = XMMatrixLookAtLH(eye, target, up);
}

void Camera::AddInputVector(const Math::Vector3& input)
{
	m_InputVector += input;
	m_InputVector.Normalize();
}

