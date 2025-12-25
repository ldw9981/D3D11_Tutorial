#pragma once
#include "GameObject.h"
#include <DirectXCollision.h>
#include <vector>
#include <string>

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

	// 모든 GameObject 제거 및 메모리 해제
	void Clear();

	// GameObject 개수
	size_t GetCount() const;

	// Ray 충돌 검사 - 첫 번째 충돌하는 GameObject 반환
	GameObject* RayCast(const Ray& ray, float* outDistance = nullptr) const;

	// 팩토리 메서드 - 템플릿 기반 생성
	template<typename T>
	T* CreateGameObject()
	{
		static_assert(std::is_base_of<GameObject, T>::value, "T must derive from GameObject");
		T* obj = new T();
		m_GameObjects.push_back(obj);
		return obj;
	}

	// 팩토리 메서드 - 타입 이름으로 생성 (RTTR 활용)
	GameObject* CreateGameObjectByTypeName(const std::string& typeName);

	// GameObject 삭제
	void DestroyGameObject(GameObject* obj);

	// 직렬화 - 파일에 저장
	bool SaveToFile(const std::string& filename) const;

	// 역직렬화 - 파일에서 로드
	bool LoadFromFile(const std::string& filename);

private:
	std::vector<GameObject*> m_GameObjects;
};
