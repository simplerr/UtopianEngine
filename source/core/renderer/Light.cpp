#include "core/renderer/Light.h"
#include "core/renderer/SceneRenderer.h"

namespace Utopian
{

	Light::Light()
	{
		// Default values
		SetMaterial(vec4(1.0f, 1.0f, 1.0f, 1.0f));
		SetDirection(vec3(1.0f, 1.0f, 1.0f));
		SetRange(1000000.0f);
		SetSpot(100.0f);
		SetAtt(1.0f, 1.0f, 1.0f);
		SetType(Utopian::Vk::LightType::DIRECTIONAL_LIGHT);
		SetIntensity(1.0f, 1.0f, 1.0f);
	}

	Light::~Light()
	{

	}

	SharedPtr<Utopian::Light> Light::Create()
	{
		SharedPtr<Light> instance(new Light());
		instance->Initialize();

		return instance;
	}

	void Light::Initialize()
	{
		SceneRenderer::Instance().AddLight(this);
	}

	void Light::SetLightData(const Utopian::Vk::LightData& lightData)
	{
		mLightData = lightData;
	}

	const Utopian::Vk::LightData& Light::GetLightData()
	{
		// Todo: Is this ok?
		mLightData.position = GetTransform().GetPosition();
		return mLightData;
	}

	void Light::SetMaterials(const const vec4& ambient, const vec4& diffuse, const vec4& specular)
	{
		mLightData.material = Utopian::Vk::Material(ambient, diffuse, specular);
		mLightData.intensity = vec3(1.0f, 1.0f, 1.0f);
	}

	void Light::SetMaterial(const vec4& color)
	{
		mLightData.material = Utopian::Vk::Material(color);
		mLightData.intensity = vec3(1.0f, 1.0f, 1.0f);
	}

	void Light::SetMaterial(const Utopian::Vk::Material & material)
	{
		mLightData.material = material;
	}

	void Light::SetDirection(const vec3& direction)
	{
		mLightData.direction = normalize(direction);
	}

	void Light::SetRange(float range)
	{
		mLightData.range = range;
	}

	void Light::SetSpot(float spot)
	{
		mLightData.spot = spot;
	}

	void Light::SetAtt(float a0, float a1, float a2)
	{
		mLightData.att = vec3(a0, a1, a2);
	}

	void Light::SetType(Utopian::Vk::LightType type)
	{
		mLightData.type = type;
	}

	void Light::SetIntensity(float ambient, float diffuse, float specular)
	{
		mLightData.intensity = vec3(ambient, diffuse, specular);
	}

	const vec3& Light::GetDirection() const
	{
		return mLightData.direction;
	}

	const vec3& Light::GetAtt() const
	{
		return mLightData.att;
	}

	const vec3& Light::GetIntensity() const
	{
		return mLightData.intensity;
	}

	Utopian::Vk::LightData* Light::GetLightDataPtr()
	{
		return &mLightData;
	}

	const Utopian::Vk::LightData& Light::GetLightData() const
	{
		return mLightData;
	}

	Utopian::Vk::Material Light::GetMaterial() const
	{
		return mLightData.material;
	}

	float Light::GetRange() const
	{
		return mLightData.range;
	}

	float Light::GetSpot() const
	{
		return mLightData.spot;
	}

	int Light::GetType() const
	{
		return mLightData.type;
	}
}