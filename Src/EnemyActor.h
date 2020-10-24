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

// ��s�錾
class MainGameScene;

/**
* �G�̃A�N�^�[.
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

//�A�j���[�V�������.
	enum class State {
		play,
		idle,		///<��~.
		run,		///<�ړ�.
		jump,		///<�W�����v.
		attack,		///<�U��.
		shot,		///<�V���b�g.

	};
	std::mt19937 rand;
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

#endif // ENEMYACTOR_H_INCLUDED

