/**
* @file PlayerActor.h
*/
#ifndef PLAYERACTOR_H_INCLUDED
#define PLAYERACTOR_H_INCLUDED
#include "GLFWEW.h"
#include "SkeletalMeshActor.h"
#include "Terrain.h"
#include "Audio\Audio.h"
#include <memory>

/**
* プレイヤーアクター.
*/
class PlayerActor : public SkeletalMeshActor
{
public:
	PlayerActor(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
	const glm::vec3& pos, const glm::vec3& rot = glm::vec3(0));
	virtual ~PlayerActor() = default;

	virtual void Update(float) override;
	virtual void OnHit(const ActorPtr&, const glm::vec3&);
	void Jump();
	void ProcessInput();
	void SetBoardingActor(ActorPtr);
	const ActorPtr& GetAttackCollision() const { return attackCollision; }
	const ActorPtr& GetPlayerStatusUp() const { return playerStatusUp; }
	void StatusUp();
	
	static int pLevel;		//プレイヤーレベル.
	static int pAbility;	//アビリティレベル.
	static int pExPoint;	//経験値.
	static int pExCount;	//レベルアップまで.
	static int pMP;			//MP.
	static int pHP;			//HP.
	static int maxHP;		//最大HP.
	static int maxMP;		//最大MP.
	static int playerID;	//プレイヤーのメッシュ.

private:

	void CheckRun(const GamePad& gamepad);
	void CheckJump(const GamePad& gamepad);
	void CheckAttack(const GamePad& gamepad);
	void CheckShot(const GamePad& gamepad);
	
		//アニメーション状態.
		enum class State {
		idle,		///<停止.
		run,		///<移動.
		jump,		///<ジャンプ.
		attack,		///<攻撃.
		shot,		///<ショット.
		
	};
		Mesh::Buffer meshBuffer;
	State state = State::idle;	///< 現在のアニメーション状態.
	bool isInAir = false;		///< 空中判定フラグ.
	bool isAttack = false;		///<攻撃中判定フラグ.
	ActorPtr boardingActor;		///< 乗っているアクター.
	float moveSpeed = 7.0f;		///< 移動速度.
	ActorPtr attackCollision;	///< 攻撃判定.
	ActorPtr playerStatusUp;
	float attackTimer = 0;		///< 攻撃時間.
	float intervalTimer = 0;		///< 攻撃時間.
	int health = 10;

	int enemyBlow = 0;
	int pJump = 1;
	int pSpeed = 1;

	int meshType = 0;

	ActorPtr BulletCollision;
	float playerBulletTimer = 0;
	bool playerNoDamege = false;	//無敵状態.
	float noDamegeTimer = 0.0f;		//無敵時間.
	bool dashFrag = false;	//ゴブリンのダッシュフラグ.

	float skillTimerW = 0.0f , skillTimerE = 0.0f, skillTimerN = 0.0f;	//スキルインターバル.
	bool skillW = false, skillE = false, skillN = false;	//スキルインターバル.

	Audio::SoundPtr bgm;

	const Terrain::HeightMap* heightMap = nullptr;
};
using PlayerActorPtr = std::shared_ptr<PlayerActor>;

#endif // PLAYERACTOR_H_INCLUDED
