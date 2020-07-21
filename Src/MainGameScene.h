/**
*@file MainGameScene.h
*/

#ifndef MAINGAMESCENE_H_INCLUDED
#define MAINGAMESCENE_H_INCLUDED
#include "Scene.h"
#include "Sprite.h"
#include "Font.h"
#include "Mesh.h"
#include "Terrain.h"
#include "Actor.h"
#include "PlayerActor.h"
#include "EnemyActor.h"
#include "Shader.h"
#include "Audio\Audio.h"
#include "Light.h"
#include "FramebufferObject.h"
#include "Particle.h"
#include "TextWindow.h"
#include "Camera.h"
#include <random>
#include <vector>

/**
*���C���Q�[�����.
*/

class MainGameScene : public Scene {

public:
	MainGameScene() : Scene("MainGameScene") {}
	virtual ~MainGameScene() = default;

	virtual bool Initialize() override;
	virtual void ProcessInput() override;
	virtual void Update(float) override;
	virtual void Render() override;
	virtual void Finalize() override {}

	bool HandleJizoEffects(int id, const glm::vec3& pos);
	bool HandleWarpEffects(int id, const glm::vec3& pos);
	bool HandleCoinEffects(int id, const glm::vec3& pos);

	void selectUI(int a, int b, int c, int d, int e, int f);
	void skSelectUI(int a, int b, int c, int d, int e, int f);
	void EnemyTargetID(ActorList& a);
	void EnemySpawn();
	void EnemyAI(float, ActorList& x, int a, int b);
	void EnemyDetectCollision(int i);

	static int StageNo;
	static bool StClearedE, StClearedW, StClearedS, StClearedN;	///<�X�e�[�g�ڍs�\���.

private:
	void RenderMesh(const Collision::Frustum*, Mesh::DrawType);

	int jizoId = -1; ///< ���ݐ퓬���̂��n���l��ID.
	bool achivements[4] = { false, false, false, false }; ///< �G�������.
	std::mt19937 rand;
	bool flag = false;
	std::vector<Sprite> sprites;
	SpriteRenderer spriteRenderer;
	FontRenderer fontRenderer;
	Audio::SoundPtr bgm;
	Mesh::Buffer meshBuffer;
	Terrain::HeightMap heightMap;
	PlayerActorPtr player;
	EnemyActorPtr enemy;
	ActorList enemies[4];
	ActorList trees;
	ActorList objects;
	ActorList walls;
	ActorList effects;
	ActorList objectives;
	ActorList items[4];
	ActorList upItem[3];
	ActorList warp[4];
	ActorList bullet[10];
	ActorList defencePoint;

	LightBuffer lightBuffer;
	ActorList lights;

	ParticleSystem particleSystem;
	TextWindow textWindow;

	FramebufferObjectPtr fboMain;
	FramebufferObjectPtr fboDepthOfField;
	FramebufferObjectPtr fboBloom[6][2];
	FramebufferObjectPtr fboShadow;

	Shader::Program progSimple;
	Shader::Program progLighting;

	//�Q�[�����.
	enum class State {
		play,		//�ʏ�v���C��.
		menu,		//�ꎞ��~���.
		result,		//���U���g���.
		load,		//���[�h���.
		select,		//Yes or No���.
		//���j���[���.
		pMenu, cMenu, sMenu, gameClose, menuClose
	};

	State state = State::play;

	//�^�C�}�[�n.
	float nextStageTimer = 0.0f;								//���̃X�e�[�W�A�ʂ̃X�e�[�W�ڍs���̃^�C�}�[.
	float loadTimer = 0.0f;										//�V�[���ڍs���̃��[�h��ʃ^�C�}�[.
	float actionWaitTimer = 0.0f;								//�U���̃^�C�}�[.
	float timer = 0.0f;											//�V�[���^�C�}�[.
	float clearTimer = 0, overTimer = 0.0f;						//�Q�[���N���A�A�Q�[���I�[�o�̃^�C�}�[.
	float playerBulletTimerA = 0, playerBulletTimerB = 0,
		playerBulletTimerC = 0, playerBulletTimerD = 0.0f;		//�e�̃^�C�}�[.
	float attackingTimer = 3.0f;								//�G�̍U���^�C�}�[.
	float enemyPopTimerA = 0, enemyPopTimerB = 0,
		enemyPopTimerC = 0.0f, enemyPopTimerD = 0.0f;			//�G�̏o�����ԊԊu.
	float itemTimerA = 0, itemTimerB = 0, itemTimerC = 0.0f;	//�A�C�e���o���^�C�}�[.
	float eIntTimer = 0, wIntTimer = 0, 
		sIntTimer = 0, nIntTimer = 0.0f;						//�e�U���̃C���^�[�o��.
	float allTimer = 0.0f;										//�\���p�^�C�}�[.
	float particleTimerA = 0, particleTimerB = 0;				//�U���p�[�e�B�N������^�C�}�[.
	float walkParticleTimer = 0;								//�����p�[�e�B�N������^�C�}�[.
	float bombTimer = 0;										//���e�̃^�C�}�\.

