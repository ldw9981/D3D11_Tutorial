#pragma once
#include "GameObject.h"
#include <DirectXCollision.h>
#include <vector>

using namespace DirectX;

class GameWorld
{
public:
	GameWorld();
	~GameWorld();

	// GameObject 추가
	void Insert(GameObject* gameObject);

	// GameObject 목록 가져오기
	const std::vector<GameObject*>& GetGameObjects() const;

	// 모든 GameObject 제거
	void Clear();

	// GameObject 개수
	size_t GetCount() const;

	// Ray 충돌 검사 - 첫 번째 충돌하는 GameObject 반환
	GameObject* RayCast(const Ray& ray, float* outDistance = nullptr) const;

private:
	std::vector<GameObject*> m_GameObjects;
};
