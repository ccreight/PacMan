#include "AnimatedSprite.h"
#include "Actor.h"
#include "Game.h"
#include <iostream>

AnimatedSprite::AnimatedSprite(Actor* owner, int drawOrder)
	:SpriteComponent(owner, drawOrder)
{

	mOwner = owner;

}

void AnimatedSprite::Update(float deltaTime)
{
	if (mAnimName.empty()) {
		return;
	}

	if (!mIsPaused) {
		
		mAnimTimer += mAnimFPS * deltaTime;
		
		// Loops mAnimTimer around
		while (mAnimTimer >= mAnims[mAnimName].size()) {
			mAnimTimer -= mAnims[mAnimName].size();
		}

	}

	std::vector<SDL_Texture*> textures = mAnims[mAnimName];

	if (textures.size() == 1) {
		mOwner->GetComponent<SpriteComponent>()->SetTexture(textures[0]);
		return;
	}

	SDL_Texture* texture = textures[(int)floor(mAnimTimer)];

	mOwner->GetComponent<SpriteComponent>()->SetTexture(texture);

}

void AnimatedSprite::SetAnimation(const std::string& name, bool resetTimer)
{
	if (mAnimName != name)
	{
		mAnimName = name;
	}
	
	if (resetTimer)
	{
		mAnimTimer = 0.0f;
	}
}

void AnimatedSprite::AddAnimation(const std::string& name,
								  const std::vector<SDL_Texture*>& images)
{
	mAnims.emplace(name, images);
}
