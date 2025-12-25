#include "GameWorld.h"
#include "CubeObject.h"
#include <rttr/registration>
#include <algorithm>
#include <fstream>

// Windows.h의 min/max 매크로를 해제하여 RTTR과의 충돌 방지
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include "../Common/json.hpp"

using namespace rttr;


GameWorld::GameWorld()
{
}

GameWorld::~GameWorld()
{
	Clear();
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
	// 모든 GameObject 메모리 해제
	for (GameObject* obj : m_GameObjects)
	{
		delete obj;
	}
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

GameObject* GameWorld::CreateGameObjectByTypeName(const std::string& typeName)
{
	// RTTR로 타입 찾기
	rttr::type t = rttr::type::get_by_name(typeName);
	if (!t.is_valid())
		return nullptr;

	// as_raw_ptr 정책으로 등록된 생성자 호출 (힙에 직접 생성)
	rttr::variant obj = t.create();
	if (!obj.is_valid())
		return nullptr;

	// as_raw_ptr로 등록했으므로 포인터로 직접 추출
	GameObject* gameObj = obj.get_value<GameObject*>();

	if (gameObj != nullptr)
	{
		m_GameObjects.push_back(gameObj);
	}

	return gameObj;
}

void GameWorld::DestroyGameObject(GameObject* obj)
{
	auto it = std::find(m_GameObjects.begin(), m_GameObjects.end(), obj);
	if (it != m_GameObjects.end())
	{
		delete *it;
		m_GameObjects.erase(it);
	}
}

bool GameWorld::SaveToFile(const std::string& filename) const
{
	nlohmann::json root;
	
	root["objects"] = nlohmann::json::array();

	for (GameObject* obj : m_GameObjects)
	{
		if (!obj)
			continue;

		// GameObject의 Serialize 함수 호출
		nlohmann::json objJson = obj->Serialize();
		root["objects"].push_back(objJson);
	}

	// 파일에 저장 (들여쓰기 2칸)
	std::ofstream file(filename);
	if (!file.is_open())
		return false;

	file << root.dump(2);
	file.close();
	return true;
}

bool GameWorld::LoadFromFile(const std::string& filename)
{
	// 파일에서 JSON 읽기
	std::ifstream file(filename);
	if (!file.is_open())
		return false;

	nlohmann::json root;
	try
	{
		file >> root;
	}
	catch (const nlohmann::json::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
		file.close();
		return false;
	}
	file.close();

	// 기존 객체들 모두 제거
	Clear();		

	// 오브젝트 로드
	if (!root.contains("objects") || !root["objects"].is_array())
		return false;

	for (const auto& objJson : root["objects"])
	{
		if (!objJson.contains("type"))
			continue;

		std::string typeName = objJson["type"];
		GameObject* obj = CreateGameObjectByTypeName(typeName);
		if (!obj)
			continue;

		// GameObject의 Deserialize 함수 호출
		obj->Deserialize(objJson);
	}

	return true;
}
