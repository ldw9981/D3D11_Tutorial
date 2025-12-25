#include "GameWorld.h"


GameWorld::GameWorld()
{
}

GameWorld::~GameWorld()
{
}

void GameWorld::Insert(GameObject* gameObject)
{
	if (gameObject != nullptr)
	{
		m_GameObjects.push_back(gameObject);
	}
}

const std::vector<GameObject*>& GameWorld::GetGameObjects() const
{
	return m_GameObjects;
}

void GameWorld::Clear()
{
	m_GameObjects.clear();
}

size_t GameWorld::GetCount() const
{
	return m_GameObjects.size();
}

GameObject* GameWorld::RayCast(const Ray& ray, float* outDistance) const
{
	GameObject* hitObject = nullptr;
	float minDistance = FLT_MAX;

	for (GameObject* obj : m_GameObjects)
	{
		if (obj == nullptr)
			continue;

		float distance = 0.0f;
		if (ray.Intersects(obj->m_AABB, distance))
		{
			// 더 가까운 충돌체만 기록
			if (distance < minDistance)
			{
				minDistance = distance;
				hitObject = obj;
			}
		}
	}

	// 충돌 거리 반환 (선택적)
	if (outDistance != nullptr && hitObject != nullptr)
	{
		*outDistance = minDistance;
	}

	return hitObject;
}
