#include "scene/components/COrbit.h"
#include "scene/components/CNoClip.h"
#include "scene/components/CTransform.h"
#include "scene/components/CCamera.h"
#include "scene/components/CPlayerControl.h"
#include "scene/components/Actor.h"
#include "Input.h"

namespace Utopian
{
	CPlayerControl::CPlayerControl(Actor* parent)
		: Component(parent)
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