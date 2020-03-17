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
* �v���C���[�A�N�^�[.
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
	
	static int pLevel;		//�v���C���[���x��.
	static int pAbility;	//�A�r���e�B���x��.
	static int pExPoint;	//�o���l.
	static int pExCount;	//���x���A�b�v�܂�.
	static int pMP;			//MP.
	static int pHP;			//HP.
	static int maxHP;		//�ő�HP.
	static int maxMP;		//�ő�MP.
	static int playerID;	//�v���C���[�̃��b�V��.

private:

	void CheckRun(const GamePad& gamepad);
	void CheckJump(const GamePad& gamepad);
	void CheckAttack(const GamePad& gamepad);
	void CheckShot(const GamePad& gamepad);
	
		//�A�j���[�V�������.
		enum class State {
		idle,		///<��~.
		run,		///<�ړ�.
		jump,		///<�W�����v.
		attack,		///<�U��.
		shot,		///<�V���b�g.
		
	};
		Mesh::Buffer meshBuffer;
	State state = State::idle;	///< ���݂̃A�j���[�V�������.
	bool isInAir = false;		///< �󒆔���t���O.
	bool isAttack = false;		///<�U��������t���O.
	ActorPtr boardingActor;		///< ����Ă���A�N�^�[.
	float moveSpeed = 7.0f;		///< �ړ����x.
	ActorPtr attackCollision;	///< �U������.
	ActorPtr playerStatusUp;
	float attackTimer = 0;		///< �U������.
	float intervalTimer = 0;		///< �U������.
	int health = 10;

	int enemyBlow = 0;
	int pJump = 1;
	int pSpeed = 1;

	int meshType = 0;

	ActorPtr BulletCollision;
	float playerBulletTimer = 0;
	bool playerNoDamege = false;	//���G���.
	float noDamegeTimer = 0.0f;		//���G����.
	bool dashFrag = false;	//�S�u�����̃_�b�V���t���O.

	float skillTimerW = 0.0f , skillTimerE = 0.0f, skillTimerN = 0.0f;	//�X�L���C���^�[�o��.
	bool skillW = false, skillE = false, skillN = false;	//�X�L���C���^�[�o��.

	Audio::SoundPtr bgm;

	const Terrain::HeightMap* heightMap = nullptr;
};
using PlayerActorPtr = std::shared_ptr<PlayerActor>;

#endif // PLAYERACTOR_H_INCLUDED
