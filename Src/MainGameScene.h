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
*メインゲーム画面.
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
	static bool StClearedE, StClearedW, StClearedS, StClearedN;	///<ステート移行可能状態.

private:
	void RenderMesh(const Collision::Frustum*, Mesh::DrawType);

	int jizoId = -1; ///< 現在戦闘中のお地蔵様のID.
	bool achivements[4] = { false, false, false, false }; ///< 敵討伐状態.
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

	//ゲーム状態.
	enum class State {
		play,		//通常プレイ中.
		menu,		//一時停止画面.
		result,		//リザルト画面.
		load,		//ロード画面.
		select,		//Yes or No画面.
		//メニュー画面.
		pMenu, cMenu, sMenu, gameClose, menuClose
	};

	State state = State::play;

	//タイマー系.
	float nextStageTimer = 0.0f;								//次のステージ、別のステージ移行中のタイマー.
	float loadTimer = 0.0f;										//シーン移行中のロード画面タイマー.
	float actionWaitTimer = 0.0f;								//攻撃のタイマー.
	float timer = 0.0f;											//シーンタイマー.
	float clearTimer = 0, overTimer = 0.0f;						//ゲームクリア、ゲームオーバのタイマー.
	float playerBulletTimerA = 0, playerBulletTimerB = 0,
		playerBulletTimerC = 0, playerBulletTimerD = 0.0f;		//弾のタイマー.
	float attackingTimer = 3.0f;								//敵の攻撃タイマー.
	float enemyPopTimerA = 0, enemyPopTimerB = 0,
		enemyPopTimerC = 0.0f, enemyPopTimerD = 0.0f;			//敵の出現時間間隔.
	float itemTimerA = 0, itemTimerB = 0, itemTimerC = 0.0f;	//アイテム出現タイマー.
	float eIntTimer = 0, wIntTimer = 0, 
		sIntTimer = 0, nIntTimer = 0.0f;						//各攻撃のインターバル.
	float allTimer = 0.0f;										//予備用タイマー.
	float particleTimerA = 0, particleTimerB = 0;				//攻撃パーティクル制御タイマー.
	float walkParticleTimer = 0;								//砂埃パーティクル制御タイマー.
	float bombTimer = 0;										//爆弾のタイマ―.

	//ID.
	int warpID = -1;
	int CoinID = -1;
	int enemyID = -1;

	//カウントやタイプ.
	int selectCount = 0;	//選択アイコン用.
	int selectCount2 = 0;	//選択アイコン用.
	int selectCount3 = 0;	////選択アイコン用.
	int stageChage = 0;		//どの試練か.
	float defenceLine = 100;	//０になるとゲームオーバ.

	//敵の情報.
	int enemySpawn = 100;	//一度に湧く敵の出現数.
	int enemyBlow = 0;		//敵を倒した数.
	int enemyStock = 1;		//敵の残数.
	int enemyAttack = 0;	//敵の攻撃力.
	float baseSpeed = 1.0f;	//敵の移動スピード.
	int randTarget = 0;		//ランダムでターゲット取得.


	float skillIntervalMax = 5.0f;

	glm::vec3 comammdSetPos[4] =					//好きな場所にアイコンをセットする為の配列.(上から時計回り)
	{ glm::vec3(500, -150, 0),glm::vec3(580, -230, 0),glm::vec3(500, -310, 0),glm::vec3(420, -230, 0) };
	int eCommandA = 0, wCommandA = 0, sCommandA = 0, nCommandA = 0;	//表側のスキル.
	int eCommandB = 0, wCommandB = 0, sCommandB = 0, nCommandB = 0;	//裏側のスキル.
	int skComAttack = 0;							//数字に応じて攻撃するスキルが異なる.
	int skComCount = 0;								//スキル表の時のカウント.

	struct ItemCount {
		int itemHP = 0;			//HPアップ.
		int itemMP = 0;			//MPアップ.
		int itemAll = 0;		//両方回復.
		int attackUP = 0;		//攻撃アップ.
		int blockUP = 0;		//防御アップ.
		int muteki = 0;			//無敵状態.
	};

	ItemCount itemC;

	//フラグ系.
	bool gameOverFlag = false;						///<ゲームオーバー状態.
	bool gameClearFlag = false;						///<ゲームクリア状態.
	bool nextStateFlag = false;						///<ステート移行可能状態.

	bool selectIconFlag = false;					///<セレクトアイコン.
	bool shotTimerFlagA = false, shotTimerFlagB = false, 
		shotTimerFlagC = false;						///<攻撃でタイマーを動かすフラグ.
	bool chargeShotFlagA, chargeShotFlagB = false;	///<溜め攻撃用フラグ.
	bool isAttacking = false;						///<敵の攻撃.
	bool bulletFragA, bulletFragB = false;			///<周囲に出る.
	bool eInterval = false, wInterval = false,
		sInterval = false, nInterval = false;		///<各ボタンのインターバル状態のフラグ.
	bool eCommand = false, wCommand = false,
		sCommand = false, nCommand = false;			///<どの攻撃ボタンを押しているか.

	bool iconUpFlag = false, iconDownFlag = false;	//アイコン動作状態.
	bool mHouseFlag = false;						//モンスターハウス.
	bool defenceFlag = false;						//防衛ラインが攻撃されているか.

	bool eventFrag = false;							//セリフイベント表示フラグ.

	bool cameraFar = false;			//カメラを離す.
	bool cameraNear = false;		//カメラを寄せる.
	bool gamePadText = false;		//ゲームパッドイラスト.
	bool keybordText = false;		//キーボードイラスト.
	bool particleFlagY = false;		//攻撃パーティクル制御フラグ.
	bool particleFlagB = false;		//攻撃パーティクル制御フラグ.
	bool walkParticleFlag = false;	//砂埃のパーティクル制御フラグ.
	bool bombFlag = false;			//爆弾のフラグ.

	Actor* target = nullptr;
	glm::vec3 position = glm::vec3(0);
	glm::vec3 velocity = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);

	struct MiniMapIcon
	{
		glm::vec3 position = glm::vec3(530, 280, 0);
		glm::vec3 velocity = glm::vec3(0);

		float width = 1280 / 5; ///< 画面の幅(ピクセル数).
		float height = 720 / 5; ///< 画面の高さ(ピクセル数).
	};
	MiniMapIcon mapIcon;

	Camera camera;
};

#endif // MAINGAMESCENE_H_INCLUDED