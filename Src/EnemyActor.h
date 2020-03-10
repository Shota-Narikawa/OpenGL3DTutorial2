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
#include <memory>

/**
* 敵のアクター.
*/
class EnemyActor : public SkeletalMeshActor
{
public:
	EnemyActor(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
		const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0));
	virtual ~EnemyActor() = default;

	virtual void Update(float) override;
	virtual void OnHit(const ActorPtr&, const glm::vec3&);
	/*void Jump();
	void ProcessInput();*/
	void SetBoardingActor(ActorPtr);
	const ActorPtr& GetAttackCollision() const { return attackCollision; }

	static int pLevel;		//プレイヤーレベル.
	static int pAbility;	//アビリティレベル.
	static int pExPoint;	//経験値.
	static int pExCount;	//レベルアップまで.
	static int pMP;			//MP.
	static int pHP;			//HP.
	static int maxHP;		//最大HP.
	static int maxMP;		//最大MP.
	static int playerID;	//プレイヤーのメッシュ.
	static int pType;		//プレイヤーの属性.	0.無 1.火 2.水 3.風 4.土 5.光 6.闇

private:

	/*void CheckRun(const GamePad& gamepad);
	void CheckJump(const GamePad& gamepad);
	void CheckAttack(const GamePad& gamepad);
	void CheckShot(const GamePad& gamepad);
*/
	//アニメーション状態.
	enum class State {
		idle,		///<停止.
		run,		///<移動.
		jump,		///<ジャンプ.
		attack,		///<攻撃.
		shot,		///<ショット.

	};
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

#endif // PLAYERACTOR_H_INCLUDED

