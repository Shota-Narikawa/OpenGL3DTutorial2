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
* �G�̃A�N�^�[.
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

	static int pLevel;		//�v���C���[���x��.
	static int pAbility;	//�A�r���e�B���x��.
	static int pExPoint;	//�o���l.
	static int pExCount;	//���x���A�b�v�܂�.
	static int pMP;			//MP.
	static int pHP;			//HP.
	static int maxHP;		//�ő�HP.
	static int maxMP;		//�ő�MP.
	static int playerID;	//�v���C���[�̃��b�V��.
	static int pType;		//�v���C���[�̑���.	0.�� 1.�� 2.�� 3.�� 4.�y 5.�� 6.��

private:

	/*void CheckRun(const GamePad& gamepad);
	void CheckJump(const GamePad& gamepad);
	void CheckAttack(const GamePad& gamepad);
	void CheckShot(const GamePad& gamepad);
*/
	//�A�j���[�V�������.
	enum class State {
		idle,		///<��~.
		run,		///<�ړ�.
		jump,		///<�W�����v.
		attack,		///<�U��.
		shot,		///<�V���b�g.

	};
	Mesh::Buffer meshBuffer;
	State state = State::idle; ///< ���݂̃A�j���[�V�������.
	bool isInAir = false;      ///< �󒆔���t���O.
	ActorPtr target;
	PlayerActorPtr player;
	ActorPtr boardingActor;   ///< ����Ă���A�N�^�[.
	float moveSpeed = 5.0f;    ///< �ړ����x.
	ActorPtr attackCollision;  ///< �U������.
	ActorPtr playerStatusUp;
	float attackTimer = 0;     ///< �U������.
	int health = 10;

	int enemyBlow = 0;
	int pJump = 1;
	int meshType = 0;

	float baseSpeed = 1.0f;
	bool isAttacking = false;		///<�G�̍U��.
	float attackingTimer = 3.0f;	//�U���̃^�C�}�[.

	ActorPtr BulletCollision;
	float playerBulletTimer = 0;
	bool playerNoDamege = false;	//���G���.
	float noDamegeTimer = 0.0f;		//���G����.

	const Terrain::HeightMap* heightMap = nullptr;
};
using EnemyActorPtr = std::shared_ptr<EnemyActor>;

#endif // PLAYERACTOR_H_INCLUDED