	//ID.
	int warpID = -1;
	int CoinID = -1;
	int enemyID = -1;

	//�J�E���g��^�C�v.
	int selectCount = 0;	//�I���A�C�R���p.
	int selectCount2 = 0;	//�I���A�C�R���p.
	int selectCount3 = 0;	////�I���A�C�R���p.
	int stageChage = 0;		//�ǂ̎�����.
	float defenceLine = 100;	//�O�ɂȂ�ƃQ�[���I�[�o.

	//�G�̏��.
	int enemySpawn = 100;	//��x�ɗN���G�̏o����.
	int enemyBlow = 0;		//�G��|������.
	int enemyStock = 1;		//�G�̎c��.
	int enemyAttack = 0;	//�G�̍U����.
	float baseSpeed = 1.0f;	//�G�̈ړ��X�s�[�h.
	int randTarget = 0;		//�����_���Ń^�[�Q�b�g�擾.


	float skillIntervalMax = 5.0f;

	glm::vec3 comammdSetPos[4] =					//�D���ȏꏊ�ɃA�C�R�����Z�b�g����ׂ̔z��.(�ォ�玞�v���)
	{ glm::vec3(500, -150, 0),glm::vec3(580, -230, 0),glm::vec3(500, -310, 0),glm::vec3(420, -230, 0) };
	int eCommandA = 0, wCommandA = 0, sCommandA = 0, nCommandA = 0;	//�\���̃X�L��.
	int eCommandB = 0, wCommandB = 0, sCommandB = 0, nCommandB = 0;	//�����̃X�L��.
	int skComAttack = 0;							//�����ɉ����čU������X�L�����قȂ�.
	int skComCount = 0;								//�X�L���\�̎��̃J�E���g.

	struct ItemCount {
		int itemHP = 0;			//HP�A�b�v.
		int itemMP = 0;			//MP�A�b�v.
		int itemAll = 0;		//������.
		int attackUP = 0;		//�U���A�b�v.
		int blockUP = 0;		//�h��A�b�v.
		int muteki = 0;			//���G���.
	};

	ItemCount itemC;

	//�t���O�n.
	bool gameOverFlag = false;						///<�Q�[���I�[�o�[���.
	bool gameClearFlag = false;						///<�Q�[���N���A���.
	bool nextStateFlag = false;						///<�X�e�[�g�ڍs�\���.

	bool selectIconFlag = false;					///<�Z���N�g�A�C�R��.
	bool shotTimerFlagA = false, shotTimerFlagB = false, 
		shotTimerFlagC = false;						///<�U���Ń^�C�}�[�𓮂����t���O.
	bool chargeShotFlagA, chargeShotFlagB = false;	///<���ߍU���p�t���O.
	bool isAttacking = false;						///<�G�̍U��.
	bool bulletFragA, bulletFragB = false;			///<���͂ɏo��.
	bool eInterval = false, wInterval = false,
		sInterval = false, nInterval = false;		///<�e�{�^���̃C���^�[�o����Ԃ̃t���O.
	bool eCommand = false, wCommand = false,
		sCommand = false, nCommand = false;			///<�ǂ̍U���{�^���������Ă��邩.

	bool iconUpFlag = false, iconDownFlag = false;	//�A�C�R��������.
	bool mHouseFlag = false;						//�����X�^�[�n�E�X.
	bool defenceFlag = false;						//�h�q���C�����U������Ă��邩.

	bool eventFrag = false;							//�Z���t�C�x���g�\���t���O.

	bool cameraFar = false;			//�J�����𗣂�.
	bool cameraNear = false;		//�J�������񂹂�.
	bool gamePadText = false;		//�Q�[���p�b�h�C���X�g.
	bool keybordText = false;		//�L�[�{�[�h�C���X�g.
	bool particleFlagY = false;		//�U���p�[�e�B�N������t���O.
	bool particleFlagB = false;		//�U���p�[�e�B�N������t���O.
	bool walkParticleFlag = false;	//�����̃p�[�e�B�N������t���O.
	bool bombFlag = false;			//���e�̃t���O.

	Actor* target = nullptr;
	glm::vec3 position = glm::vec3(0);
	glm::vec3 velocity = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);

	struct MiniMapIcon
	{
		glm::vec3 position = glm::vec3(530, 280, 0);
		glm::vec3 velocity = glm::vec3(0);

		float width = 1280 / 5; ///< ��ʂ̕�(�s�N�Z����).
		float height = 720 / 5; ///< ��ʂ̍���(�s�N�Z����).
	};
	MiniMapIcon mapIcon;

	Camera camera;
};

#endif // MAINGAMESCENE_H_INCLUDED