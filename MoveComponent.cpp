#include "MoveComponent.h"
#include <iostream>

MoveComponent::MoveComponent(class Actor* owner)
:Component(owner, 50)
,mAngularSpeed(0.0f)
,mForwardSpeed(0.0f)
{
	
}

void MoveComponent::Update(float deltaTime)
{
	// Moves the owner depending on their speed
	//Vector2 position = Vector2((float)(mOwner->GetForward().x * mForwardSpeed * ((double)deltaTime)), (float)(mOwner->GetForward().y * mForwardSpeed * (((double)deltaTime))));
	//mOwner->SetPosition(mOwner->GetPosition() + position);
	mOwner->SetPosition(mOwner->GetPosition() + Vector2(mDirection.x * mMoveSpeed * deltaTime, mDirection.y * mMoveSpeed * deltaTime));

}
