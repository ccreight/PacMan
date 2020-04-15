#include "CollisionComponent.h"
#include "Actor.h"
#include <iostream>
#include <math.h> 

CollisionComponent::CollisionComponent(class Actor* owner)
:Component(owner)
,mWidth(0.0f)
,mHeight(0.0f)
{
	mOwner = owner;
}

CollisionComponent::~CollisionComponent()
{
	
}

bool CollisionComponent::Intersect(const CollisionComponent* other)
{
	// Checks if there is no possible intersection
	if ((this->GetMin().x > other->GetMax().x || this->GetMax().x < other->GetMin().x) ||
		(this->GetMin().y > other->GetMax().y || this->GetMax().y < other->GetMin().y)){
		return false;
	}

	return true;
}

Vector2 CollisionComponent::GetMin() const
{
	if (mOwner != nullptr) {

		// Calculates x coord of minimum
		float minX = (float)(mOwner->GetPosition().x);
		minX -= (float)(this->GetWidth() * mOwner->GetScale()) / 2.0f;

		// Calculates y coord of minimum
		float minY = (float)(mOwner->GetPosition().y);
		minY -= (float)(this->GetHeight() * mOwner->GetScale()) / 2.0f;

		Vector2 min(minX, minY);

		return min;

	}
	return Vector2::Zero;
}

Vector2 CollisionComponent::GetMax() const
{
	if (mOwner != nullptr) {

		// Calculates x coord of maximum
		float maxX = (float)(mOwner->GetPosition().x);
		maxX += (float)(this->GetWidth() * mOwner->GetScale()) / 2.0f;

		// Calculates y coord of maximum
		float maxY = (float)(mOwner->GetPosition().y);
		maxY += (float)(this->GetHeight() * mOwner->GetScale()) / 2.0f;

		Vector2 max(maxX, maxY);

		return max;
	}

	return Vector2::Zero;
}

const Vector2& CollisionComponent::GetCenter() const
{
	return mOwner->GetPosition();
}

CollSide CollisionComponent::GetMinOverlap(
	const CollisionComponent* other, Vector2& offset)
{
	if (mOwner == nullptr) {
		offset = Vector2::Zero;
		return CollSide::None;
	}

	// Returns no collision if "this" doesn't intersect with "other"
	if (!this->Intersect(other)) {
		offset = Vector2::Zero;
		return CollSide::None;
	}

	float overlapLeft, overlapRight, overlapTop, overlapBottom;
	float thisMinX, thisMinY, thisMaxX, thisMaxY;
	float otherMinX, otherMinY, otherMaxX, otherMaxY;

	// Sets coordinates of "this"
	thisMinX = this->GetMin().x;
	thisMinY = this->GetMin().y;
	thisMaxX = this->GetMax().x;
	thisMaxY = this->GetMax().y;

	// Sets coordinates of "other"
	otherMinX = other->GetMin().x;
	otherMinY = other->GetMin().y;
	otherMaxX = other->GetMax().x;
	otherMaxY = other->GetMax().y;
	
	// Calculates overlap
	overlapLeft = abs(thisMaxX - otherMinX);
	overlapRight = abs(thisMinX - otherMaxX);
	overlapTop = abs(thisMaxY - otherMinY);
	overlapBottom = abs(thisMinY - otherMaxY);

	// Finds minimum overlap
	float minValue = fmin(overlapLeft, fmin(overlapRight, fmin(overlapTop, overlapBottom)));

	// Returns where overlap is + sets offset value
	if (minValue == overlapLeft) {
		offset.x = -1 * overlapLeft;
		return CollSide::Left;
	}
	else if (minValue == overlapRight) {
		offset.x = overlapRight;
		return CollSide::Right;
	}
	else if (minValue == overlapTop) {
		offset.y = -1 * overlapTop;
		return CollSide::Top;
	}
	else {
		offset.y = overlapBottom;
		return CollSide::Bottom;
	}

}
