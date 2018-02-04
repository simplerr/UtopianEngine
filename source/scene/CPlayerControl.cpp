#include "scene/COrbit.h"
#include "scene/CNoClip.h"
#include "scene/CTransform.h"
#include "scene/CCamera.h"
#include "scene/CPlayerControl.h"
#include "scene/Actor.h"
#include "Input.h"

namespace Scene
{
	CPlayerControl::CPlayerControl(Actor* parent)
		: SceneComponent(parent)
	{
		mTransform = GetParent()->GetComponent<CTransform>();
		mCamera = GetParent()->GetComponent<CCamera>();
		mOrbit = GetParent()->GetComponent<COrbit>();
		mNoClip = GetParent()->GetComponent<CNoClip>();

		mOrbit->SetActive(false);
	}

	CPlayerControl::~CPlayerControl()
	{
	}

	void CPlayerControl::Update()
	{
		if (gInput().KeyPressed('R'))
		{
			mOrbit->SetActive(true);
			mNoClip->SetActive(false);

			mOrbit->SetRadius(glm::distance(mTransform->GetPosition(), mOrbit->GetTarget()));
		}
		else if (gInput().KeyPressed('T'))
		{
			mOrbit->SetActive(false);
			mNoClip->SetActive(true);
		}
	}
}