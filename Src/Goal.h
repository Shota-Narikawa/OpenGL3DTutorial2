/**
* @file Goal.h
*/
#ifndef GOAL_H_INCLUDED
#define GOAL_H_INCLUDED
#include "Actor.h"
#include "Mesh.h"

// êÊçsêÈåæ.
class MainGameScene;

/**
* ÉSÅ[Éã.
*/
class GoalActor : public StaticMeshActor
{
public:
	GoalActor(const Mesh::FilePtr& m, const glm::vec3& pos, int id,
		MainGameScene* p);
	virtual ~GoalActor() = default;

	virtual void OnHit(const ActorPtr& other, const glm::vec3& p) override;

private:
	int id = 0;
	MainGameScene* parent = nullptr;
	bool isCointed = false;
	
};
using GoalActorPtr = std::shared_ptr<GoalActor>;

#endif // COIN_H_INCLUDED

