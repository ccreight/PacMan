#include "GhostAI.h"
#include "Actor.h"
#include "Game.h"
#include "CollisionComponent.h"
#include "Game.h"
#include "PathNode.h"
#include "AnimatedSprite.h"
#include <SDL/SDL.h>
#include <unordered_map>
#include "Ghost.h"
#include "PacMan.h"
#include "Random.h"
#include <algorithm>
#include <iostream>
#include "MoveComponent.h"

GhostAI::GhostAI(class Actor* owner)
:Component(owner, 50)
{
	mGhost = static_cast<Ghost*>(owner);
}

void GhostAI::adjustPos(Vector2 v) {

	if (v.x < 0) {
		mGhost->GetComponent<AnimatedSprite>()->SetAnimation("left");
		mGhost->GetComponent<MoveComponent>()->SetDirection(mLeft);
	}
	else if (v.x > 0) {
		mGhost->GetComponent<AnimatedSprite>()->SetAnimation("right");
		mGhost->GetComponent<MoveComponent>()->SetDirection(mRight);
	}
	else if (v.y < 0) {
		mGhost->GetComponent<AnimatedSprite>()->SetAnimation("up");
		mGhost->GetComponent<MoveComponent>()->SetDirection(mUp);
	}
	else {
		mGhost->GetComponent<AnimatedSprite>()->SetAnimation("down");
		mGhost->GetComponent<MoveComponent>()->SetDirection(mDown);
	}

}

