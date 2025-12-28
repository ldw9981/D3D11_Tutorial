#include "GameObject.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define RTTR_DLL
#include <rttr/registration>
#include <rttr/detail/policies/ctor_policies.h>


using namespace rttr;

RTTR_REGISTRATION
{
	registration::class_<GameObject>("GameObject")
		.constructor<>()
			(rttr::policy::ctor::as_raw_ptr)
		.property("Position", &GameObject::m_Position)
		.property("Rotation", &GameObject::m_Rotation)
		.property("Scale", &GameObject::m_Scale);

	
	registration::class_<Vector3>("Vector3")
		.constructor<>()
		.constructor<float, float, float>()
		.property("x", &Vector3::x)
		.property("y", &Vector3::y)
		.property("z", &Vector3::z);

	registration::class_<Vector4>("Vector4")
		.constructor<>()
		.constructor<float, float, float, float>()
		.property("x", &Vector4::x)
		.property("y", &Vector4::y)
		.property("z", &Vector4::z)
		.property("w", &Vector4::w);
}

GameObject::GameObject()
	: m_Position(0.0f, 0.0f, 0.0f)
	, m_Rotation(0.0f, 0.0f, 0.0f)
	, m_Scale(1.0f, 1.0f, 1.0f)
	, m_AABB(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.5f, 0.5f, 0.5f))
{
}

GameObject::~GameObject()
{
}

Matrix GameObject::GetWorldMatrix()
{
	// Scale -> Rotation -> Translation
	Matrix scaleMatrix = Matrix::CreateScale(m_Scale);
	// 3축 회전은 쿼터니언으로 변환		
	Quaternion rotationQuat = Quaternion::CreateFromYawPitchRoll(
		DirectX::XMConvertToRadians(m_Rotation.y),
		DirectX::XMConvertToRadians(m_Rotation.x),
		DirectX::XMConvertToRadians(m_Rotation.z));
	Matrix rotationMatrix = Matrix::CreateFromQuaternion(rotationQuat);
	Matrix translationMatrix = Matrix::CreateTranslation(m_Position);

	m_WorldM = scaleMatrix * rotationMatrix * translationMatrix;
	return m_WorldM;
}

void GameObject::UpdateAABB()
{	
	BoundingBox originalAABB(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.5f, 0.5f, 0.5f));

	// Scale 
	Vector3 scaledExtents = originalAABB.Extents * m_Scale;

	// Position 
	m_AABB.Center = m_Position;
	m_AABB.Extents = scaledExtents;
}

nlohmann::json GameObject::Serialize() const
{
	nlohmann::json objJson;
	
	type t = type::get(*this);
	objJson["type"] = t.get_name().to_string();
	objJson["properties"] = nlohmann::json::object();
	
	for (auto& prop : t.get_properties())
	{
		std::string propName = prop.get_name().to_string();
		variant value = prop.get_value(*this);

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

	return objJson;
}

bool GameObject::Deserialize(const nlohmann::json& jsonObj)
{
	if (!jsonObj.contains("properties") || !jsonObj["properties"].is_object())
		return false;

	type t = type::get(*this);

	for (auto& prop : t.get_properties())
	{
		std::string propName = prop.get_name().to_string();

		if (!jsonObj["properties"].contains(propName))
			continue;

		type propType = prop.get_type();
		const auto& propValue = jsonObj["properties"][propName];

		if (propType == type::get<float>() && propValue.is_number())
		{
			prop.set_value(*this, propValue.get<float>());
		}
		else if (propType == type::get<int>() && propValue.is_number_integer())
		{
			prop.set_value(*this, propValue.get<int>());
		}
		else if (propType == type::get<std::string>() && propValue.is_string())
		{
			prop.set_value(*this, propValue.get<std::string>());
		}
		else if (propType == type::get<Vector3>() && propValue.is_array() && propValue.size() == 3)
		{
			Vector3 v;
			v.x = propValue[0].get<float>();
			v.y = propValue[1].get<float>();
			v.z = propValue[2].get<float>();
			prop.set_value(*this, v);
		}
		else if (propType == type::get<Vector4>() && propValue.is_array() && propValue.size() == 4)
		{
			Vector4 v;
			v.x = propValue[0].get<float>();
			v.y = propValue[1].get<float>();
			v.z = propValue[2].get<float>();
			v.w = propValue[3].get<float>();
			prop.set_value(*this, v);
		}
	}
	
	UpdateAABB();

	return true;
}
