/**
* @file EnemyActor.h
*/
#ifndef ENEMYACTOR_H_INCLUDED
#define ENEMYACTOR_H_INCLUDED
#include "GLFWEW.h"
#include "SkeletalMeshActor.h"
#include "PlayerActor.h"
#include "Terrain.h"
#include "Audio\Audio.h"
#include <random>
#include <memory>

// 先行宣言
class MainGameScene;

/**
* 敵のアクター.
*/
class EnemyActor : public SkeletalMeshActor
{
public:
	EnemyActor(MainGameScene* mainGameScene, const Mesh::SkeletalMeshPtr& m, const std::string& name, int hp, const glm::vec3& pos,
		const glm::vec3& rot , const glm::vec3& scale = glm::vec3(1));
	virtual ~EnemyActor() = default;

	virtual void Update(float) override;

	virtual void EnemySpawn(ActorList enemy[], int count, float deltaTime, float timer, int spawn, int target, int n);
	virtual void EnemyTarget(ActorList& a, int id);

private:
	MainGameScene* mainGameScene = nullptr;

//アニメーション状態.
	enum class State {
		play,
		idle,		///<停止.
		run,		///<移動.
		jump,		///<ジャンプ.
		attack,		///<攻撃.
		shot,		///<ショット.

	};
	std::mt19937 rand;
	Mesh::Buffer meshBuffer;
	State state = State::idle; ///< 現在のアニメーション状態.
	bool isInAir = false;      ///< 空中判定フラグ.
	ActorPtr target;
	PlayerActorPtr player;
	ActorPtr boardingActor;   ///< 乗っているアクター.
	float moveSpeed = 5.0f;    ///< 移動速度.
	ActorPtr attackCollision;  ///< 攻撃判定.
	ActorPtr playerStatusUp;
	float attackTimer = 0;     ///< 攻撃時間.
	int health = 10;

	int enemyBlow = 0;
	int pJump = 1;
	int meshType = 0;

	float baseSpeed = 1.0f;
	bool isAttacking = false;		///<敵の攻撃.
	float attackingTimer = 3.0f;	//攻撃のタイマー.

	ActorPtr BulletCollision;
	float playerBulletTimer = 0;
	bool playerNoDamege = false;	//無敵状態.
	float noDamegeTimer = 0.0f;		//無敵時間.

	const Terrain::HeightMap* heightMap = nullptr;
};
using EnemyActorPtr = std::shared_ptr<EnemyActor>;

#endif // ENEMYACTOR_H_INCLUDED

