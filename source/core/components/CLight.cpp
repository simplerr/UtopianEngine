#include "core/components/CLight.h"
#include "core/SceneNode.h"
#include "core/World.h"
#include "imgui/imgui.h"

namespace Utopian
{
	CLight::CLight(Actor* parent)
		: Component(parent)
	{
		SetName("CLight");
	}

	CLight::~CLight()
	{

	}

	void CLight::Update()
	{
	}

	void CLight::OnCreated()
	{
		mInternal = Light::Create();

		World::Instance().BindNode(mInternal, GetParent());
	}

	void CLight::SetMaterials(const const vec4& ambient, const vec4& diffuse, const vec4& specular)
	{
		mInternal->SetMaterials(ambient, diffuse, specular);
	}

	void CLight::SetMaterial(const vec4& color)
	{
		mInternal->SetMaterial(color);
	}

	void CLight::SetMaterial(const Utopian::Vk::Material& material)
	{
		mInternal->SetMaterial(material);
	}

	void CLight::SetDirection(const vec3& direction)
	{
		mInternal->SetDirection(direction);
	}

	void CLight::SetDirection(float x, float y, float z)
	{
		mInternal->SetDirection(vec3(x, y, z));
	}

	void CLight::SetRange(float range)
	{
		mInternal->SetRange(range);
	}

	void CLight::SetSpot(float spot)
	{
		mInternal->SetSpot(spot);
	}

	void CLight::SetAtt(float a0, float a1, float a2)
	{
		mInternal->SetAtt(a0, a1, a2);
	}

	void CLight::SetType(Utopian::Vk::LightType type)
	{
		mInternal->SetType(type);
	}

	void CLight::SetIntensity(float ambient, float diffuse, float specular)
	{
		mInternal->SetIntensity(ambient, diffuse, specular);
	}

	void CLight::SetIntensity(vec3 intensity)
	{
		SetIntensity(intensity.x, intensity.y, intensity.z);
	}

	const Utopian::Vk::LightData& CLight::GetLightData() const
	{
		return mInternal->GetLightData();
	}

	const vec3& CLight::GetDirection() const
	{
		return mInternal->GetDirection();
	}

	const vec3& CLight::GetAtt() const
	{
		return mInternal->GetAtt();
	}

	const vec3& CLight::GetIntensity() const
	{
		return mInternal->GetIntensity();
	}

	Utopian::Vk::Material CLight::GetMaterial() const
	{
		return mInternal->GetMaterial();
	}

	float CLight::GetRange() const
	{
		return mInternal->GetRange();
	}

	float CLight::GetSpot() const
	{
		return mInternal->GetSpot();
	}

	int CLight::GetLightType() const
	{
		return mInternal->GetType();
	}
}