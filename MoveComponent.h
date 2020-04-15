#pragma once
#include "Component.h"
#include "Actor.h"

class MoveComponent : public Component
{
public:
	MoveComponent(class Actor* owner);

	// Update the move component
	void Update(float deltaTime) override;
	
	// Getters/setters
	float GetAngularSpeed() const { return mAngularSpeed; }
	float GetForwardSpeed() const { return mForwardSpeed; }
	void SetAngularSpeed(float speed) { mAngularSpeed = speed; }
	void SetForwardSpeed(float speed) { mForwardSpeed = speed; }
	void SetDirection(Vector2 d) {
		mDirection = d;
	}
	void SetMoveSpeed(float s) {
		mMoveSpeed = s;
	}
	Vector2 GetDirection() {
		return mDirection;
	}

private:
	// Angular speed (in radians/second)
	float mAngularSpeed = 0.0f;
	// Forward speed (in pixels/second)
	float mForwardSpeed = 0.0f;
	float mMoveSpeed = 0.0f;
	Vector2 mDirection = Vector2::Zero;
};
