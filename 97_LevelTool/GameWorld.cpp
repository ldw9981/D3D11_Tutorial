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
using json = nlohmann::json;

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
	json root;
	root["version"] = "1.0";
	root["objects"] = json::array();

	for (GameObject* obj : m_GameObjects)
	{
		if (!obj)
			continue;

		// RTTR로 타입 정보 가져오기
		type t = type::get(*obj);
		std::string typeName = t.get_name().to_string();

		json objJson;
		objJson["type"] = typeName;
		objJson["properties"] = json::object();

		// 모든 프로퍼티 직렬화
		for (auto& prop : t.get_properties())
		{
			std::string propName = prop.get_name().to_string();
			variant value = prop.get_value(*obj);

			if (value.is_type<float>())
			{
				objJson["properties"][propName] = value.get_value<float>();
			}
			else if (value.is_type<int>())
			{
				objJson["properties"][propName] = value.get_value<int>();
			}
			else if (value.is_type<std::string>())
			{
				objJson["properties"][propName] = value.get_value<std::string>();
			}
			else if (value.is_type<Vector3>())
			{
				Vector3 v = value.get_value<Vector3>();
				objJson["properties"][propName] = { v.x, v.y, v.z };
			}
			else if (value.is_type<Vector4>())
			{
				Vector4 v = value.get_value<Vector4>();
				objJson["properties"][propName] = { v.x, v.y, v.z, v.w };
			}
		}

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

	json root;
	try
	{
		file >> root;
	}
	catch (const json::exception& e)
	{
		file.close();
		return false;
	}
	file.close();

	// 기존 객체들 모두 제거
	Clear();

	// 버전 확인 (선택적)
	if (root.contains("version"))
	{
		std::string version = root["version"];
		// 필요시 버전별로 다른 로딩 로직 적용 가능
	}

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

		// 프로퍼티 복원
		if (objJson.contains("properties") && objJson["properties"].is_object())
		{
			type t = type::get(*obj);

			for (auto& prop : t.get_properties())
			{
				std::string propName = prop.get_name().to_string();

				if (!objJson["properties"].contains(propName))
					continue;

				type propType = prop.get_type();
				const auto& propValue = objJson["properties"][propName];

				if (propType == type::get<float>() && propValue.is_number())
				{
					prop.set_value(*obj, propValue.get<float>());
				}
				else if (propType == type::get<int>() && propValue.is_number_integer())
				{
					prop.set_value(*obj, propValue.get<int>());
				}
				else if (propType == type::get<std::string>() && propValue.is_string())
				{
					prop.set_value(*obj, propValue.get<std::string>());
				}
				else if (propType == type::get<Vector3>() && propValue.is_array() && propValue.size() == 3)
				{
					Vector3 v;
					v.x = propValue[0].get<float>();
					v.y = propValue[1].get<float>();
					v.z = propValue[2].get<float>();
					prop.set_value(*obj, v);
				}
				else if (propType == type::get<Vector4>() && propValue.is_array() && propValue.size() == 4)
				{
					Vector4 v;
					v.x = propValue[0].get<float>();
					v.y = propValue[1].get<float>();
					v.z = propValue[2].get<float>();
					v.w = propValue[3].get<float>();
					prop.set_value(*obj, v);
				}
			}
		}

		// AABB 업데이트
		obj->UpdateAABB();
	}

	return true;
}
