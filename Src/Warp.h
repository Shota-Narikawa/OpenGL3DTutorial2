/**
* @file Warp.h
*/
#ifndef WARP_H_INCLUDED
#define WARP_H_INCLUDED
#include "Actor.h"
#include "Mesh.h"

// êÊçsêÈåæ.
class MainGameScene;

/**
* Ç®ínë†ól.
*/
class WarpActor : public StaticMeshActor
{
public:
	WarpActor(const Mesh::FilePtr& m, const glm::vec3& pos, int id,
		MainGameScene* p);
	virtual ~WarpActor() = default;

	virtual void OnHit(const ActorPtr& other, const glm::vec3& p) override;

private:
	int id = 0;
	MainGameScene* parent = nullptr;
	bool isWarped = false;
	int health = 0;
};
using WarpActorPtr = std::shared_ptr<WarpActor>;

#endif // WARP_H_INCLUDED