void GhostAI::Update(float deltaTime)
{
	mTimeSinceScatter += deltaTime;

	MoveComponent* move = mGhost->GetComponent<MoveComponent>();
	CollisionComponent* ghostCol = mGhost->getCollision();
	CollisionComponent* nodeCol = mNextNode->GetComponent<CollisionComponent>();
	AnimatedSprite* anim = mGhost->GetComponent<AnimatedSprite>();

	if (mState == State::Dead) {

		move->SetMoveSpeed(DEAD_SPEED);

		Vector2 offset;
		CollSide c = ghostCol->GetMinOverlap(nodeCol, offset);

		if (c != CollSide::None) {

			mGhost->SetPosition(mNextNode->GetPosition());

			mPrevNode = mNextNode;
			mCurrentNodeInPath--;
			mNextNode = mPath[mCurrentNodeInPath];

			if (mCurrentNodeInPath > 0) {
				mTargetNode = mPath[mCurrentNodeInPath];
			}

			if (mCurrentNodeInPath == 0) {

				GetPath(mNextNode, getNearestNode(mNextNode));
				mTargetNode = getNearestNode(mNextNode);
				mPrevNode = mNextNode;

				if (!mPath.empty()) {
					mNextNode = mPath[mPath.size() - 1];
				}
				else {
					mNextNode = getNearestNode(mNextNode);
				}

				mTargetNode = getNearestNode(mNextNode);
				mCurrentNodeInPath = mPath.size();

			}
			else if (mCurrentNodeInPath == -1) {
				GetPath(getNearestNode(mNextNode), mGhost->GetScatterNode());
				if (!mPath.empty()) {
					mNextNode = mPath[mPath.size() - 1];
				}
				else {
					mNextNode = mGhost->GetScatterNode();
				}

				mTargetNode = mGhost->GetScatterNode();

				mCurrentNodeInPath = mPath.size();
			}

			Vector2 diff = mNextNode->GetPosition() - mGhost->GetPosition();

			if (diff.x < 0) {
				anim->SetAnimation("deadleft");
				move->SetDirection(mLeft);
			}
			else if (diff.x > 0) {
				anim->SetAnimation("deadright");
				move->SetDirection(mRight);
			}
			else if (diff.y < 0) {
				anim->SetAnimation("deadup");
				move->SetDirection(mUp);
			}
			else {
				anim->SetAnimation("deaddown");
				move->SetDirection(mDown);
			}

		}

		if (mNextNode == mGhost->GetSpawnNode()) {
			mState = State::Scatter;
			Start(mNextNode);
		}

		return;

	}
	
	if (GetState() == State::Frightened) {
		
		mTimeSinceFright += deltaTime;

		if (mTimeSinceFright < 5.0f) {
			anim->SetAnimation("scared0");
		}

		else {
			anim->SetAnimation("scared1");
		}

		if (mTimeSinceFright > 7.0f) {

			mTimeSinceFright = 0.0f;
			mState = State::Scatter;
			GetPath(mNextNode, mGhost->GetScatterNode());
			mCurrentNodeInPath = mPath.size() - 1;

			Vector2 diff = mNextNode->GetPosition() - mGhost->GetPosition();
			adjustPos(diff);

			return;
		}

		if (mFirstFright) {

			//mGhost->SetPosition(mNextNode->GetPosition());

			if (mNextNode->GetType() == PathNode::Type::Tunnel) {
				for (PathNode* n : mNextNode->mAdjacent) {
					if (n->GetType() == PathNode::Type::Tunnel) {
						mCurrentNodeInPath--;
						mGhost->SetPosition(n->GetPosition());
						break;
					}
				}
			}

			//mNextNode = GetRandomChild(mNextNode);
			Vector2 diff = mNextNode->GetPosition() - mGhost->GetPosition();
			
			if (diff.x < 0) {
				move->SetDirection(mLeft);
			}
			else if (diff.x > 0) {
				move->SetDirection(mRight);
			}
			else if (diff.y < 0) {
				move->SetDirection(mUp);
			}
			else {
				move->SetDirection(mDown);
			}

			mTargetNode = mNextNode;
			mFirstFright = false;

		}

		Vector2 offset;
		CollSide c = ghostCol->GetMinOverlap(nodeCol, offset);

		if (c != CollSide::None) {

			mGhost->SetPosition(mNextNode->GetPosition());

			if (mNextNode->GetType() == PathNode::Type::Tunnel) {
				for (PathNode* n : mNextNode->mAdjacent) {
					if (n->GetType() == PathNode::Type::Tunnel) {
						mCurrentNodeInPath--;
						mGhost->SetPosition(n->GetPosition());
						mNextNode = n;
						break;
					}
				}
			}

			mPrevNode = mNextNode;
			mNextNode = GetRandomChild(mPrevNode);

			GetPath(mPrevNode, mNextNode);
			mTargetNode = mNextNode;

			Vector2 diff = mNextNode->GetPosition() - mGhost->GetPosition();
			
			if (diff.x < 0) {
				move->SetDirection(mLeft);
			}
			else if (diff.x > 0) {
				move->SetDirection(mRight);
			}
			else if (diff.y < 0) {
				move->SetDirection(mUp);
			}
			else {
				move->SetDirection(mDown);
			}

		}

		return;

	}
	else {
		mFirstFright = true;
	}

	if (GetState() == State::Scatter && mTimeSinceScatter < 5.0f) {

		mFirstIter = true;

		move->SetMoveSpeed(SCATTER_SPEED);

		Vector2 offset;
		CollSide c = ghostCol->GetMinOverlap(nodeCol, offset);

		if (c != CollSide::None) {

			mGhost->SetPosition(mNextNode->GetPosition());

			if (mNextNode->GetType() == PathNode::Type::Tunnel) {
				for (PathNode* n : mNextNode->mAdjacent) {
					if (n->GetType() == PathNode::Type::Tunnel) {
						mCurrentNodeInPath--;
						mGhost->SetPosition(n->GetPosition());
						break;
					}
				}
			}

			mPrevNode = mNextNode;
			mCurrentNodeInPath--;

			if(mCurrentNodeInPath > -1)
				mNextNode = mPath[mCurrentNodeInPath];

			//mTargetNode = mPath[0];
			mTargetNode = mGhost->GetScatterNode();

			if (mCurrentNodeInPath == 0) {

				GetPath(mNextNode, getNearestNode(mNextNode));
				mTargetNode = getNearestNode(mNextNode);
				mPrevNode = mNextNode;

				if (!mPath.empty()) {
					mNextNode = mPath[mPath.size() - 1];
				}
				else {
					mNextNode = getNearestNode(mNextNode);
				}

				mTargetNode = getNearestNode(mNextNode);
				mCurrentNodeInPath = mPath.size();

			}
			else if (mCurrentNodeInPath <= -1) {
				GetPath(getNearestNode(mNextNode), mGhost->GetScatterNode());
				if (!mPath.empty()) {
					mNextNode = mPath[mPath.size() - 1];
				}
				else {
					mNextNode = mGhost->GetScatterNode();
				}

				mTargetNode = mGhost->GetScatterNode();

				mCurrentNodeInPath = mPath.size();
				std::cout << mPath.size() << std::endl;
			}

			Vector2 diff = mNextNode->GetPosition() - mGhost->GetPosition();
			adjustPos(diff);

		}

	}

	if (mTimeSinceScatter > 5.0f) {

		move->SetMoveSpeed(SCATTER_SPEED);

		if (mTimeSinceScatter > 25.0f) {
			mFirstIter = true;
			mTimeSinceScatter = 0.0f;
			mState = State::Scatter;
			Start(mNextNode);

			if (mPrevNode->GetPosition().x < mNextNode->GetPosition().x) {
				anim->SetAnimation("right");
				move->SetDirection(mRight);
			}
			else if (mPrevNode->GetPosition().x > mNextNode->GetPosition().x) {
				anim->SetAnimation("left");
				move->SetDirection(mLeft);
			}
			else if (mPrevNode->GetPosition().y < mNextNode->GetPosition().y) {
				anim->SetAnimation("down");
				move->SetDirection(mDown);
			}
			else {
				anim->SetAnimation("up");
				move->SetDirection(mUp);
			}
			return;
		}

		mState = State::Chase;

		if(mFirstIter){
			GetPath(mPrevNode, getTarget(mGhost));
			mCurrentNodeInPath = mPath.size() - 1;
			mFirstIter = false;
			mPrevNode = mNextNode;
			//mCurrentNodeInPath--;
			mNextNode = mPath[mCurrentNodeInPath];
			std::cout << "setting target" << std::endl;
			mTargetNode = getTarget(mGhost);

			Vector2 diff = mNextNode->GetPosition() - mGhost->GetPosition();
			adjustPos(diff);

			return;

		}

		Vector2 offset;
		CollSide c = ghostCol->GetMinOverlap(nodeCol, offset);

		if (c != CollSide::None) {

			GetPath(mNextNode, getTarget(mGhost));
			mCurrentNodeInPath = mPath.size() - 1;
			mFirstIter = false;
			mPrevNode = mNextNode;
			mNextNode = mPath[mCurrentNodeInPath];
			std::cout << "setting target" << std::endl;
			mTargetNode = getTarget(mGhost);


			mGhost->SetPosition(mNextNode->GetPosition());

			mPrevNode = mNextNode;
			mCurrentNodeInPath--;
			mNextNode = mPath[mCurrentNodeInPath];

			if (mNextNode->GetType() == PathNode::Type::Tunnel) {
				for (PathNode* n : mNextNode->mAdjacent) {
					if (n->GetType() == PathNode::Type::Tunnel) {
						mCurrentNodeInPath--;
						mGhost->SetPosition(n->GetPosition());
						break;
					}
				}
			}

			mTargetNode = mPath[0];

			if (mCurrentNodeInPath == 0) {

				GetPath(mNextNode, getNearestNode(mNextNode));

				if (!mPath.empty()) {
					mNextNode = mPath[mPath.size() - 1];
				}
				else {
					mNextNode = mGhost->GetScatterNode();
				}

				mTargetNode = mGhost->GetScatterNode();
				mCurrentNodeInPath = mPath.size();

			}

			else if (mCurrentNodeInPath == -1) {

				GetPath(getNearestNode(mNextNode), mGhost->GetScatterNode());
				if (!mPath.empty()) {
					mNextNode = mPath[mPath.size() - 1];
				}
				else {
					mNextNode = mGhost->GetScatterNode();
				}

				mTargetNode = mGhost->GetScatterNode();

				mCurrentNodeInPath = mPath.size();
				std::cout << mPath.size() << std::endl;

			}

			Vector2 diff = mNextNode->GetPosition() - mGhost->GetPosition();
			adjustPos(diff);

		}

	}

}

