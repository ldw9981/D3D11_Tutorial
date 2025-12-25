#include "GameWorld.h"
#include "CubeObject.h"
#include <rttr/registration>
#include <algorithm>

// Windows.h의 min/max 매크로를 해제하여 RTTR과의 충돌 방지
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

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
	FILE* file = nullptr;
	fopen_s(&file, filename.c_str(), "w");
	if (!file)
		return false;

	fprintf(file, "{\n");
	fprintf(file, "  \"objects\": [\n");

	for (size_t i = 0; i < m_GameObjects.size(); ++i)
	{
		GameObject* obj = m_GameObjects[i];
		if (!obj)
			continue;

		// RTTR로 타입 이름 가져오기
		type t = type::get(*obj);
		std::string typeName = t.get_name().to_string();

		fprintf(file, "    {\n");
		fprintf(file, "      \"type\": \"%s\",\n", typeName.c_str());

		// 모든 프로퍼티 직렬화
		for (auto& prop : t.get_properties())
		{
			std::string propName = prop.get_name().to_string();
			variant value = prop.get_value(*obj);

			if (value.is_type<float>())
			{
				fprintf(file, "      \"%s\": %.3f,\n", propName.c_str(), value.get_value<float>());
			}
			else if (value.is_type<std::string>())
			{
				fprintf(file, "      \"%s\": \"%s\",\n", propName.c_str(), value.get_value<std::string>().c_str());
			}
			else if (value.is_type<Vector3>())
			{
				Vector3 v = value.get_value<Vector3>();
				fprintf(file, "      \"%s\": [%.3f, %.3f, %.3f],\n", propName.c_str(), v.x, v.y, v.z);
			}
			else if (value.is_type<Vector4>())
			{
				Vector4 v = value.get_value<Vector4>();
				fprintf(file, "      \"%s\": [%.3f, %.3f, %.3f, %.3f],\n", propName.c_str(), v.x, v.y, v.z, v.w);
			}
		}

		// 마지막 쉼표 제거를 위해 더미 필드 추가
		fprintf(file, "      \"_end\": true\n");

		if (i < m_GameObjects.size() - 1)
			fprintf(file, "    },\n");
		else
			fprintf(file, "    }\n");
	}

	fprintf(file, "  ]\n");
	fprintf(file, "}\n");
	fclose(file);
	return true;
}

bool GameWorld::LoadFromFile(const std::string& filename)
{
	FILE* file = nullptr;
	fopen_s(&file, filename.c_str(), "r");
	if (!file)
		return false;

	// 기존 객체들 모두 제거
	Clear();

	char line[1024];
	std::string currentType;
	GameObject* currentObj = nullptr;

	while (fgets(line, sizeof(line), file))
	{
		// "type" 필드 찾기
		if (strstr(line, "\"type\":"))
		{
			char typeName[256];
			if (sscanf_s(line, "      \"type\": \"%[^\"]\"", typeName, (unsigned)_countof(typeName)) == 1)
			{
				currentType = typeName;
				currentObj = CreateGameObjectByTypeName(currentType);
			}
		}
		else if (currentObj != nullptr)
		{
			// 프로퍼티 파싱
			type t = type::get(*currentObj);

			for (auto& prop : t.get_properties())
			{
				std::string propName = prop.get_name().to_string();
				std::string searchPattern = "\"" + propName + "\":";

				if (strstr(line, searchPattern.c_str()))
				{
					type propType = prop.get_type();

					if (propType == type::get<float>())
					{
						float value;
						if (sscanf_s(line, "      \"%*[^\"]\": %f", &value) == 1)
						{
							prop.set_value(*currentObj, value);
						}
					}
					else if (propType == type::get<std::string>())
					{
						char value[256];
						if (sscanf_s(line, "      \"%*[^\"]\": \"%[^\"]\"", value, (unsigned)_countof(value)) == 1)
						{
							prop.set_value(*currentObj, std::string(value));
						}
					}
					else if (propType == type::get<Vector3>())
					{
						Vector3 v;
						if (sscanf_s(line, "      \"%*[^\"]\": [%f, %f, %f]", &v.x, &v.y, &v.z) == 3)
						{
							prop.set_value(*currentObj, v);
						}
					}
					else if (propType == type::get<Vector4>())
					{
						Vector4 v;
						if (sscanf_s(line, "      \"%*[^\"]\": [%f, %f, %f, %f]", &v.x, &v.y, &v.z, &v.w) == 4)
						{
							prop.set_value(*currentObj, v);
						}
					}
				}
			}
		}
	}

	fclose(file);
	return true;
}
