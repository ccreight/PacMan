#include "Actor.h"
#include "Game.h"
#include "Component.h"
#include <algorithm>

Actor::Actor(Game* game)
	:mGame(game)
	,mState(ActorState::Active)
	,mPosition(Vector2::Zero)
	,mScale(1.0f)
	,mRotation(0.0f)
{
	game->AddActor(this);

}

Actor::~Actor()
{
	mGame->RemoveActor(this);

	std::vector<Component*> temp = mComponents;

	for (unsigned int i = 0; i < temp.size(); i++) {
		delete temp[i];
	}

	mComponents.clear();

}

Vector2 Actor::GetForward() {

	// Calculates the forward position
	Vector2 position = Vector2(cos(mRotation), -1 * sin(mRotation));
	return position;

}

void Actor::Update(float deltaTime)
{
	// Updates active actors
	if (mState == ActorState::Active) {
		for (unsigned int x = 0; x < mComponents.size(); x++) {
			mComponents[x]->Update(deltaTime);
		}
		OnUpdate(deltaTime);
	}
}

void Actor::OnUpdate(float deltaTime)
{
}

void Actor::ProcessInput(const Uint8* keyState)
{
	// Processes input for active actors
	if (mState == ActorState::Active) {
		for (unsigned int x = 0; x < mComponents.size(); x++) {
			mComponents[x]->ProcessInput(keyState);
		}
		OnProcessInput(keyState);
	}

}

void Actor::OnProcessInput(const Uint8* keyState)
{
}

void Actor::AddComponent(Component* c)
{
	mComponents.emplace_back(c);
	std::sort(mComponents.begin(), mComponents.end(), [](Component* a, Component* b) {
		return a->GetUpdateOrder() < b->GetUpdateOrder();
	});
}