PathNode* GhostAI::GetRandomChild(PathNode* n) {

	PathNode* node = nullptr;
		
	while (node == nullptr) {
			
		int rand_child = rand() % n->mAdjacent.size();
		PathNode* temp = n->mAdjacent[rand_child];

		if (temp->GetType() == PathNode::Type::Ghost) {
			continue;
		}

		if (!(temp->GetPosition().x > n->GetPosition().x) && mGhost->GetComponent<MoveComponent>()->GetDirection().x == mLeft.x) {
			node = temp;
			return node;
		}

		else if (!(temp->GetPosition().x < n->GetPosition().x) && mGhost->GetComponent<MoveComponent>()->GetDirection().x == mRight.x) {
			node = temp;
			return node;
		}
			
		else if (!(temp->GetPosition().y > n->GetPosition().y) && mGhost->GetComponent<MoveComponent>()->GetDirection().y == mUp.y) {
			node = temp;
			return node;
		}

		else if (!(temp->GetPosition().y < n->GetPosition().y) && mGhost->GetComponent<MoveComponent>()->GetDirection().y == mDown.y) {
			node = temp;
			return node;
		}

	}

	return node;

}

PathNode* GhostAI::getTarget(Ghost* g) {

	if (g->GetType() == Ghost::Type::Blinky) {
		return g->GetGame()->mPlayer->GetPrevNode();
	}
	else if (g->GetType() == Ghost::Type::Pinky) {

		//Given a Vector2, return a node
		PathNode* closest = g->GetGame()->mPathNodes[0];
		Vector2 v = g->GetGame()->mPlayer->GetPointInFrontOf(80.0f);
		float min = (float)INT_MAX;

		for (size_t i = 0; i < g->GetGame()->mPathNodes.size(); i++) {

			float dist = abs((v - g->GetGame()->mPathNodes[i]->GetPosition()).Length());

			if (dist < min) {
				min = dist;
				closest = g->GetGame()->mPathNodes[i];
			}

		}

		return closest;

	}

	else if (g->GetType() == Ghost::Type::Inky) {

		//Given a Vector2, return a node
		PathNode* closest = g->GetGame()->mPathNodes[0];
		Vector2 v = g->GetGame()->mPlayer->GetPointInFrontOf(40.0f);
		Vector2 blinkyV = g->GetGame()->mGhosts[0]->GetPosition() + 2 * (g->GetGame()->mGhosts[0]->GetPosition() - v);
		float min = (float)INT_MAX;

		for (size_t i = 0; i < g->GetGame()->mPathNodes.size(); i++) {

			float dist = abs((blinkyV - g->GetGame()->mPathNodes[i]->GetPosition()).Length());

			if (dist < min) {
				min = dist;
				closest = g->GetGame()->mPathNodes[i];
			}

		}

		return closest;

	}
	else {

		float dist = abs((g->GetPosition() - g->GetGame()->mPlayer->GetPosition()).Length());
		
		if (dist > 150.0f) {
			return g->GetGame()->mPlayer->GetPrevNode();
		}
		else {
			return g->GetScatterNode();
		}

	}

	return g->GetScatterNode();

}

PathNode* GhostAI::getNearestNode(PathNode* n) {

	float minHeuristic = FLT_MAX;
	PathNode* smallest = nullptr;

	for (PathNode* node : n->mAdjacent) {
		float l = abs((node->GetPosition() - n->GetPosition()).Length());
		if (minHeuristic == LONG_MAX || smallest == nullptr || l < minHeuristic) {
			smallest = node;
			minHeuristic = l;
		}
	}

	return smallest;

}

void GhostAI::Frighten()
{
	// TODO: Implement
	
	if (mGhost->IsDead()) {
		return;
	}

	if (mGhost->IsFrightened()) {
		mTimeSinceFright = 0.0f;
		mFirstFright = true;
	}

	mGhost->GetComponent<MoveComponent>()->SetDirection(-1 * mGhost->GetComponent<MoveComponent>()->GetDirection());
	mGhost->GetComponent<MoveComponent>()->SetForwardSpeed(FRIGHTENED_SPEED);

	mState = State::Frightened;

}

void GhostAI::Start(PathNode* startNode)
{

	mOwner->SetPosition(startNode->GetPosition());
	mState = State::Scatter;
	mPrevNode = nullptr;
	mNextNode = nullptr;
	mTargetNode = nullptr;
	mPath.clear();

	mTimeSinceScatter = 0.0f;
	mTimeSinceFright = 0.0f;
	mFirstIter = true;
	mFirstFright = true;

	bool hasPath = GetPath(startNode, mGhost->GetScatterNode());
	mPrevNode = startNode;

	if (!mPath.empty()) {
		mNextNode = mPath[mPath.size() - 1];
	}
	else {
		mNextNode = mGhost->GetScatterNode();
	}

	mTargetNode = mGhost->GetScatterNode();

	mCurrentNodeInPath = mPath.size() - 1;
	mGhost->GetComponent<MoveComponent>()->SetDirection(mUp);

}

void GhostAI::Die()
{
	
	mState = State::Dead;
	mGhost->GetComponent<AnimatedSprite>()->SetAnimation("deaddown");

	bool hasPath = GetPath(mNextNode, mGhost->GetSpawnNode());
	mPrevNode = mNextNode;

	if (!mPath.empty()) {
		mNextNode = mPath[mPath.size() - 1];
	}
	else {
		mNextNode = mGhost->GetScatterNode();
	}

	mTargetNode = mGhost->GetSpawnNode();

	mCurrentNodeInPath = mPath.size() - 1;

}

// HELPFUL INFO ON SLIDES
// g will store the current distance we've gone
// h is storing the distance from currNode to the targetNode
bool GhostAI::GetPath(PathNode* startNode, PathNode* targetNode) {

	// Check if the start node is equal to the target node
	if (startNode == targetNode) {
		return false;
	}

	mPath.clear();

	PathNode* currNode = startNode;

	// Open set and closed set, where open set also stores heuristic
	std::unordered_map<PathNode*, float> openSet;
	std::vector<PathNode*> closedSet;

	// This will be mPath
	std::unordered_map<PathNode*, PathNode*> path;

	// Stores the gValue of a node
	std::unordered_map<PathNode*, float> gValue;

	gValue[currNode] = 0.0f; // haven't gone anywhere

	// Heuristic is Euclidean distance
	openSet[currNode] = getMinDistance(currNode, targetNode);
	
	// There are more nodes to try
	while (!openSet.empty()) {

		PathNode* curr = nullptr;
		float minHeuristic = -1;

		// Gets us the min cost node of all potential nodes to visit
		for (auto item : openSet) {
			if (item.second < minHeuristic || minHeuristic == -1) {
				curr = item.first;
				minHeuristic = item.second;
			}
		}

		if (curr == targetNode) {
			mPath = formPath(path, curr);
			break;
		}

		openSet.erase(curr);
		
		closedSet.push_back(curr);

		for (PathNode* n : curr->mAdjacent) {

			if (std::find(closedSet.begin(), closedSet.end(), n) != closedSet.end()) {
				continue;
			}

			float g = gValue[curr] + abs((n->GetPosition() - curr->GetPosition()).Length());

			// If g is new or better than the old one
			if (gValue.find(n) == gValue.end() || g < gValue[n]) {

				path[n] = curr;
				gValue[n] = g;

				if (openSet.find(n) == openSet.end()) {
					// Total value: distance between nodes so far and distance to end
					openSet[n] = gValue[n] + getMinDistance(n, targetNode);
				}

			}

		}

	}

	return false;

}

float GhostAI::getMinDistance(class PathNode* startNode, class PathNode* targetNode) {

	float val1 = (startNode->GetPosition() - targetNode->GetPosition()).Length();

	float val2 = (startNode->GetPosition() - mOwner->GetGame()->mTunnelRight->GetPosition()).Length();
	val2 += (mOwner->GetGame()->mTunnelRight->GetPosition() - targetNode->GetPosition()).Length();

	float val3 = (startNode->GetPosition() - mOwner->GetGame()->mTunnelLeft->GetPosition()).Length();
	val3 += (mOwner->GetGame()->mTunnelLeft->GetPosition() - targetNode->GetPosition()).Length();

	return Math::Min(val1, Math::Min(val2, val3));

}

std::vector<PathNode*> GhostAI::formPath(std::unordered_map<PathNode*, PathNode*> p, PathNode* c) {

	std::vector<PathNode*> path;
	path.push_back(c);

	while (p.find(c) != p.end()) {
		c = p[c];
		path.push_back(c);
	}

	return path;

}

void GhostAI::DebugDrawPath(SDL_Renderer* render)
{
	// Draw a rectangle at the target node
	if (mTargetNode != nullptr)
	{
		const int SIZE = 16;
		SDL_Rect r;
		r.x = static_cast<int>(mTargetNode->GetPosition().x) - SIZE / 2;
		r.y = static_cast<int>(mTargetNode->GetPosition().y) - SIZE / 2;
		r.w = SIZE;
		r.h = SIZE;
		SDL_RenderDrawRect(render, &r);
	}

	// Line from ghost to next node
	if (mNextNode != nullptr)
	{
		SDL_RenderDrawLine(render,
			static_cast<int>(mOwner->GetPosition().x),
			static_cast<int>(mOwner->GetPosition().y),
			static_cast<int>(mNextNode->GetPosition().x),
			static_cast<int>(mNextNode->GetPosition().y));
	}

	// Exit if no path
	if (mPath.empty())
	{
		return;
	}

	// Line from next node to subsequent on path
	SDL_RenderDrawLine(render,
		static_cast<int>(mNextNode->GetPosition().x),
		static_cast<int>(mNextNode->GetPosition().y),
		static_cast<int>(mPath.back()->GetPosition().x),
		static_cast<int>(mPath.back()->GetPosition().y));

	// Lines for rest of path
	for (size_t i = 0; i < mPath.size() - 1; i++)
	{
		SDL_RenderDrawLine(render,
			static_cast<int>(mPath[i]->GetPosition().x),
			static_cast<int>(mPath[i]->GetPosition().y),
			static_cast<int>(mPath[i + 1]->GetPosition().x),
			static_cast<int>(mPath[i + 1]->GetPosition().y));
	}
}
