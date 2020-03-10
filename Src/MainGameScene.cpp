/**
*@file MainGameScene.cpp
*/
#include "GLFWEW.h"
#include "MainGameScene.h"
#include "StatusScene.h"
#include "EventScene.h"
#include "GameOverScene.h"
#include "ObjectiveActor.h"
#include "TitleScene.h"
#include "Mesh.h"
#include "SkeletalMeshActor.h"
#include "PlayerActor.h"
#include "EnemyActor.h"
#include "JizoActor.h"
#include "Scene.h"
#include "Warp.h"
#include "Goal.h"
#include <random>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <string>

int MainGameScene::StageNo = 1;
int PlayerActor::pLevel = 1;		//プレイヤーレベル.
int PlayerActor::pAbility = 1;		//アビリティレベル.
int PlayerActor::pExPoint = 100;	//経験値.
int PlayerActor::pExCount = 500;	//レベルアップまで.
int PlayerActor::pMP = 50;			//MP.
int PlayerActor::pHP = 200;			//HP.
int PlayerActor::maxHP;				//最大HP.
int PlayerActor::maxMP;				//最大MP.
int PlayerActor::playerID = 0;		//プレイヤーのメッシュ.

/**
*衝突を解決する.
*
*@param a	衝突したアクターその１.
*@param b	衝突したアクターその２.
*@param p	衝突位置.
*/
void PlayerCollisionHandler(const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
{
	const glm::vec3 v = a->colWorld.s.center - p;
	//衝突位置との距離が近すぎないか調べる.
		if (dot(v, v) > FLT_EPSILON) {
		// aをbに重ならない位置まで移動.
		const glm::vec3 vn = normalize(v);
		float radiusSum = a->colWorld.s.r;
		switch (b->colWorld.type) {
			case Collision::Shape::Type::sphere: radiusSum += b->colWorld.s.r; break;
			case Collision::Shape::Type::capsule: radiusSum += b->colWorld.c.r; break;
			
		}
		const float distance = radiusSum - glm::length(v) + 0.01f;
		a->position += vn * distance;
		a->colWorld.s.center += vn * distance;

		if (a->velocity.y < 0 && vn.y >= glm::cos(glm::radians(60.0f))) {
			a->velocity.y = 0;
		}
	}
		else {
			// 移動を取り消す(距離が近すぎる場合の例外処理).
			const float deltaTime = static_cast<float>(GLFWEW::Window::Instance().DeltaTime());
			const glm::vec3 deltaVelocity = a->velocity * deltaTime;
			a->position -= deltaVelocity;
			a->colWorld.s.center -= deltaVelocity;
		}
}

/**
*UIを操作する.
*
*@param count	表示されるUI.
*@param a		countがa以上なら上に.
*@param b		countがbと同じなら.
*@param c		countがcなら一番下に移動.
*@param d		countがd以下なら下に.
*@param e		countがeと同じなら.
*@param f		countがfなら一番上に移動.

*/void MainGameScene::selectUI(int a, int b,int c ,int d,int e,int f)
 {

	GLFWEW::Window& window = GLFWEW::Window::Instance();
	const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();

	if (window.GetGamePad().buttonDown & GamePad::DPAD_UP) {
		Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
		if (selectCount >= a) {
			--selectCount;
			if (selectCount == b) {
				selectCount = c;
			}
		}
	}
	//下にずれる.
	else if (window.GetGamePad().buttonDown & GamePad::DPAD_DOWN) {
		if (selectCount <= d) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			++selectCount;
			if (selectCount == e) {
				selectCount = f;
			}
		}
	}
}

/**
*UIを操作する.
*
*@param count	表示されるUI.
*@param a		countがa以上なら上に.
*@param b		countがbと同じなら.
*@param c		countがcなら一番下に移動.
*@param d		countがd以下なら下に.
*@param e		countがeと同じなら.
*@param f		countがfなら一番上に移動.

*/void MainGameScene::skSelectUI(int a, int b,int c ,int d,int e,int f)
 {

	GLFWEW::Window& window = GLFWEW::Window::Instance();
	const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();

	if (window.GetGamePad().buttonDown & GamePad::DPAD_UP) {
		Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
		if (skComCount>= a) {
			--skComCount;
			if (skComCount == b) {
				skComCount = c;
			}
		}
	}
	//下にずれる.
	else if (window.GetGamePad().buttonDown & GamePad::DPAD_DOWN) {
		if (skComCount <= d) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			++skComCount;
			if (skComCount == e) {
				skComCount = f;
			}
		}
	}
}

/*
*.
*/
void MainGameScene::EnemyTargetID(ActorList& a) {
	int id = std::uniform_int_distribution<>(0, 2)(rand);
	////ターゲットの方向を調べる.
	for (auto i = a.begin(); i != a.end(); ++i) {
		const int index = i - a.begin() - id;
		if (index == 0) {
			enemyID = 0;
		}
		if (index == 1) {
			enemyID = 1;
		}
		if (index == 2) {
			enemyID = 2;
		}
	}
}

/*
*.
*/
void MainGameScene::EnemySpawn() {
	const int a = std::uniform_int_distribution<>(0,2)(rand);
	const int b = std::uniform_int_distribution<>(0, 2)(rand);
	const int c = std::uniform_int_distribution<>(0, 2)(rand);
	const int d = std::uniform_int_distribution<>(0, 2)(rand);
	if (state == State::play) {
		const size_t oniCountA = 3;
		const size_t oniCountB = 3;
		const size_t oniCountC = 3;
		const size_t oniCountD = 2;
		glm::vec3 position = glm::vec3(0);

		if (enemyPopTimerA >= 5.0f) {
			if (enemySpawn >= 0) {
				for (size_t i = 0; i < oniCountA; i++)
				{
					position.x += std::uniform_real_distribution<float>(50, 100)(rand);
					position.z += std::uniform_real_distribution<float>(50, 100)(rand);
					glm::vec3 rotation(0);
					rotation.y = std::uniform_real_distribution<float>(0, 3.14f * 2.0f)(rand);
					position.y = heightMap.Height(position);

					const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
					SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
						mesh, "Kooni", 13, position, rotation);
					p->colLocal = Collision::CreateCapsule(
						glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
					enemies[0].Add(p);
					randTarget = a;
					EnemyTargetID(enemies[0]);
				}
				enemyPopTimerA = 0.0f;
				enemySpawn -= 3;
			}
		}
		if (enemyPopTimerB >= 6.0f) {
			if (enemySpawn >= 0) {
				for (size_t i = 0; i < oniCountB; i++)
				{
					position.x += std::uniform_real_distribution<float>(50, 100)(rand);
					position.z += std::uniform_real_distribution<float>(50, 100)(rand);
					glm::vec3 rotation(0);
					rotation.y = std::uniform_real_distribution<float>(0, 3.14f * 2.0f)(rand);
					position.y = heightMap.Height(position);
					const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
					SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
						mesh, "Kooni", 13, position, rotation);
					p->colLocal = Collision::CreateCapsule(
						glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 1.0f);
					p->scale = glm::vec3(2);
					enemies[1].Add(p);
					randTarget = b;
					EnemyTargetID(enemies[1]);
				}
				enemyPopTimerB = 0.0f;
				enemySpawn -= 3;
			}
		}
		if (enemyPopTimerC >= 10.0f) {
			if (enemySpawn >= 0) {
				for (size_t i = 0; i < oniCountC; i++)
				{
					position.x += std::uniform_real_distribution<float>(50, 100)(rand);
					position.z += std::uniform_real_distribution<float>(50, 100)(rand);
					glm::vec3 rotation(0);
					rotation.y = std::uniform_real_distribution<float>(0, 3.14f * 2.0f)(rand);
					position.y = heightMap.Height(position);
					const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
					SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
						mesh, "Kooni", 13, position, rotation);
					p->colLocal = Collision::CreateCapsule(
						glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 1.5f);
					p->scale = glm::vec3(3);
					enemies[2].Add(p);
					randTarget = c;
					EnemyTargetID(enemies[2]);
				}
				enemyPopTimerC = 0.0f;
				enemySpawn -= 3;
			}
		}
		if (enemyPopTimerD >= 5.0f) {
				for (size_t i = 0; i < oniCountA; i++)
				{
					position.x += std::uniform_real_distribution<float>(50, 100)(rand);
					position.z += std::uniform_real_distribution<float>(50, 100)(rand);
					glm::vec3 rotation(0);
					rotation.y = std::uniform_real_distribution<float>(0, 3.14f * 2.0f)(rand);
					position.y = heightMap.Height(position);

					const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
					SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
						mesh, "Kooni", 13, position, rotation);
					p->colLocal = Collision::CreateCapsule(
						glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
					enemies[3].Add(p);
					randTarget = d;
					EnemyTargetID(enemies[3]);
				}
				enemyPopTimerD = 0.0f;
		}
	}
}

void MainGameScene::EnemyAI(float deltaTime ,ActorList& x,int a ,int b) {
	//敵が追いかけてくる.
	for (auto& e : enemies[a]) {
		SkeletalMeshActorPtr enemy = std::static_pointer_cast<SkeletalMeshActor>(e);
		Mesh::SkeletalMeshPtr mesh = enemy->GetMesh();

		//死亡したら消す.
		if (mesh->GetAnimation() == "Down") {
			if (mesh->IsFinished()) {
				enemy->health = 0;
				enemyBlow += 1;
				player->pExPoint -= 10;
				player->pExCount -= 50;

				enemySpawn += 1;
			}
			continue;
		}

		const float moveSpeed = baseSpeed * 3.0f - a + 1;
		const float rotationSpeed = baseSpeed * glm::radians(60.0f);
		const float frontRange = glm::radians(15.0f);

		const glm::vec3 v = (*(x.begin() + b))->position - e->position;
		const glm::vec3 vTarget = glm::normalize(v);
		float radian = std::atan2(-vTarget.z, vTarget.x) - glm::radians(90.0f);

		if (radian <= 0) {
			radian += glm::radians(360.0f);
		}
		const glm::vec3 vEnemyFront = glm::rotate(
			glm::mat4(1), e->rotation.y + glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);

		//ターゲットが正面にいなかったら、正面にとらえるような左右に旋回.
		if (std::abs(radian - e->rotation.y) > frontRange) {
			const glm::vec3 vRotDir = glm::cross(vEnemyFront, vTarget);
			if (vRotDir.y >= 0) {
				e->rotation.y += rotationSpeed * deltaTime;
				if (e->rotation.y >= glm::radians(360.0f)) {
					e->rotation.y -= glm::radians(360.0f);
				}
			}
			else {
				e->rotation.y -= rotationSpeed * deltaTime;
				if (e->rotation.y < 0) {
					e->rotation.y += glm::radians(360.0f);
				}
			}
		}
		if (e->health <= 0) {
			e->velocity = glm::vec3(0);	//死んでいるのでもう移動しない.
		}
		//十分に接近していなければ移動する。接近していれば攻撃する.
		if (glm::length(v) > 2.5f) {
			e->velocity = vEnemyFront * moveSpeed;
			if (mesh->GetAnimation() != "Run") {
				mesh->Play("Run");
			}
		}
		else {
			e->velocity = glm::vec3(0);	//接近しているのでもう移動しない.
			if (mesh->GetAnimation() != "Wait") {
				if ((mesh->GetAnimation() != "Attack" && mesh->GetAnimation() != "Hit") ||
					mesh->IsFinished()) {
					mesh->Play("Wait");
				}
			}						//定期的に攻撃状態になる.
			if (isAttacking) {
				isAttacking = false;
				attackingTimer = 3.0f;	//次の攻撃は５秒後.
				mesh->Play("Attack", false);
			}
			else {
				attackingTimer -= deltaTime;
				if (attackingTimer <= 0) {
					isAttacking = true;
					defenceFrag = true;
						defenceLine -= 1 + a;
				}
			}
		}
	}
}

void MainGameScene::EnemyDetectCollision(int i) {

	// プレイヤーの攻撃判定.
	ActorPtr attackCollision = player->GetAttackCollision();
	if (attackCollision) {
		bool hit = false;
		DetectCollision(attackCollision, enemies[i],
			[this, &hit](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			SkeletalMeshActorPtr bb = std::static_pointer_cast<SkeletalMeshActor>(b);
			bb->health -= a->health;
			float scaleFactor = 1;
			if (bb->health <= 0) {
				bb->colLocal = Collision::Shape{};
				bb->health = 1;
				bb->GetMesh()->Play("Down", false);
				scaleFactor = 1.5f;
			}
			else {
				bb->GetMesh()->Play("Hit", false);
			}
			std::cerr << "[INFO] enemy hp=" << bb->health << "\n";

			auto mesh = meshBuffer.GetSkeletalMesh("Effect.Hit");
			mesh->Play(mesh->GetAnimationList()[0].name, false);
			mesh->SetAnimationSpeed(std::uniform_real_distribution<float>(0.75f, 1.0f)(rand) / scaleFactor);
			glm::vec3 rot = player->rotation;
			rot.x += std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
			rot.y += std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
			rot.z += std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
			glm::vec3 scale(std::uniform_real_distribution<float>(1.0f, 1.5f)(rand) * scaleFactor);
			auto effect = std::make_shared<SkeletalMeshActor>(mesh, "Effect.Hit", 1, p, rot, scale * b->scale);
			effects.Add(effect);
			hit = true;
		}
		);
		if (hit) {
			attackCollision->health = 0;
			Audio::Engine::Instance().Prepare("Res/Audio/Enemy.mp3")->Play();
		}
	}
}

/**
*シーンを初期化する.
*
*@retval true	初期化成功.
*@retval false	初期化失敗.ゲーム進行不可につき、プログラムを終了すること
*/
bool MainGameScene::Initialize() {

	spriteRenderer.Init(1000, "Res/Sprite.vert", "Res/Sprite.frag");
	sprites.reserve(100);
	meshBuffer.Init(1'000'000 * sizeof(Mesh::Vertex), 3'000'000 * sizeof(GLushort));
	lightBuffer.Init(1);
	lightBuffer.BindToShader(meshBuffer.GetStaticMeshShader());
	lightBuffer.BindToShader(meshBuffer.GetTerrainShader());
	/*lightBuffer.BindToShader(meshBuffer.GetWaterShader());*/
	meshBuffer.LoadMesh("Res/red_pine_tree.gltf");
	meshBuffer.LoadMesh("Res/jizo_statue.gltf");
	meshBuffer.LoadSkeletalMesh("Res/bikuni.gltf");
	meshBuffer.LoadSkeletalMesh("Res/oni_small.gltf");
	meshBuffer.LoadSkeletalMesh("Res/skeleton.gltf");
	meshBuffer.LoadMesh("Res/Blackshadow.gltf");
	meshBuffer.LoadMesh("Res/Skeltal.gltf");
	meshBuffer.LoadMesh("Res/Gate.gltf");
	meshBuffer.LoadMesh("Res/Triangle.gltf");
	meshBuffer.LoadMesh("Res/RangeStone.gltf");
	meshBuffer.LoadMesh("Res/GateBlock.gltf");
	meshBuffer.LoadMesh("Res/HP.gltf");
	meshBuffer.LoadMesh("Res/MP.gltf");
	meshBuffer.LoadMesh("Res/HPMP.gltf");
	meshBuffer.LoadSkeletalMesh("Res/effect_hit_normal.gltf");
	meshBuffer.LoadSkeletalMesh("Res/effect_curse.gltf");

	//ロード画面.
	Sprite road0(Texture::Image2D::Create("Res/Black.tga"));
	road0.Scale(glm::vec2(0));
	sprites.push_back(road0);

	//メイン情報(右).
	Sprite Menu1(Texture::Image2D::Create("Res/MenuWindow5.tga"));
	Menu1.Position(glm::vec3(160, 0, 0));
	Menu1.Scale(glm::vec2(0));
	sprites.push_back(Menu1);

	//メニュー一覧(左上).
	Sprite Menu2(Texture::Image2D::Create("Res/MenuWindow5.tga"));
	Menu2.Position(glm::vec3(-380, 85, 0));
	Menu2.Scale(glm::vec2(0));
	sprites.push_back(Menu2);

	//その他(左下).
	Sprite Menu3(Texture::Image2D::Create("Res/MenuWindow5.tga"));
	Menu3.Position(glm::vec3(-380, -220, 0));
	Menu3.Scale(glm::vec2(0));
	sprites.push_back(Menu3);

	//ロード画面中の画像.
	Sprite LoadA4(Texture::Image2D::Create("Res/LoadA.tga"));
	LoadA4.Position(glm::vec3(0,130,0));
	LoadA4.Scale(glm::vec2(0));
	sprites.push_back(LoadA4);

	Sprite LoadB5(Texture::Image2D::Create("Res/LoadB.tga"));
	LoadB5.Position(glm::vec3(0,130,0));
	LoadB5.Scale(glm::vec2(0));
	sprites.push_back(LoadB5);

	Sprite LoadC6(Texture::Image2D::Create("Res/LoadC.tga"));
	LoadC6.Position(glm::vec3(0,130,0));
	LoadC6.Scale(glm::vec2(0));
	sprites.push_back(LoadC6);

	Sprite LoadD7(Texture::Image2D::Create("Res/LoadD.tga"));
	LoadD7.Position(glm::vec3(0,100,0));
	LoadD7.Scale(glm::vec2(0));
	sprites.push_back(LoadD7);

	//リザルト画面のロゴ.
	Sprite NextLogo8(Texture::Image2D::Create("Res/Next.tga"));
	NextLogo8.Position(glm::vec3(100, 120, 0));
	NextLogo8.Scale(glm::vec2(0));
	sprites.push_back(NextLogo8);

	Sprite ToTitle9(Texture::Image2D::Create("Res/ToTitle.tga"));
	ToTitle9.Position(glm::vec3(100, -120, 0));
	ToTitle9.Scale(glm::vec2(0));
	sprites.push_back(ToTitle9);

	//ゲームオーバー時.
	Sprite GameOver10(Texture::Image2D::Create("Res/Over.tga"));
	GameOver10.Scale(glm::vec2(0));
	GameOver10.Position(glm::vec3(0, 100, 0));
	sprites.push_back(GameOver10);

	//ゲームクリア時.
	Sprite GameClear11(Texture::Image2D::Create("Res/Clear.tga"));
	GameClear11.Scale(glm::vec2(0));
	GameClear11.Position(glm::vec3(0, 100, 0));
	sprites.push_back(GameClear11);
	
	//メニュー画面のプレイヤーアイコン.
	Sprite pSoul12(Texture::Image2D::Create("Res/pSoul.tga"));
	pSoul12.Position(glm::vec3(-50, 110, 0));
	pSoul12.Scale(glm::vec2(0));
	sprites.push_back(pSoul12);

	Sprite pGoblin13(Texture::Image2D::Create("Res/pGoblin.tga"));
	pGoblin13.Position(glm::vec3(-50, 110, 0));
	pGoblin13.Scale(glm::vec2(0));
	sprites.push_back(pGoblin13);

	Sprite pWizard14(Texture::Image2D::Create("Res/pWizard.tga"));
	pWizard14.Position(glm::vec3(-50, 110, 0));
	pWizard14.Scale(glm::vec2(0));
	sprites.push_back(pWizard14);

	Sprite pGaikotsu15(Texture::Image2D::Create("Res/pGaikotsu.tga"));
	pGaikotsu15.Position(glm::vec3(-50, 110, 0));
	pGaikotsu15.Scale(glm::vec2(0));
	sprites.push_back(pGaikotsu15);

	//HP.
	Sprite barHP16(Texture::Image2D::Create("Res/Green.tga"));
	barHP16.Scale(glm::vec2(0));
	sprites.push_back(barHP16);

	//MP.
	Sprite barMP17(Texture::Image2D::Create("Res/Blue.tga"));
	barMP17.Scale(glm::vec2(0));
	sprites.push_back(barMP17);

	//ウィザードと骸骨の攻撃コマンド.
		Sprite AttackNormal18(Texture::Image2D::Create("Res/AttackTest.dds"));
		AttackNormal18.Position(glm::vec3(500, -310, 0));	//下アイコン初期値.
		AttackNormal18.Scale(glm::vec2(0));
		sprites.push_back(AttackNormal18);

		Sprite AttackStrong19(Texture::Image2D::Create("Res/Attack2.tga"));
		AttackStrong19.Position(glm::vec3(420, -230, 0));	//左アイコン初期値.
		AttackStrong19.Scale(glm::vec2(0));
		sprites.push_back(AttackStrong19);

	//ウィザードの攻撃コマンド.
		Sprite Magic20(Texture::Image2D::Create("Res/Slowing.tga"));
		Magic20.Position(glm::vec3(500, -150, 0));	//上アイコン初期値.
		Magic20.Scale(glm::vec2(0));
		sprites.push_back(Magic20);

		Sprite Meteo21(Texture::Image2D::Create("Res/Meteo.tga"));
		Meteo21.Position(glm::vec3(580, -230, 0));	//右アイコン初期値.
		Meteo21.Scale(glm::vec2(0));//Meteo33.Scale(glm::vec2(0.12f));
		sprites.push_back(Meteo21);
		
	//選択アイコン.
	Sprite IconBlue22(Texture::Image2D::Create("Res/select.tga"));
	IconBlue22.Position(glm::vec3(-380, 210, 0));
	//IconBlue22.Scale(glm::vec2(0.8f , 0.15f));
	IconBlue22.Scale(glm::vec2(0));	//IconBlue36.Scale(glm::vec2(0.28f,0.22f));
	sprites.push_back(IconBlue22);

	//選択アイコン.
	Sprite selectIcon23(Texture::Image2D::Create("Res/selectA.tga"));
	selectIcon23.Position(glm::vec3(-490, 210, 0));
	selectIcon23.Scale(glm::vec2(0));
	sprites.push_back(selectIcon23);

	//選択アイコン.
	Sprite selectIcon24(Texture::Image2D::Create("Res/selectA.tga"));
	selectIcon24.Position(glm::vec3(-490, 210, 0));
	selectIcon24.Scale(glm::vec2(0));
	sprites.push_back(selectIcon24);

		//防衛ラインのHP(攻撃を受けている時).
		Sprite defenceHP25(Texture::Image2D::Create("Res/Red.tga"));
		defenceHP25.Scale(glm::vec2(0));
		sprites.push_back(defenceHP25);

		//防衛ラインのアイコン.
		Sprite defenceIcon26(Texture::Image2D::Create("Res/Defence.tga"));
		defenceIcon26.Position(glm::vec3(-180, 350, 0));
		defenceIcon26.Scale(glm::vec2(0));
		sprites.push_back(defenceIcon26);

		//敵の残数アイコン.
		Sprite enemyIcon27(Texture::Image2D::Create("Res/Enemy.tga"));
		enemyIcon27.Position(glm::vec3(330, 350, 0));
		enemyIcon27.Scale(glm::vec2(0));
		sprites.push_back(enemyIcon27);

		//防衛ラインのHP（攻撃を受けてない時）.
		Sprite defenceHP28(Texture::Image2D::Create("Res/SkyBlue.tga"));
		defenceHP28.Scale(glm::vec2(0));
		sprites.push_back(defenceHP28);

	//骸骨の攻撃アイコン.
		Sprite AttackBladeA29(Texture::Image2D::Create("Res/AttackBlade1.tga"));
		AttackBladeA29.Position(glm::vec3(500, -150, 0));	//上アイコン初期値.
		AttackBladeA29.Scale(glm::vec2(0));
		sprites.push_back(AttackBladeA29);

		Sprite AttackBladeB30(Texture::Image2D::Create("Res/AttackBlade2.tga"));
		AttackBladeB30.Position(glm::vec3(580, -230, 0));	//右アイコン初期値.
		AttackBladeB30.Scale(glm::vec2(0));
		sprites.push_back(AttackBladeB30);

	//ゴブリンのアイコン.
		Sprite GobAttack31(Texture::Image2D::Create("Res/GobAttack.tga"));
		GobAttack31.Position(glm::vec3(500, -310, 0));	//下アイコン初期値.
		GobAttack31.Scale(glm::vec2(0));
		sprites.push_back(GobAttack31);

		Sprite GobDash32(Texture::Image2D::Create("Res/StatusUp.tga"));
		GobDash32.Position(glm::vec3(420, -230, 0));	//左アイコン初期値.
		GobDash32.Scale(glm::vec2(0));
		sprites.push_back(GobDash32);

	//何も使えないアイコン.
		Sprite NoAttack33(Texture::Image2D::Create("Res/NoAttack.tga"));
		NoAttack33.Position(glm::vec3(500, -310, 0));	//下アイコン初期値.
		NoAttack33.Scale(glm::vec2(0));//NoAttack50.Scale(glm::vec2(0.2f));
		sprites.push_back(NoAttack33);

		//各コマンドのクールタイム.
		Sprite IconGreen34(Texture::Image2D::Create("Res/IconGreen.tga"));
		IconGreen34.Position(glm::vec3(0, 0, 0));
		IconGreen34.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
		sprites.push_back(IconGreen34);

		Sprite IconGreen35(Texture::Image2D::Create("Res/IconGreen.tga"));
		IconGreen35.Position(glm::vec3(0, 0, 0));
		IconGreen35.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
		sprites.push_back(IconGreen35);

		Sprite IconGreen36(Texture::Image2D::Create("Res/IconGreen.tga"));
		IconGreen36.Position(glm::vec3(0, 0, 0));
		IconGreen36.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
		sprites.push_back(IconGreen36);

		Sprite IconGreen37(Texture::Image2D::Create("Res/IconGreen.tga"));
		IconGreen37.Position(glm::vec3(0, 0, 0));
		IconGreen37.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
		sprites.push_back(IconGreen37);

	if (StageNo == 1) {
		bgm = Audio::Engine::Instance().Prepare("Res/Audio/game_maoudamashii_4_field03.mp3");
		bgm->Play(Audio::Flag_Loop);
	}
	else if (StageNo == 2) {
		bgm = Audio::Engine::Instance().Prepare("Res/Audio/game_maoudamashii_7_rock01.mp3");
		bgm->Play(Audio::Flag_Loop);
	}
	else if (StageNo == 3) {
		bgm = Audio::Engine::Instance().Prepare("Res/Audio/game_maoudamashii_7_rock11.mp3");
		bgm->Play(Audio::Flag_Loop);
	}
	else if (StageNo == 4) {
		bgm = Audio::Engine::Instance().Prepare("Res/Audio/game_maoudamashii_7_rock20.mp3");
		bgm->Play(Audio::Flag_Loop);
	}
	else if (StageNo == 5) {
		bgm = Audio::Engine::Instance().Prepare("Res/Audio/game_maoudamashii_7_rock30.mp3");
		bgm->Play(Audio::Flag_Loop);
	}

	progLighting.Reset(Shader::BuildFromFile("Res/FragmentLighting.vert", "Res/FragmentLighting.frag"));

	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/font.fnt");

	//パーティクル・システムを初期化する.
	particleSystem.Init(1000);

	// FBOを作成する.
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	fboMain = FramebufferObject::Create(window.Width(), window.Height(), GL_RGBA16F);
	Mesh::FilePtr rt = meshBuffer.AddPlane("RenderTarget");
	if (rt) {
		rt->materials[0].program = Shader::Program::Create(
			"Res/DepthOfField.vert", "Res/DepthOfField.frag");
		rt->materials[0].texture[0] = fboMain->GetColorTexture();
		rt->materials[0].texture[1] = fboMain->GetDepthTexture();

	}
	if (!rt || !rt->materials[0].program) {
		return false;
	}

	//DoF描画用のFBOを作る.
	fboDepthOfField = FramebufferObject::Create(window.Width(), window.Height(), GL_RGBA16F);

	//元解像度の縦横1/2(面積が1/4)の大きさのブルーム用FBOを作る.
	int w = window.Width();
	int h = window.Height();
	for (int j = 0; j < sizeof(fboBloom) / sizeof(fboBloom[0]); ++j) {
		w /= 2;
		h /= 2;
		for (int i = 0; i < sizeof(fboBloom[0]) / sizeof(fboBloom[0][0]); ++i) {
			fboBloom[j][i] = FramebufferObject::Create(w, h, GL_RGBA16F, FrameBufferType::colorOnly);
			if (!fboBloom[j][i]) {
				return false;
			}
		}
	}

	//ブルーム・エフェクト用の平面ポリゴンメッシュを作成する.
	if (Mesh::FilePtr mesh = meshBuffer.AddPlane("BrightPassFilter")) {
		Shader::ProgramPtr p = Shader::Program::Create("Res/Simple.vert", "Res/BrightPassFilter.frag");
		p->Use();
		p->SetViewProjectionMatrix(glm::mat4(1));
		mesh->materials[0].program = p;
	}
	if (Mesh::FilePtr mesh = meshBuffer.AddPlane("NormalBlur")) {
		Shader::ProgramPtr p = Shader::Program::Create("Res/Simple.vert", "Res/NormalBlur.frag");
		p->Use();
		p->SetViewProjectionMatrix(glm::mat4(1));
		mesh->materials[0].program = p;
	}
	if (Mesh::FilePtr mesh = meshBuffer.AddPlane("Simple")) {
		Shader::ProgramPtr p = Shader::Program::Create("Res/Simple.vert", "Res/Simple.frag");
		p->Use();
		p->SetViewProjectionMatrix(glm::mat4(1));
		mesh->materials[0].program = p;
	}
	if (glGetError()) {
		std::cout << "[エラー]" << __func__ << ": ブルーム用メッシュの作成に失敗.\n";
		return false;
	}

	//デプスシャドウマッピング用のFBOを作成する.
	{
		fboShadow = FramebufferObject::Create(
			4096, 4096, GL_NONE, FrameBufferType::depthOnly);
		if (glGetError()) {
			std::cout << "[エラー]" << __func__ << ":シャドウ用FBOの作成に失敗.\n";
			return false;
			
		}
		//sampler2DShadowの比較モードを設定する.
		glBindTexture(GL_TEXTURE_2D, fboShadow->GetDepthTexture()->Get());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	std::random_device rd;
	rand.seed(rd());

	//ハイトマップを作成する.
	if (StageNo % 2 == 1) {
		if (!heightMap.LoadFromFile("Res/WhiteFront.tga", 20.0f, 0.5f)) {

			return false;
		}
	}
	else if (StageNo % 2 == 0) {
		if (!heightMap.LoadFromFile("Res/WhiteFront.tga", 20.0f, 0.5f)) {

			return false;
		}
	}

	if (!heightMap.CreateMesh(meshBuffer, "Terrain")) {

		return false;
	}
	//if (!heightMap.CreateWaterMesh(meshBuffer, "Water", 0.0)) { // 水面の高さは要調整.

	//	return false;
	//}

	if (StageNo == 2) {
		enemyStock = 20;
		enemySpawn = 5;
	}
	else if (StageNo == 3) {
		enemyStock = 30;
		enemySpawn = 10;
	}
	else if (StageNo == 4) {
		enemyStock = 40;
		enemySpawn = 20;
	}
	else if (StageNo == 5) {
		enemyStock = 100;
		enemySpawn = 100;
	}

	glm::vec3 startPos(79, 0, 100);
	startPos.y = heightMap.Height(startPos);
	player = std::make_shared<PlayerActor>(&heightMap, meshBuffer, startPos);
	if (player->playerID == 0) {
		player->SetStaticMesh(meshBuffer.GetFile("Res/Blackshadow.gltf"), 0);
	}
	else if (player->playerID == 1) {
		player->SetMesh(meshBuffer.GetSkeletalMesh("oni_small"), 0);
		player->GetMesh()->Play("Idle.LookAround");
	}
	else if (player->playerID == 2) {
		player->SetMesh(meshBuffer.GetSkeletalMesh("Bikuni"), 0);
		player->GetMesh()->Play("Idle");
	}
	else if (player->playerID == 3) {
		player->SetMesh(meshBuffer.GetSkeletalMesh("Skeleton"), 0);
		player->GetMesh()->Play("Idle");
	}
	player->maxHP = player->pHP;
	player->maxMP = player->pMP;

	// ライトを配置
	lights.Add(std::make_shared<DirectionalLightActor>
		("DLight", glm::vec3(1.0f, 0.94f, 0.91f), glm::normalize(glm::vec3(1, -1, -1))));
	//if (StageNo % 2 == 1) {
	//	{
	//		glm::vec3 color = glm::vec3(1, 0.8f, 0.5f) * 7.0f;
	//		/*glm::vec3 position(0);
	//		position.x = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
	//		position.z = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
	//		position.y = heightMap.Height(position) + 5;*/
	//		/*lights.Add(std::make_shared<PointLightActor>("PointLight", color, glm::vec3(91, 13, 100)));
	//		lights.Add(std::make_shared<PointLightActor>("PointLight", color, glm::vec3(67, 13, 100)));
	//		lights.Add(std::make_shared<PointLightActor>("PointLight", color, glm::vec3(79, 13, 112)));
	//		lights.Add(std::make_shared<PointLightActor>("PointLight", color, glm::vec3(79, 13, 88)));*/
	//	}

	//	{
	//		glm::vec3 color = glm::vec3(1, 2, 3) * 15.0f;
	//		glm::vec3 direction(glm::normalize(glm::vec3(0.25f, -1, 0.25f)));
	//		/*glm::vec3 position(0);
	//		position.x = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
	//		position.z = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
	//		position.y = heightMap.Height(position) + 5;*/
	//		/*lights.Add(std::make_shared<SpotLightActor>("SpotLight", color,
	//			glm::vec3(87.5f, heightMap.Height(position) + 5, 99), direction, glm::radians(20.0f), glm::radians(15.0f)));
	//		lights.Add(std::make_shared<SpotLightActor>("SpotLight", color,
	//			glm::vec3(67.5f, heightMap.Height(position) + 5, 99), direction, glm::radians(20.0f), glm::radians(15.0f)));
	//		lights.Add(std::make_shared<SpotLightActor>("SpotLight", color,
	//			glm::vec3(77.5f, heightMap.Height(position) + 5, 89), direction, glm::radians(20.0f), glm::radians(15.0f)));
	//		lights.Add(std::make_shared<SpotLightActor>("SpotLight", color,
	//			glm::vec3(77.5f, heightMap.Height(position) + 5, 109), direction, glm::radians(20.0f), glm::radians(15.0f)));*/
	//	}
	//}

	lights.Update(0);
	lightBuffer.Update(lights, glm::vec3(0.1f, 0.05f, 0.15f));
	heightMap.UpdateLightIndex(lights);

	//ディフェンスポイントを配置
	if (StageNo != 1) {
		const Mesh::FilePtr meshDefencePoint = meshBuffer.GetFile("Res/jizo_statue.gltf");
		for (int i = 0; i < 3; ++i) {
			glm::vec3 position(0);
			position.x = static_cast<float>(std::uniform_int_distribution<>(40,120)(rand));
			position.z = static_cast<float>(std::uniform_int_distribution<>(60,140)(rand));
			position.y = heightMap.Height(position);
			glm::vec3 rotation(0);
			rotation.y = std::uniform_real_distribution<float>(0.0f, 3.14f * 2.0f)(rand);
			if (position.y >= 4.0f) {
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshDefencePoint, "DefencePoint", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateCapsule(
					glm::vec3(0, 1.5f, 0), glm::vec3(0, 1.5f, 0), 1.5f);
				p->scale = glm::vec3(6); // 見つけやすいように拡大.
				defencePoint.Add(p);
			}
			else {
				--i;
			}
		}
	}

	//初期ステージ石壁を配置.
	{
		if (StageNo == 1) {
			//下側の壁.
			const size_t wallCount = 9;
			walls.Reserve(wallCount);

			const Mesh::FilePtr meshStoneWall = meshBuffer.GetFile("Res/Skeltal.gltf");

			//下側の壁.
			for (size_t i = 0; i < wallCount; ++i) {

				const int posX = 25 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(posX, 2, 25);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, 1), glm::vec3(10, 20, 1));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
			//上側の壁.
			for (size_t i = 0; i < wallCount; ++i) {

				const int posX = 25 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(posX, 2, -23);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, 11), glm::vec3(10, 20, 1));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
			//右側の壁.
			for (size_t i = 0; i < wallCount; ++i) {

				const int posZ = 25 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(25, 2, posZ);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(-1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, -1), glm::vec3(1, 20, 10));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
			//左側の壁.
			for (size_t i = 0; i < wallCount; ++i) {

				const int posZ = 25 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(-23, 2, posZ);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, -1), glm::vec3(1, 20, 10));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
		}
	}
	//それ以外のステージの石壁を配置.
	{
		if (StageNo != 1) {
			//下側の壁.
			const size_t wallCount = 20;
			walls.Reserve(wallCount);

			const Mesh::FilePtr meshStoneWall = meshBuffer.GetFile("Res/Skeltal.gltf");

			//下側の壁.
			for (size_t i = 0; i < wallCount; ++i) {

				const int posX = 65 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(posX, 2, 50);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, 1), glm::vec3(10, 20, 1));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
			//上側の壁.
			for (size_t i = 0; i < wallCount; ++i) {

				const int posX = 65 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(posX, 2, -50);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, 11), glm::vec3(10, 20, 1));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
			//右側の壁.
			for (size_t i = 0; i < wallCount; ++i) {

				const int posZ = 65 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(50, 2, posZ);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(-1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, -1), glm::vec3(1, 20, 10));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
			//左側の壁.
			for (size_t i = 0; i < wallCount; ++i) {

				const int posZ = 65 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(-50, 2, posZ);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, -1), glm::vec3(1, 20, 10));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
		}
	}

	//ワープゲートの配置.
	{
	if (StageNo == 1) {
			//上から東西南北の順.
			const Mesh::FilePtr meshWarpGate = meshBuffer.GetFile("Res/Gate.gltf");
			const Mesh::FilePtr meshGateBlock = meshBuffer.GetFile("Res/GateBlock.gltf");

			glm::vec3 positionE = player->position + glm::vec3(12, 0, 0);
			glm::vec3 positionW = player->position + glm::vec3(-12, 0, 0);
			glm::vec3 positionS = player->position + glm::vec3(0, 0, 12);
			glm::vec3 positionN = player->position + glm::vec3(0, 0, -12);

			positionE.y = heightMap.Height(position);
			positionW.y = heightMap.Height(position);
			positionS.y = heightMap.Height(position);
			positionN.y = heightMap.Height(position);

			StaticMeshActorPtr GateE = std::make_shared<StaticMeshActor>(
				meshWarpGate, "GateE", 100, positionE, glm::vec3(0, 0, 0));
			StaticMeshActorPtr GateBlockE = std::make_shared<StaticMeshActor>(
				meshGateBlock, "GateBlockE", 100, positionE, glm::vec3(0, 0, 0));
			GateE->scale = glm::vec3(1, 3.5f, 1);
			GateE->colLocal = Collision::CreateCapsule(
				glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.4f);
			warp[0].Add(GateE);
			objects.Add(GateBlockE);

			StaticMeshActorPtr GateW = std::make_shared<StaticMeshActor>(
				meshWarpGate, "GateW", 100, positionW, glm::vec3(0, 0, 0));
			StaticMeshActorPtr GateBlockW = std::make_shared<StaticMeshActor>(
				meshGateBlock, "GateBlockW", 100, positionW, glm::vec3(0, 0, 0));
			GateW->scale = glm::vec3(1, 3.5f, 1);
			GateW->colLocal = Collision::CreateCapsule(
				glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.4f);
			warp[1].Add(GateW);
			objects.Add(GateBlockW);

			StaticMeshActorPtr GateS = std::make_shared<StaticMeshActor>(
				meshWarpGate, "GateS", 100, positionS, glm::vec3(0, 0, 0));
			StaticMeshActorPtr GateBlockS = std::make_shared<StaticMeshActor>(
				meshGateBlock, "GateBlockS", 100, positionS, glm::vec3(0, glm::radians(90.0f), 0));
			GateS->scale = glm::vec3(1, 3.5f, 1);
			GateS->colLocal = Collision::CreateCapsule(
				glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.4f);
			warp[2].Add(GateS);
			objects.Add(GateBlockS);

			StaticMeshActorPtr GateN = std::make_shared<StaticMeshActor>(
				meshWarpGate, "GateN", 100, positionN, glm::vec3(0, 0, 0));
			StaticMeshActorPtr GateBlockN = std::make_shared<StaticMeshActor>(
				meshGateBlock, "GateBlockN", 100, positionN, glm::vec3(0, glm::radians(90.0f), 0));
			GateN->scale = glm::vec3(1, 3.5f, 1);
			GateN->colLocal = Collision::CreateCapsule(
				glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.4f);
			warp[3].Add(GateN);
			objects.Add(GateBlockN);
		}
	}

	//乗り移れる体(初級)の配置.
	if (StageNo == 1) {
		{
			glm::vec3 position = player->position + glm::vec3(-5, 0, -5);
			position.y = heightMap.Height(position);
			const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
			SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
				mesh, "Kooni", 13, position, rotation);
			p->GetMesh()->Play("Wait");
			p->colLocal = Collision::CreateCapsule(
				glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
			items[1].Add(p);
		}

		//乗り移れる体(中級)の配置.
		{
			glm::vec3 position = player->position + glm::vec3(0, 0, -5);
			position.y = heightMap.Height(position);
			const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("Bikuni");
			SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
				mesh, "bikuni", 13, position, rotation);
			p->GetMesh()->Play("Idle");
			p->colLocal = Collision::CreateCapsule(
				glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
			items[2].Add(p);
		}

		//乗り移れる体(上級)の配置.
		{
			glm::vec3 position = player->position + glm::vec3(5, 0, -5);
			position.y = heightMap.Height(position);
			const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("Skeleton");
			SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
				mesh, "skeleton", 13, position, rotation);
			p->GetMesh()->Play("Idle");
			p->colLocal = Collision::CreateCapsule(
				glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
			items[3].Add(p);
		}
	}


//木を配置.
	//{
	//	const size_t treeCount = 10;
	//	enemies.Reserve(treeCount);
	//	const Mesh::FilePtr mesh = meshBuffer.GetFile("Res/red_pine_tree.gltf");
	//	for (size_t i = 0; i < treeCount; ++i) {
	//		//敵の位置を(50,50)-(150,150)の範囲からランダムに選択.
	//		glm::vec3 position(0);
	//		position.x = std::uniform_real_distribution<float>(50, 150)(rand);
	//		position.z = std::uniform_real_distribution<float>(50, 150)(rand);
	//		position.y = heightMap.Height(position);
	//		// 敵の向きをランダムに選択.
	//		glm::vec3 rotation(0);
	//		rotation.y = std::uniform_real_distribution<float>(0, 6.3f)(rand);
	//		if (position.y >= 4.0f) {
	//			StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
	//				mesh, "tree", 13, position, rotation);
	//			p->colLocal = Collision::CreateCapsule(
	//				glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
	//			trees.Add(p);
	//		}
	//	}
	//}

	if (StageNo == 1) {
		if (StClearedE) {
			//パーティクル・システムのテスト用にエミッターを追加.
			{
				//エミッター1個目.
				ParticleEmitterParameter ep;
				//ep.imagePath = "Res/DiskParticle.tga";
				ep.imagePath = "Res/FireParticle.tga";
				ep.tiles = glm::ivec2(2, 2);
				ep.position = glm::vec3(91, 0, 100);
				ep.position.y = heightMap.Height(ep.position) + 4;
				ep.emissionsPerSecond = 20.0f;
				ep.dstFactor = GL_ONE; // 加算合成.
				ep.gravity = 0;
				ParticleParameter pp;
				pp.scale = glm::vec2(0.5f);
				pp.color = glm::vec4(0.9f, 0.3f, 0.1f, 1.0f);
				particleSystem.Add(ep, pp);
			}
		}
		if (StClearedW) {
			//パーティクル・システムのテスト用にエミッターを追加.
			{
				//エミッター1個目.
				ParticleEmitterParameter ep;
				//ep.imagePath = "Res/DiskParticle.tga";
				ep.imagePath = "Res/FireParticle.tga";
				ep.tiles = glm::ivec2(2, 2);
				ep.position = glm::vec3(67, 0, 100);
				ep.position.y = heightMap.Height(ep.position) + 5;
				ep.emissionsPerSecond = 20.0f;
				ep.dstFactor = GL_ONE; // 加算合成.
				ep.gravity = 0;
				ParticleParameter pp;
				pp.scale = glm::vec2(0.5f);
				pp.color = glm::vec4(0.9f, 0.3f, 0.1f, 1.0f);
				particleSystem.Add(ep, pp);
			}
		}
		if (StClearedS) {
			//パーティクル・システムのテスト用にエミッターを追加.
			{
				//エミッター1個目.
				ParticleEmitterParameter ep;
				//ep.imagePath = "Res/DiskParticle.tga";
				ep.imagePath = "Res/FireParticle.tga";
				ep.tiles = glm::ivec2(2, 2);
				ep.position = glm::vec3(79, 0, 112);
				ep.position.y = heightMap.Height(ep.position) + 5;
				ep.emissionsPerSecond = 20.0f;
				ep.dstFactor = GL_ONE; // 加算合成.
				ep.gravity = 0;
				ParticleParameter pp;
				pp.scale = glm::vec2(0.5f);
				pp.color = glm::vec4(0.9f, 0.3f, 0.1f, 1.0f);
				particleSystem.Add(ep, pp);
			}
		}
		if (StClearedN) {
			//パーティクル・システムのテスト用にエミッターを追加.
			{
				//エミッター1個目.
				ParticleEmitterParameter ep;
				//ep.imagePath = "Res/DiskParticle.tga";
				ep.imagePath = "Res/FireParticle.tga";
				ep.tiles = glm::ivec2(2, 2);
				ep.position = glm::vec3(79, 0, 88);
				ep.position.y = heightMap.Height(ep.position) + 5;
				ep.emissionsPerSecond = 20.0f;
				ep.dstFactor = GL_ONE; // 加算合成.
				ep.gravity = 0;
				ParticleParameter pp;
				pp.scale = glm::vec2(0.5f);
				pp.color = glm::vec4(0.9f, 0.3f, 0.1f, 1.0f);
				particleSystem.Add(ep, pp);
			}
		}
	}
	//{
	//	//エミッター2個目.
	//	ParticleEmitterParameter ep;
	//	ep.imagePath = "Res/DiskParticle.tga";
	//	ep.position = glm::vec3(75, 0, 100);
	//	ep.position.y = heightMap.Height(ep.position);
	//	ep.angle = glm::radians(30.0f);
	//	ParticleParameter pp;
	//	pp.lifetime = 2;
	//	pp.scale = glm::vec2(0.2f);
	//	pp.velocity = glm::vec3(0, 3, 0);
	//	pp.color = glm::vec4(0.1f, 0.3f, 0.8f, 1.0f);
	//	particleSystem.Add(ep, pp);
	//	}

	{
		effects.Reserve(100);
		for (const auto& e : objectives) {
			for (size_t i = 0; i < 1; ++i) {
				const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("CurseEffect");
				const std::vector<Mesh::Animation>& animList = mesh->GetAnimationList();
				if (!animList.empty()) {
					mesh->Play(animList[0].name);
					mesh->SetAnimationSpeed(std::uniform_real_distribution<float>(0.5f, 1.5f)(rand));
					mesh->SetPosition(std::uniform_real_distribution<float>(0.0f, 2.5f)(rand));
				}
				glm::vec3 rotation(0);
				rotation.y = std::uniform_real_distribution<float>(-glm::pi<float>() * 0.5f, glm::pi<float>() * 0.5f)(rand);
				glm::vec3 scale(std::uniform_real_distribution<float>(0.5f, 1.5f)(rand));
				SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(mesh, "CurseEffect", 1, e->position, rotation, glm::vec3(0.5));
				p->scale = glm::vec3(1.25f);
				p->color = glm::vec4(1, 1, 1, 1);
				effects.Add(p);
			}
		}
		SceneFader::Instance().FadeIn(1);
	}
	// オープニングスクリプトを実行.
	if (!StClearedE && !StClearedN && !StClearedS && !StClearedW && StageNo == 1 && eventFrag == false) {
		camera.target = player->position;
		camera.position = camera.target + glm::vec3(0, 40, 60);
		SceneStack::Instance().Push(std::make_shared<EventScene>("Res/Event/OpeningScript.txt"));
		return true;
	}
	if (StageNo == 2) {
		camera.target = player->position;
		camera.position = player->position + glm::vec3(0, 40, 60);
		SceneStack::Instance().Push(std::make_shared<EventScene>("Res/Event/Stage2.txt"));
		return true;
	}
	else if (StageNo == 3) {
		camera.target = player->position;
		camera.position = camera.target + glm::vec3(0, 40, 60);
		SceneStack::Instance().Push(std::make_shared<EventScene>("Res/Event/Stage3.txt"));
		return true;
	}
	else if (StageNo == 4) {
		camera.target = player->position;
		camera.position = camera.target + glm::vec3(0, 40, 60);
		SceneStack::Instance().Push(std::make_shared<EventScene>("Res/Event/Stage4.txt"));
		return true;
	}
	else if (StageNo == 5) {
		camera.target = player->position;
		camera.position = camera.target + glm::vec3(0, 40, 60);
		SceneStack::Instance().Push(std::make_shared<EventScene>("Res/Event/Stage5.txt"));
		return true;
	}
}

/**
*プレイヤーの入力を処理する.
*/
void MainGameScene::ProcessInput() {
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();

	if (state == State::play) {
		player->ProcessInput();
	}
	//デバック用ボタン.
	//ステージのクリア表示.
	if (window.GetGamePad().buttonDown & GamePad::Z) {
		StClearedE = true;
		StClearedW = true;
		StClearedN = true;
		StClearedS = true;
	}
	//敵の残数を減らす.
	if (window.GetGamePad().buttonDown & GamePad::XX) {
		enemyBlow += 1;
	}
	//防衛ラインを減らす.
	if (window.GetGamePad().buttonDown & GamePad::C){
		defenceLine -= 10;
	}
	//全回復.
	if (window.GetGamePad().buttonDown & GamePad::V) {
		player->pHP += 100;
		player->pMP += 10;
	}
	//クリア条件.
	if (window.GetGamePad().buttonDown & GamePad::BB) {
		enemyBlow = 0;
	}
	//レベルアップ.
	if (window.GetGamePad().buttonDown & GamePad::C) {
		player->pExPoint = 0;
		player->pExCount = 0;
	}
	//MP減少.
	if (window.GetGamePad().buttonDown & GamePad::N) {
		player->pMP -= 5;
	}
	//HP減少.
	if (window.GetGamePad().buttonDown & GamePad::M) {
		player->pHP -= 10;
	}
	
		//メニュー画面の表示.
		if (state == State::play &&
			gameClearFlag == false && gameOverFlag == false && nextStateFlag == false &&
			window.GetGamePad().buttonDown & GamePad::SPACE) {
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			state = State::menu;
			selectCount = 1;
			if (player->playerID != 0) {
				player->GetMesh()->Play("Idle");
			}
			sprites[1].Scale(glm::vec2(1, 3.5f));
			sprites[2].Scale(glm::vec2(0.4f, 2.5f));
			sprites[3].Scale(glm::vec2(0.4f, 0.9f));
			sprites[2].Position(glm::vec3(-380, 85, 0));
			sprites[3].Position(glm::vec3(-380, -220, 0));
			if (player->playerID == 0) {
				sprites[12].Scale(glm::vec2(0.25f));
			}
			else if (player->playerID == 1) {
				sprites[13].Scale(glm::vec2(0.3f));
			}
			else if (player->playerID == 2) {
				sprites[14].Scale(glm::vec2(0.3f));
			}
			else if (player->playerID == 3) {
				sprites[15].Scale(glm::vec2(0.3f));
			}
			sprites[22].Scale(glm::vec2(1.08f, 0.11f));
			sprites[23].Scale(glm::vec2(0.1f));
			sprites[24].Scale(glm::vec2(0.1f));

			sprites[16].Scale(glm::vec2(0));
			sprites[17].Scale(glm::vec2(0));
			sprites[18].Scale(glm::vec2(0));
			sprites[19].Scale(glm::vec2(0));
			sprites[20].Scale(glm::vec2(0));
			sprites[21].Scale(glm::vec2(0));
			sprites[26].Scale(glm::vec2(0));
			sprites[27].Scale(glm::vec2(0));
			sprites[28].Scale(glm::vec2(0));
			sprites[31].Scale(glm::vec2(0));
			sprites[32].Scale(glm::vec2(0));
			sprites[29].Scale(glm::vec2(0));
			sprites[30].Scale(glm::vec2(0));
			sprites[33].Scale(glm::vec2(0));
			sprites[34].Scale(glm::vec2(0));
			sprites[35].Scale(glm::vec2(0));
			sprites[36].Scale(glm::vec2(0));
			sprites[37].Scale(glm::vec2(0));
		}
		//スペースボタンでも閉じれる.
		else if(window.GetGamePad().buttonDown & GamePad::SPACE)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			sprites[1].Scale(glm::vec2(0));
			sprites[2].Scale(glm::vec2(0));
			sprites[3].Scale(glm::vec2(0));
			sprites[12].Scale(glm::vec2(0));
			sprites[13].Scale(glm::vec2(0));
			sprites[14].Scale(glm::vec2(0));
			sprites[15].Scale(glm::vec2(0));
			sprites[16].Scale(glm::vec2(0));
			sprites[17].Scale(glm::vec2(0));
			sprites[22].Scale(glm::vec2(0));
			sprites[23].Scale(glm::vec2(0));
			sprites[24].Scale(glm::vec2(0));
			if (StageNo != 1) {
				sprites[26].Scale(glm::vec2(1));
				sprites[27].Scale(glm::vec2(1));
			}
			sprites[28].Scale(glm::vec2(0));
			
			state = State::play;
			selectCount = 0;
		}

		
		//スキルセット画面時の選択アイコンを動かす.
		if (state == State::sMenu && skComCount > -1) {

			//スキル画面操作.
			if (window.GetGamePad().buttonDown & GamePad::DPAD_RIGHT) {
				selectCount = -1;
				skComCount = 1;
			}
			else if (window.GetGamePad().buttonDown & GamePad::DPAD_LEFT) {
				selectCount = 3;
				skComCount = 0;
			}

			skSelectUI(1, 0, 4, 4, 5, 1);

			//スキル表の時.
			if (skComCount == 1) {
				//選択アイコン.
				sprites[22].Position(glm::vec3(-30, 160, 0));
				sprites[23].Position(glm::vec3(80, 160, 0));
				sprites[24].Position(glm::vec3(-140, 160, 0));
			}
			else if (skComCount == 2) {
				//選択アイコン.
				sprites[22].Position(glm::vec3(-30, 110, 0));
				sprites[23].Position(glm::vec3(80,110, 0));
				sprites[24].Position(glm::vec3(-140,110, 0));
			}
			else if (skComCount == 3) {
				//選択アイコン.
				sprites[22].Position(glm::vec3(-30, 60, 0));
				sprites[23].Position(glm::vec3(80, 60, 0));
				sprites[24].Position(glm::vec3(-140, 60, 0));
			}
			else if (skComCount == 4) {
				//選択アイコン.
				sprites[22].Position(glm::vec3(-30, 10, 0));
				sprites[23].Position(glm::vec3(80, 10, 0));
				sprites[24].Position(glm::vec3(-140, 10, 0));
			}
		}

		//メニュー画面時の選択アイコンを動かす.
		if (state == State::menu || state == State::cMenu || state == State::sMenu && selectCount > -1) {
			selectUI(1,0,5,5,6,1);

			//選択ボタンの操作.
				if (selectCount == 1) {
					//選択アイコン.
					sprites[22].Position(glm::vec3(-380, 210, 0));
					sprites[23].Position(glm::vec3(-270, 210, 0));
					sprites[24].Position(glm::vec3(-490, 210, 0));
				}
				else if (selectCount == 2) {
					//選択アイコン.
					sprites[22].Position(glm::vec3(-380, 160, 0));
					sprites[23].Position(glm::vec3(-270, 160, 0));
					sprites[24].Position(glm::vec3(-490, 160, 0));
				}
				else if (selectCount == 3) {
					//選択アイコン.
					sprites[22].Position(glm::vec3(-380, 110, 0));
					sprites[23].Position(glm::vec3(-270, 110, 0));
					sprites[24].Position(glm::vec3(-490, 110, 0));
				}
				else if (selectCount == 4) {
					//選択アイコン.
					sprites[22].Position(glm::vec3(-380, 10, 0));
					sprites[23].Position(glm::vec3(-270, 10, 0));
					sprites[24].Position(glm::vec3(-490, 10, 0));
				}
				else if (selectCount == 5) {
					//選択アイコン.
					sprites[22].Position(glm::vec3(-380, -40, 0));
					sprites[23].Position(glm::vec3(-270, -40, 0));
					sprites[24].Position(glm::vec3(-490, -40, 0));
				}
			}

		//ステージ移行の有無時の選択アイコンを動かす.
		if (state == State::select) {
			selectUI(1,0,2,2,3,1);
			
			//”はい”でステージ移行、”いいえ”でプレイに戻る.
			if (selectCount == 1 && window.GetGamePad().buttonDown & GamePad::START) {
				Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();

				player->position += glm::vec3(0, 100, 0);
				selectCount2 = 0;
				bgm->Stop();
				timer = 2.0f;
				selectCount = 0;
			}
			else if (selectCount == 2 && window.GetGamePad().buttonDown & GamePad::START) {
				Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
				sprites[2].Scale(glm::vec2(0));
				sprites[3].Scale(glm::vec2(0));
				sprites[22].Scale(glm::vec2(0));
				sprites[23].Scale(glm::vec2(0));
				sprites[24].Scale(glm::vec2(0));
				state = State::play;
				player->position = glm::vec3(79, 0, 100);
				selectCount = 0;
			}

			//ステージ移行の有無の選択ボタンの操作.
				if (selectCount == 1) {
					//選択アイコン.
					sprites[24].Position(glm::vec3(82, -75, 0));
					sprites[22].Position(glm::vec3(0, -75, 0));
					sprites[23].Position(glm::vec3(-83, -75, 0));
				}
				else if (selectCount == 2) {
					//選択アイコン.
					sprites[24].Position(glm::vec3(82, -115, 0));
					sprites[22].Position(glm::vec3(0, -115, 0));
					sprites[23].Position(glm::vec3(-83, -115, 0));
				}
		}


		//リザルト画面時の選択アイコンを動かす.
		if (state == State::result) {
			selectUI(1, 0, 2, 2, 3, 1);
			
			if (selectCount == 1) {
				//選択アイコン.
				sprites[22].Scale(glm::vec2(1.61f, 0.22f));
				sprites[23].Scale(glm::vec2(0.2f));
				sprites[24].Scale(glm::vec2(0.2f));
				sprites[24].Position(glm::vec3(260, 120, 0));
				sprites[22].Position(glm::vec3(100, 120, 0));
				sprites[23].Position(glm::vec3(-60, 120, 0));
			}
			else if (selectCount == 2) {
				//選択アイコン.
				sprites[22].Scale(glm::vec2(1.61f, 0.22f));
				sprites[23].Scale(glm::vec2(0.2f));
				sprites[24].Scale(glm::vec2(0.2f));
				sprites[24].Position(glm::vec3(260, -120, 0));
				sprites[22].Position(glm::vec3(100, -120, 0));
				sprites[23].Position(glm::vec3(-60, -125, 0));
			}

			//リザルト画面から次のステージ移行.
				if (selectCount == 1 && window.GetGamePad().buttonDown & GamePad::START)
				{
					Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
					state = State::play;
					nextStateFlag = false;
					selectCount = 0;
					StageNo = 1;
					SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
					return;
				}
				else if (selectCount == 2 && window.GetGamePad().buttonDown & GamePad::START) {
					Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
					selectCount = 0;
					loadTimer = 0.5f;
				}
		}

		//選択アイコン使用結果.
		//プレイヤー情報画面.
		if (selectCount == 1 && nextStateFlag == false &&
			window.GetGamePad().buttonDown & GamePad::START) {
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			state = State::menu;

			if (player->playerID == 0) {
				sprites[12].Scale(glm::vec2(0.25f));
			}
			else if (player->playerID == 1) {
				sprites[13].Scale(glm::vec2(0.3f));
			}
			else if (player->playerID == 2) {
				sprites[14].Scale(glm::vec2(0.3f));
			}
			else if (player->playerID == 3) {
				sprites[15].Scale(glm::vec2(0.3f));
			}

		}
		//コマンド表.
		else if (selectCount == 2 && nextStateFlag == false &&
			window.GetGamePad().buttonDown & GamePad::START) {
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			sprites[12].Scale(glm::vec2(0));
			sprites[13].Scale(glm::vec2(0));
			sprites[14].Scale(glm::vec2(0));
			sprites[15].Scale(glm::vec2(0));

			sprites[16].Scale(glm::vec2(0));
			sprites[17].Scale(glm::vec2(0));
			gamePadText = true;
			state = State::cMenu;
		}
			//スキル表.
		else if (selectCount == 3 && nextStateFlag == false &&
			window.GetGamePad().buttonDown & GamePad::START) {
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			sprites[12].Scale(glm::vec2(0));
			sprites[13].Scale(glm::vec2(0));
			sprites[14].Scale(glm::vec2(0));
			sprites[15].Scale(glm::vec2(0));

			sprites[16].Scale(glm::vec2(0));
			sprites[17].Scale(glm::vec2(0));
			state = State::sMenu;
		}
		//タイトルに戻る.
		else if (selectCount == 4 && nextStateFlag == false &&
			window.GetGamePad().buttonDown & GamePad::START) {
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			bgm->Stop();
			StageNo = 1;
			eventFrag = true;
			loadTimer = 0.5f;
		}
		//プレイに戻る.
		else if (selectCount == 5 && nextStateFlag == false &&
			window.GetGamePad().buttonDown & GamePad::START) {

			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			sprites[1].Scale(glm::vec2(0));
			sprites[2].Scale(glm::vec2(0));
			sprites[3].Scale(glm::vec2(0));
			sprites[12].Scale(glm::vec2(0));
			sprites[13].Scale(glm::vec2(0));
			sprites[14].Scale(glm::vec2(0));
			sprites[15].Scale(glm::vec2(0));
			sprites[16].Scale(glm::vec2(0));
			sprites[17].Scale(glm::vec2(0));
			sprites[22].Scale(glm::vec2(0));
			sprites[23].Scale(glm::vec2(0));
			sprites[24].Scale(glm::vec2(0));
			if (StageNo != 1) {
				sprites[26].Scale(glm::vec2(1));
				sprites[27].Scale(glm::vec2(1));
			}
			sprites[28].Scale(glm::vec2(0));

			state = State::play;
			selectCount = 0;
		}

		if (state == State::cMenu) {
			if (gamePadText && window.GetGamePad().buttonDown & GamePad::Y) {
				Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
				gamePadText = false;
			}
			else if (!gamePadText && window.GetGamePad().buttonDown & GamePad::Y) {
				Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
				gamePadText = true;
			}
		}

		//視点切り替え.
		if (window.GetGamePad().buttons & GamePad::YY) {

			cameraFar = true;
		}
		else {
			cameraFar = false;
		}
		if (window.GetGamePad().buttons & GamePad::I) {

			cameraNear = true;
		}
		else {
			cameraNear = false;
		}

		//範囲攻撃用フラグ立て.
		if (player->playerID == 3 && window.GetGamePad().buttons & GamePad::B) {

			shotTimerFragB = true;
			shotTimerFragC = true;
		}

		//スキルセット攻撃用フラグ立て.

		//初期攻撃用フラグ立て.
		if (window.GetGamePad().buttons & GamePad::A) {

			sCommand = true;
		}
		else {
			sCommand = false;
		}

		//強攻撃用フラグ立て.
		if (wCommand == false && window.GetGamePad().buttons & GamePad::X) {

			wCommand = true;
			/*sprites[36].Scale(glm::vec2(0));*/
		}

		//溜め攻撃用フラグ立て.
		if (eCommand == false && window.GetGamePad().buttons & GamePad::B) {

			chargeShotFlagA = true;
			eCommand = true;
		}

		//遠距離攻撃用フラグ立て.
		if (nCommand == false && window.GetGamePad().buttons & GamePad::Y) {

			shotTimerFragA = true;
			nCommand = true;
		}
}

/**
*シーンを更新する.
*
*@param deltaTime	前回からの更新からの経過時間（秒）.
*/
void MainGameScene::Update(float deltaTime) {

	const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
	spriteRenderer.BeginUpdate();
	for (const Sprite& e : sprites) {
		spriteRenderer.AddVertices(e);
	}
	spriteRenderer.EndUpdate();

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const float w = window.Width();
	const float h = window.Height();
	const float lineHeight = fontRenderer.LineHeight();

	// カメラの状態を更新.
	{
		if (state != State::select) {
			camera.target = player->position;
			camera.position = camera.target + glm::vec3(0, 15, 25);
		}
		if (cameraFar == true) {
			camera.position = camera.target + glm::vec3(0, 10, 20);
			camera.velocity += glm::vec3(3);
		}

		if (cameraNear == true) {
			camera.position = camera.target + glm::vec3(0, 60, 30);
			camera.velocity += glm::vec3(3);
		}
	}

	//ステートがプレイの時のみ.
	if (state == State::play && nextStateFlag == false) {
		player->Update(deltaTime);
		enemies[0].Update(deltaTime);
		enemies[1].Update(deltaTime);
		enemies[2].Update(deltaTime);
		enemies[3].Update(deltaTime);
		trees.Update(deltaTime);
		objects.Update(deltaTime);
		defencePoint.Update(deltaTime);
		walls.Update(deltaTime);
		lights.Update(deltaTime);
		particleSystem.Update(deltaTime);
		items[0].Update(deltaTime);
		items[1].Update(deltaTime);
		items[2].Update(deltaTime);
		items[3].Update(deltaTime);
		warp[0].Update(deltaTime);
		warp[1].Update(deltaTime);
		warp[2].Update(deltaTime);
		warp[3].Update(deltaTime);
		bullet[0].Update(deltaTime);
		upItem[0].Update(deltaTime);
		upItem[1].Update(deltaTime);
		upItem[2].Update(deltaTime);
	}

	/*if (actionWaitTimer > 0) {
		actionWaitTimer -= deltaTime;
	}*/
	if (StageNo != 1) {
		DetectCollision(player, enemies[0]);
		DetectCollision(player, enemies[1]);
		DetectCollision(player, enemies[2]);
		DetectCollision(player, enemies[3]);
		DetectCollision(player, defencePoint);
		DetectCollision(defencePoint, enemies[0]);
		DetectCollision(defencePoint, enemies[1]);
		DetectCollision(defencePoint, enemies[2]);
		DetectCollision(defencePoint, enemies[3]);
	}
	DetectCollision(player, trees);
	DetectCollision(player, objects);
	DetectCollision(player, objectives);
	DetectCollision(player, walls);

	//敵の出現.
	if (StageNo != 1) {	
		enemyPopTimerA += deltaTime;
		enemyPopTimerB += deltaTime;
		enemyPopTimerC += deltaTime;

		EnemySpawn();
		EnemyAI(deltaTime, defencePoint, 0, 0);
		EnemyAI(deltaTime, defencePoint, 1, 1);
		EnemyAI(deltaTime, defencePoint, 2, 2);
		//敵が追いかけてくる.
		for (auto& e : enemies[3]) {
			SkeletalMeshActorPtr enemy = std::static_pointer_cast<SkeletalMeshActor>(e);
			Mesh::SkeletalMeshPtr mesh = enemy->GetMesh();

			//死亡したら消す.
			if (mesh->GetAnimation() == "Down") {
				if (mesh->IsFinished()) {
					enemy->health = 0;
					enemyBlow += 1;
					player->pExPoint -= 10;
					player->pExCount -= 50;

					enemySpawn += 1;
				}
				continue;
			}

			const float moveSpeed = baseSpeed * 3.0f;
			const float rotationSpeed = baseSpeed * glm::radians(60.0f);
			const float frontRange = glm::radians(15.0f);

			const glm::vec3 v = player->position - e->position;
			const glm::vec3 vTarget = glm::normalize(v);
			float radian = std::atan2(-vTarget.z, vTarget.x) - glm::radians(90.0f);

			if (radian <= 0) {
				radian += glm::radians(360.0f);
			}
			const glm::vec3 vEnemyFront = glm::rotate(
				glm::mat4(1), e->rotation.y + glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);

			//ターゲットが正面にいなかったら、正面にとらえるような左右に旋回.
			if (std::abs(radian - e->rotation.y) > frontRange) {
				const glm::vec3 vRotDir = glm::cross(vEnemyFront, vTarget);
				if (vRotDir.y >= 0) {
					e->rotation.y += rotationSpeed * deltaTime;
					if (e->rotation.y >= glm::radians(360.0f)) {
						e->rotation.y -= glm::radians(360.0f);
					}
				}
				else {
					e->rotation.y -= rotationSpeed * deltaTime;
					if (e->rotation.y < 0) {
						e->rotation.y += glm::radians(360.0f);
					}
				}
			}
			if (e->health <= 0) {
				e->velocity = glm::vec3(0);	//死んでいるのでもう移動しない.
			}
			//十分に接近していなければ移動する。接近していれば攻撃する.
			if (glm::length(v) > 2.5f) {
				e->velocity = vEnemyFront * moveSpeed;
				if (mesh->GetAnimation() != "Run") {
					mesh->Play("Run");
				}
			}
			else {
				e->velocity = glm::vec3(0);	//接近しているのでもう移動しない.
				if (mesh->GetAnimation() != "Wait") {
					if ((mesh->GetAnimation() != "Attack" && mesh->GetAnimation() != "Hit") ||
						mesh->IsFinished()) {
						mesh->Play("Wait");
					}
				}						//定期的に攻撃状態になる.
				if (isAttacking) {
					isAttacking = false;
					attackingTimer = 3.0f;	//次の攻撃は５秒後.
					mesh->Play("Attack", false);
				}
				else {
					attackingTimer -= deltaTime;
					if (attackingTimer <= 0) {
						isAttacking = true;
						player->pHP -= 10;
					}
				}
			}
		}
		EnemyDetectCollision(0);
		EnemyDetectCollision(1);
		EnemyDetectCollision(2);
		EnemyDetectCollision(3);
	}
	
	if (player->playerID == 2 && player->pAbility >= 3) {
		if (state == State::play) {
			//プレイヤーの前方に発射.
			const Mesh::FilePtr meshShot = meshBuffer.GetFile("Res/Triangle.gltf");
			if (shotTimerFragA == true) {
				playerBulletTimerA -= deltaTime;

				if (playerBulletTimerA <= -0.01f) {

					StaticMeshActorPtr Shot = std::make_shared<StaticMeshActor>(
						meshShot, "Shot", 100, player->position, glm::vec3(0, 0, 0));
					const float speed = 10.0f;	//弾の移動速度(m/秒).
					const int x[] = { 0,10,-10,20,-20 };
					const glm::mat4 matRotY =
						glm::rotate(glm::mat4(1), player->rotation.y, glm::vec3(0, 1, 0));
					glm::vec3 rot = player->rotation;
					rot.y += std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
					Shot->rotation += rot.y;
					Shot->scale = glm::vec3(1, 1, 1);
					Shot->colLocal = Collision::CreateCapsule(
						glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
					Shot->velocity = matRotY * glm::vec4(0, 0, speed, 1);
					bullet[0].Add(Shot);
					playerBulletTimerA = 100.0f;
				}
				else if (playerBulletTimerA <= 98.0f) {
					for (ActorPtr& e : bullet[0]) {
						e->health = 0;
						shotTimerFragA = false;
						playerBulletTimerA = 0.0f;
					}
				}
			}

			//溜め攻撃.
			if (player->playerID == 2 && player->pAbility >= 4) {
				if (chargeShotFlagA == true) {
					playerBulletTimerB -= deltaTime;
					const Mesh::FilePtr meshMeteo = meshBuffer.GetFile("Res/Triangle.gltf");
					const float speed = 8.0f;	//弾の移動速度(m/秒).
					const glm::mat4 matRotY =
						glm::rotate(glm::mat4(1), player->rotation.y, glm::vec3(0, 1, 0));
					/*Audio::Engine::Instance().Prepare("Res/Audio/PlayerShot.xwm")->Play();*/

					if (playerBulletTimerB <= -2.0f) {
						glm::vec3 setPosition = player->position + glm::vec3(0, 5, 0);
						StaticMeshActorPtr Meteo = std::make_shared<StaticMeshActor>(
							meshMeteo, "Shot", 100, setPosition, glm::vec3(0, 0, 0));
						Meteo->scale = glm::vec3(3);
						Meteo->velocity = matRotY * glm::vec4(0, -speed, speed, 1);
						bullet[0].Add(Meteo);
						playerBulletTimerB = 100.0f;

					}
					else if (playerBulletTimerB <= 98.0f) {
						for (ActorPtr& e : bullet[0]) {
							e->health = 0;
							chargeShotFlagA = false;
							playerBulletTimerB = 0.0f;
						}
					}
				}
			}
		}
	}

	//if (player->playerID == 3) {
	//	if (state == State::play) {
	//		//プレイヤーの周囲に出す.
	//		const Mesh::FilePtr meshRangeStone = meshBuffer.GetFile("Res/RangeStone.gltf");

	//		if (shotTimerFragB == true && shotTimerFragC == true) {
	//			playerBulletTimerC -= deltaTime;
	//			playerBulletTimerD -= deltaTime;

	//			if (playerBulletTimerC <= -0.1f) {
	//				StaticMeshActorPtr Shot = std::make_shared<StaticMeshActor>(
	//					meshRangeStone, "Shot", 100, player->position + glm::vec3(0, -0.7f, 0), glm::vec3(0, 0, 0));
	//				Shot->colLocal = Collision::CreateCapsule(
	//					glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
	//				bullet[0].Add(Shot);
	//				playerBulletTimerC = 10.0f;
	//			}
	//			else if (playerBulletTimerC <= 9.0f) {
	//				for (ActorPtr& e : bullet[0]) {
	//					e->health = 0;
	//					playerBulletTimerC = 0.0f;
	//					shotTimerFragB = false;
	//				}
	//			}
	//			if (playerBulletTimerD <= -1.5f) {
	//				StaticMeshActorPtr Shot = std::make_shared<StaticMeshActor>(
	//					meshRangeStone, "Shot", 100, player->position + glm::vec3(0, 0.5f, 0), glm::vec3(0, 0, 0));
	//				Shot->colLocal = Collision::CreateCapsule(
	//					glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
	//				bullet[0].Add(Shot);
	//			}
	//			else if (playerBulletTimerD <= -3.0f) {
	//				for (ActorPtr& e : bullet[0]) {
	//					e->health = 0;
	//					playerBulletTimerD = 0.0f;
	//					shotTimerFragC = false;
	//				}
	//			}
	//		}
	//	}
	//}

	//アイテムの出現.
	if (StageNo != 1) {
		if (state == State::play) {
			itemTimerA -= deltaTime;
			itemTimerB -= deltaTime;
			itemTimerC -= deltaTime;

			const Mesh::FilePtr meshUpItemHP = meshBuffer.GetFile("Res/HP.gltf");
			const Mesh::FilePtr meshUpItemMP = meshBuffer.GetFile("Res/MP.gltf");
			const Mesh::FilePtr meshUpItemHPMP = meshBuffer.GetFile("Res/HPMP.gltf");
			if (itemTimerA <= -5.0f) {
				//ランダムにアイテムを配置
				glm::vec3 position(0);
				position.x = static_cast<float>(std::uniform_int_distribution<>(50, 150)(rand));
				position.z = static_cast<float>(std::uniform_int_distribution<>(50, 150)(rand));
				position.y = heightMap.Height(position) + 1;

				StaticMeshActorPtr itemHP = std::make_shared<StaticMeshActor>(
					meshUpItemHP, "HP", 100, position, glm::vec3(0, 0, 0));	

				itemHP->colLocal = Collision::CreateCapsule(
					glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
				itemHP->scale = glm::vec3(0.7f, 1, 0.7f);

				upItem[0].Add(itemHP);
				itemTimerA = 100.0f;
			}
			else if (itemTimerA <= 95.0f) {
				for (ActorPtr& e : upItem[0]) {
					e->health = 0;
				}
				itemTimerA = 0.0f;
			}
			if (itemTimerB <= -5.0f) {
				glm::vec3 position(0);
				position.x = static_cast<float>(std::uniform_int_distribution<>(50, 100)(rand));
				position.z = static_cast<float>(std::uniform_int_distribution<>(50, 100)(rand));
				position.y = heightMap.Height(position) + 1;
				StaticMeshActorPtr itemMP = std::make_shared<StaticMeshActor>(
					meshUpItemMP, "MP", 100, position, glm::vec3(0, 0, 0));
				itemMP->colLocal = Collision::CreateCapsule(
					glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
				itemMP->scale = glm::vec3(0.7f, 1, 0.7f);
				upItem[1].Add(itemMP);

				itemTimerB = 0.0f;
			}
			else if (itemTimerB <= 95.0f) {
				for (ActorPtr& e : upItem[1]) {
					e->health = 0;
				}
				itemTimerB = 0.0f;
			}
			if (itemTimerC <= -5.0f) {
				glm::vec3 position(0);
				position.x = static_cast<float>(std::uniform_int_distribution<>(50, 100)(rand));
				position.z = static_cast<float>(std::uniform_int_distribution<>(50, 100)(rand));
				position.y = heightMap.Height(position) + 1;
				StaticMeshActorPtr itemAll = std::make_shared<StaticMeshActor>(
					meshUpItemHPMP, "All", 100, position, glm::vec3(0, 0, 0));

				itemAll->colLocal = Collision::CreateCapsule(
					glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
				itemAll->scale = glm::vec3(0.7f, 1, 0.7f);

				upItem[2].Add(itemAll);
				itemTimerC = 100.0f;
			}
			else if (itemTimerC <= 95.0f) {
				for (ActorPtr& e : upItem[2]) {
					e->health = 0;
				}
				itemTimerC = 0.0f;
			}
		}
	}

	//右のステージ移行.
	if (state == State::play) {

		//アイテムの取得.
		DetectCollision(player, upItem[0],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			b->health = 0;
			Audio::Engine::Instance().Prepare("Res/Audio/CharacterGet.mp3")->Play();
			player->pHP = player->maxHP;
		}
		);

		DetectCollision(player, upItem[1],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			b->health = 0;
			Audio::Engine::Instance().Prepare("Res/Audio/CharacterGet.mp3")->Play();
			player->pMP = player->maxMP;
		}
		);

		DetectCollision(player, upItem[2],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			b->health = 0;
			Audio::Engine::Instance().Prepare("Res/Audio/CharacterGet.mp3")->Play();
			player->pHP = player->maxHP;
			player->pMP = player->maxMP;
		}
		);

		////敵と自分の攻撃.
		//DetectCollision(bullet[0], enemies[0],
		//	[this](const ActorPtr& a, const ActorPtr&b, const glm::vec3& p) {
		//	b->health = 0;
		//});

		////敵と自分の攻撃.
		//DetectCollision(bullet[0], enemies[1],
		//	[this](const ActorPtr& a, const ActorPtr&b, const glm::vec3& p) {
		//	b->health = 0;
		//});

		////敵と自分の攻撃.
		//DetectCollision(bullet[0], enemies[2],
		//	[this](const ActorPtr& a, const ActorPtr&b, const glm::vec3& p) {
		//	b->health = 0;
		//});

		//右のステージ移行.
		DetectCollision(player, warp[0],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			b->health = 1;
			stageChage = 1;
			allTimer = 0.01f;
		}
		);

		//左のステージ移行.
		DetectCollision(player, warp[1],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			b->health = 1;
			stageChage = 2;
			allTimer = 0.01f;
		}
		);

		//下のステージ移行.
		DetectCollision(player, warp[2],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			b->health = 1;
			stageChage = 3;
			allTimer = 0.01f;
		}
		);

		//上のステージ移行.
		DetectCollision(player, warp[3],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			b->health = 1;
			stageChage = 4;
			allTimer = 0.01f;
		}
		);
	}

	//魂(ソウル).
	if (player->playerID != 0) {
		DetectCollision(player, items[0],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			b->health = 0;
			Audio::Engine::Instance().Prepare("Res/Audio/CharacterGet.mp3")->Play();
			player->SetStaticMesh(meshBuffer.GetFile("Res/Blackshadow.gltf"), 0);
			player->playerID = 0;
		}
		);
	}

	//ゴブリン(初級).
	if (player->playerID != 1) {
		DetectCollision(player, items[1],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			/*b->health = 0;*/
			Audio::Engine::Instance().Prepare("Res/Audio/CharacterGet.mp3")->Play();
			player->SetMesh(meshBuffer.GetSkeletalMesh("oni_small"), 0);
			player->GetMesh()->Play("Run");
			/*player->pType = std::uniform_int_distribution<>(1, 6)(rand);*/
			player->playerID = 1;
		}
		);
	}

	//人間(中級).
	if (player->playerID != 2) {
		DetectCollision(player, items[2],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			/*b->health = 0;*/
			Audio::Engine::Instance().Prepare("Res/Audio/CharacterGet.mp3")->Play();
			player->SetMesh(meshBuffer.GetSkeletalMesh("Bikuni"), 0);
			player->GetMesh()->Play("Run");
			/*player->pType = std::uniform_int_distribution<>(1, 6)(rand);*/
			player->playerID = 2;
		}
		);
	}

	//スケルトン(上級).
	if (player->playerID != 3) {
		DetectCollision(player, items[3],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p) {
			/*b->health = 0;*/
			Audio::Engine::Instance().Prepare("Res/Audio/CharacterGet.mp3")->Play();
			player->SetMesh(meshBuffer.GetSkeletalMesh("Skeleton"), 0);
			player->GetMesh()->Play("Walk");
			/*player->pType = std::uniform_int_distribution<>(1, 6)(rand);*/
			player->playerID = 3;
		}
		);
	}

	effects.Update(deltaTime);
	for (auto& e : effects) {
		Mesh::SkeletalMeshPtr mesh = std::static_pointer_cast<SkeletalMeshActor>(e)->GetMesh();
		if (mesh->IsFinished()) {
			e->health = 0;
		}
		else if (e->name == "Effect.Hit") {
			const float total = mesh->GetTotalAnimationTime();
			const float cur = mesh->GetPosition();
			const float x = std::max(0.0f, cur / total - 0.5f) * 2.0f;
			e->color.a = (1.0f - x);
		}
	}
	objectives.Update(deltaTime);

	// ライトの更新.
	glm::vec3 ambientColor(0.1f, 0.05f, 0.15f);
	lightBuffer.Update(lights, ambientColor);
	for (auto e : objects) {
		const std::vector<ActorPtr> neighborhood = lights.FindNearbyActors(e->position, 20);
		std::vector<int> pointLightIndex;
		std::vector<int> spotLightIndex;
		pointLightIndex.reserve(neighborhood.size());
		spotLightIndex.reserve(neighborhood.size());
		for (auto light : neighborhood) {
			if (PointLightActorPtr p = std::dynamic_pointer_cast<PointLightActor>(light)) {
				if (pointLightIndex.size() < 8) {
					pointLightIndex.push_back(p->index);
				}
			}
			else if (
				SpotLightActorPtr p = std::dynamic_pointer_cast<SpotLightActor>(light)) {
				if (spotLightIndex.size() < 8) {
					spotLightIndex.push_back(p->index);
				}
			}
		}
		StaticMeshActorPtr p = std::static_pointer_cast<StaticMeshActor>(e);
		p->SetPointLightList(pointLightIndex);
		p->SetSpotLightList(spotLightIndex);
	}

	//経験値でレベルアップ.
	if (player->pExPoint <= 0) {
		Audio::Engine::Instance().Prepare("Res/Audio/LVUP.mp3")->Play();
		player->pLevel += 1;
		player->pAbility += 1;
		player->pExPoint = 100 * player->pLevel;
		player->pExCount = 500 * player->pLevel;
		player->maxHP += 100;
		player->maxMP += 10;
		player->pHP = player->maxHP;
		player->pMP = player->maxMP;
	}

	////落ちたら体力が減ってスタート地点に戻る
	//if (player->position.y <= 0.0f) {

	//	glm::vec3 startPos(79, 0, 100);
	//	startPos.y = heightMap.Height(startPos);
	//	player->position = startPos;

	//	player->pHP -= 100;
	//	player->SetStaticMesh(meshBuffer.GetFile("Res/Blackshadow.gltf"), 0);
	//	player->playerID = 0;
	//}

	//HPが０か防衛ラインのHPが０になったらゲームオーバーフラグが立つ.
	if (player->pHP <= 0 || defenceLine <= 0) {
		if (gameClearFlag == false && nextStateFlag == false) {
			gameOverFlag = true;
		}
	}

	//敵を全滅させたらゲームクリア.
	if (player->pHP > 0 && defenceLine > 0 && enemyStock == enemyBlow) {
		if (gameOverFlag == false && nextStateFlag == false) {
			gameClearFlag = true;
		}
	}
	
	player->UpdateDrawData(deltaTime);
	enemies[0].UpdateDrawData(deltaTime);
	enemies[1].UpdateDrawData(deltaTime);
	enemies[2].UpdateDrawData(deltaTime);
	enemies[3].UpdateDrawData(deltaTime);
	trees.UpdateDrawData(deltaTime);
	objects.UpdateDrawData(deltaTime);
	defencePoint.UpdateDrawData(deltaTime);
	walls.UpdateDrawData(deltaTime);
	effects.UpdateDrawData(deltaTime);
	items[0].UpdateDrawData(deltaTime);
	items[1].UpdateDrawData(deltaTime);
	items[2].UpdateDrawData(deltaTime);
	items[3].UpdateDrawData(deltaTime);
	warp[0].UpdateDrawData(deltaTime);
	warp[1].UpdateDrawData(deltaTime);
	warp[2].UpdateDrawData(deltaTime);
	warp[3].UpdateDrawData(deltaTime);
	bullet[0].UpdateDrawData(deltaTime);
	upItem[0].UpdateDrawData(deltaTime);
	upItem[1].UpdateDrawData(deltaTime);
	upItem[2].UpdateDrawData(deltaTime);

	fontRenderer.BeginUpdate();

	//ゲームオーバー時.
	if (gameOverFlag == true) {
		bgm->Stop();
		Audio::Engine::Instance().Prepare("Res/Audio/GameOver.mp3")->Play();
		sprites[10].Scale(glm::vec2(1));
		gameOverFlag = false;
		nextStateFlag = true;
		defenceFrag = false;
		overTimer = 3.0f;
	}
	//ゲームクリア時.
	else if (gameClearFlag == true) {
		bgm->Stop();
		Audio::Engine::Instance().Prepare("Res/Audio/GameClear.mp3")->Play();
		sprites[11].Scale(glm::vec2(1));
		gameClearFlag = false;
		nextStateFlag = true;
		defenceFrag = false;
		clearTimer = 3.0f;
	}

	if (StageNo != 1) {
		sprites[26].Scale(glm::vec2(1));
		sprites[27].Scale(glm::vec2(1));
		//防衛ラインのHPバーの表示.
		//攻撃を受けている時.
		if (defenceFrag == true) {
			sprites[28].Scale(glm::vec2(0));
			if (defenceLine >= 0) {
				if (state == State::play) {
					sprites[25].Scale(glm::vec2(10.0f * defenceLine / 100, 0.4f));
					sprites[25].Position(glm::vec3((160 * 2.5f * defenceLine / 100) / 2 - 150, 350, 0));
				}
				else {
					sprites[25].Scale(glm::vec2(0));
					sprites[26].Scale(glm::vec2(0));
					sprites[27].Scale(glm::vec2(0));
				}
			}
		}
		//攻撃を受けていない時.
		else if (defenceFrag == false) {
			sprites[25].Scale(glm::vec2(0));
			if (defenceLine >= 0) {
				if (state == State::play) {
					sprites[28].Scale(glm::vec2(10.0f * defenceLine / 100, 0.4f));
					sprites[28].Position(glm::vec3((160 * 2.5f * defenceLine / 100) / 2 - 150, 350, 0));
				}
				else {
					sprites[26].Scale(glm::vec2(0));
					sprites[27].Scale(glm::vec2(0));
					sprites[28].Scale(glm::vec2(0));
				}
			}
		}
	}

	//HPバーの表示.
	if (player->pHP >= 0) {
		if (state == State::play) {
			sprites[16].Scale(glm::vec2(5.0f * player->pHP / player->maxHP, 0.4f));
			sprites[16].Position(glm::vec3((80 * 2.5f * player->pHP / player->maxHP) / 2 - 570, 310, 0));
		}
		else if (state == State::menu) {
			sprites[16].Scale(glm::vec2(5.0f * player->pHP / player->maxHP, 0.4f));
			sprites[16].Position(glm::vec3((80 * 2.5f * player->pHP / player->maxHP) / 2 + 150, 110, 0));
		}
	}
	//MPバーの表示.
	if (player->pMP >= 0) {
		if (state == State::play) {
			sprites[17].Scale(glm::vec2(2.5f * player->pMP / player->maxMP, 0.4f));
			sprites[17].Position(glm::vec3((40 * 2.5f * player->pMP / player->maxMP) / 2 - 570, 260, 0));
		}
		else if (state == State::menu) {
			sprites[17].Scale(glm::vec2(2.5f * player->pMP / player->maxMP, 0.4f));
			sprites[17].Position(glm::vec3((40 * 2.5f * player->pMP / player->maxMP) / 2 + 150, 60, 0));
		}
	}

	//各キャラのスキルコマンドのアイコン.
	if (state == State::play) {
		//魂のスキルコマンドパネル.
		if (player->playerID == 0) {
			sprites[33].Scale(glm::vec2(0.2f));

			sprites[18].Scale(glm::vec2(0));
			sprites[19].Scale(glm::vec2(0));
			sprites[20].Scale(glm::vec2(0));
			sprites[21].Scale(glm::vec2(0));
			sprites[31].Scale(glm::vec2(0));
			sprites[32].Scale(glm::vec2(0));
			sprites[29].Scale(glm::vec2(0));
			sprites[30].Scale(glm::vec2(0));
		}
		else if (player->playerID == 1) {
			sprites[31].Scale(glm::vec2(0.2f));
			sprites[32].Scale(glm::vec2(0.2f));

			sprites[18].Scale(glm::vec2(0));
			sprites[19].Scale(glm::vec2(0));
			sprites[20].Scale(glm::vec2(0));
			sprites[21].Scale(glm::vec2(0));
			sprites[29].Scale(glm::vec2(0));
			sprites[30].Scale(glm::vec2(0));
			sprites[33].Scale(glm::vec2(0));
		}
		else if (player->playerID == 2) {
			sprites[18].Scale(glm::vec2(0.2f));
			sprites[19].Scale(glm::vec2(0.2f));
			sprites[20].Scale(glm::vec2(0.2f));
			sprites[21].Scale(glm::vec2(0.12f));

			sprites[31].Scale(glm::vec2(0));
			sprites[32].Scale(glm::vec2(0));
			sprites[29].Scale(glm::vec2(0));
			sprites[30].Scale(glm::vec2(0));
			sprites[33].Scale(glm::vec2(0));
		}
		else if (player->playerID == 3) {
			sprites[18].Scale(glm::vec2(0.2f));
			sprites[19].Scale(glm::vec2(0.2f));
			sprites[29].Scale(glm::vec2(0.2f));
			sprites[30].Scale(glm::vec2(0.2f));

			sprites[20].Scale(glm::vec2(0));
			sprites[21].Scale(glm::vec2(0));
			sprites[31].Scale(glm::vec2(0));
			sprites[32].Scale(glm::vec2(0));
			sprites[33].Scale(glm::vec2(0));
		}
	}

	//アイコンの使用可、不可.
	//攻撃のインターバル画像を重ねる.
	if (state == State::play) {

		//初期攻撃したかどうか.
		if (sCommand == true) {
			//下.
			sprites[36].Scale(glm::vec2(0));
		}
		else if (sCommand == false) {
			sprites[36].Scale(glm::vec2(1.55f, 1.55f));
			sprites[36].Position(glm::vec3(500, -310, 0));
		}

		//左.
		if (wCommand == true) {
			wIntTimer += deltaTime;
			
			sprites[35].Scale(glm::vec2(1.55f, 1.55f * wIntTimer / 2.0f));
			sprites[35].Position(glm::vec3(420, 24.8f * 2.5f * wIntTimer / 2.0f / 2 - 261, 0));
		}
		else if (wCommand == false) {
			sprites[35].Scale(glm::vec2(1.55f, 1.55f));
			sprites[35].Position(glm::vec3(420, -230, 0));
		}

		//右.
		if (eCommand == true) {
			eIntTimer += deltaTime;
			
			sprites[34].Scale(glm::vec2(1.55f, 1.55f * eIntTimer / 4.0f));
			sprites[34].Position(glm::vec3(580, 24.8f * 2.5f * eIntTimer / 4.0f / 2 - 261, 0));
		}
		else if (eCommand == false) {
			sprites[34].Scale(glm::vec2(1.55f, 1.55f));
			sprites[34].Position(glm::vec3(580, -230, 0));
		}
		//上.
		if (nCommand == true) {
			nIntTimer += deltaTime;
			
			sprites[37].Scale(glm::vec2(1.55f, 1.55f * nIntTimer / 3.0f));
			sprites[37].Position(glm::vec3(500, 24.8f * 2.5f * nIntTimer / 3.0f / 2 - 181, 0));
		}
		else if (nCommand == false) {
			sprites[37].Scale(glm::vec2(1.55f, 1.55f));
			sprites[37].Position(glm::vec3(500, -150, 0));
		}

		if(player->playerID == 1){
			sprites[34].Scale(glm::vec2(0));
			sprites[37].Scale(glm::vec2(0));
		}
		else if (player->playerID == 0) {
			sprites[34].Scale(glm::vec2(0));
			sprites[35].Scale(glm::vec2(0));
			sprites[37].Scale(glm::vec2(0));
		}

			if (wIntTimer >= 2.0f) {
				wCommand = false;
				wIntTimer = 0.0f;
			}
			if (eIntTimer >= 4.0f) {
				eCommand = false;
				eIntTimer = 0.0f;
			}
			if (nIntTimer >= 3.0f) {
				nCommand = false;
				nIntTimer = 0.0f;
			}
	}

	if (timer > 0.0f) {
		timer -= deltaTime;

		//ロード画面を挟む.
		if (timer <= 0.0f && player->pHP > 0) {
			bgm->Stop();
			sprites[0].Scale(glm::vec2(6.0f, 4.0f));
			sprites[1].Scale(glm::vec2(1.5f, 4.0f));
			sprites[1].Position(glm::vec3(1));
			
			sprites[2].Scale(glm::vec2(0));
			sprites[3].Scale(glm::vec2(0));
			sprites[16].Scale(glm::vec2(0));
			sprites[17].Scale(glm::vec2(0));
			sprites[18].Scale(glm::vec2(0));
			sprites[19].Scale(glm::vec2(0));
			sprites[20].Scale(glm::vec2(0));
			sprites[21].Scale(glm::vec2(0));
			sprites[22].Scale(glm::vec2(0));
			sprites[23].Scale(glm::vec2(0));
			sprites[24].Scale(glm::vec2(0));
			sprites[26].Scale(glm::vec2(0));
			sprites[27].Scale(glm::vec2(0));
			sprites[28].Scale(glm::vec2(0));
			sprites[31].Scale(glm::vec2(0));
			sprites[32].Scale(glm::vec2(0));
			sprites[29].Scale(glm::vec2(0));
			sprites[30].Scale(glm::vec2(0));
			sprites[33].Scale(glm::vec2(0));
			sprites[34].Scale(glm::vec2(0));
			sprites[35].Scale(glm::vec2(0));
			sprites[36].Scale(glm::vec2(0));
			sprites[37].Scale(glm::vec2(0));
			state = State::load;
			loadTimer = 4.0f;
		}
	}

	//ステージ移行の有無.
	if (allTimer > 0.0f) {
		allTimer -= deltaTime;

		if (allTimer <= 0.0f) {
			state = State::select;
			eventFrag = true;
			if (player->playerID != 0) {
				player->GetMesh()->Play("Idle");
			}
			sprites[2].Scale(glm::vec2(0.3f, 0.7f));
			sprites[3].Scale(glm::vec2(0.9f, 0.7f));
			sprites[22].Scale(glm::vec2(0.83f, 0.11f));
			sprites[23].Scale(glm::vec2(0.1f));
			sprites[24].Scale(glm::vec2(0.1f));
			sprites[2].Position(glm::vec3(0, -100, 0));
			sprites[3].Position(glm::vec3(0, -250, 0));
			sprites[24].Position(glm::vec3(82, -75, 0));
			sprites[22].Position(glm::vec3(0, -75, 0));
			sprites[23].Position(glm::vec3(-83, -75, 0));
		}
		allTimer = 0.0f;
	}

	//シーン切り替え待ち.
	if (overTimer > 0.0f) {
		overTimer -= deltaTime;

		//ゲームオーバー画面へ.
		if (overTimer <= 0.0f) {
			if (player->pHP <= 0 || defenceLine <= 0) {
				StageNo = 1;
				player->pLevel = 1;		//プレイヤーレベル.
				player->pAbility = 1;	//アビリティレベル.
				player->pExPoint = 100;	//経験値.
				player->pExCount = 500;	//レベルアップまで.
				player->pMP = 50;		//MP.
				player->pHP = 200;		//HP.
				player->playerID = 0;	//プレイヤーのメッシュ.
				nextStateFlag = false;

				SceneFader::Instance().FadeIn(1);
				Scene::Hide();
				SceneStack::Instance().Replace(std::make_shared<GameOverScene>());
				return;
			}
		}
	}

	//ステージクリアからリザルト画面.
	if (clearTimer > 0.0f) {
		clearTimer -= deltaTime;
		if (clearTimer <= 0.0f) {
			if (nextStateFlag == true && player->pHP > 0 && defenceLine > 0 && enemyStock == enemyBlow) {
				if (player->playerID != 0) {
					player->GetMesh()->Play("Idle");
				}
				sprites[11].Scale(glm::vec2(0));
				sprites[16].Scale(glm::vec2(0));
				sprites[17].Scale(glm::vec2(0));
				sprites[18].Scale(glm::vec2(0));
				sprites[19].Scale(glm::vec2(0));
				sprites[20].Scale(glm::vec2(0));
				sprites[21].Scale(glm::vec2(0));
				sprites[25].Scale(glm::vec2(0));
				sprites[26].Scale(glm::vec2(0));
				sprites[27].Scale(glm::vec2(0));
				sprites[28].Scale(glm::vec2(0));
				sprites[29].Scale(glm::vec2(0));
				sprites[30].Scale(glm::vec2(0));
				sprites[31].Scale(glm::vec2(0));
				sprites[32].Scale(glm::vec2(0));
				sprites[33].Scale(glm::vec2(0));
				sprites[34].Scale(glm::vec2(0));
				sprites[35].Scale(glm::vec2(0));
				sprites[36].Scale(glm::vec2(0));
				sprites[37].Scale(glm::vec2(0));


				state = State::result;
				sprites[1].Scale(glm::vec2(1, 3.5f));
				sprites[8].Scale(glm::vec2(1));
				sprites[9].Scale(glm::vec2(1));
			}
		}
	}
	

	//タイトルへロード画面中.
	if (loadTimer > 0.0f) {
		loadTimer -= deltaTime;

		if (state == State::result && loadTimer <= 0.0f && player->pHP > 0) {
			SceneStack::Instance().Replace(std::make_shared<TitleScene>());
			return;
		}
		else if (state == State::menu && loadTimer <= 0.0f && player->pHP > 0) {
			SceneStack::Instance().Replace(std::make_shared<TitleScene>());
			return;
		}
		else if (state == State::load && loadTimer <= 0.0f && 
			player->pHP > 0 && stageChage == 1) {
			StageNo += 1;
			SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
			return;
		}
		else if (state == State::load && loadTimer <= 0.0f &&
			player->pHP > 0 && stageChage == 2) {
			StageNo += 2;
			SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
			return;
		}
		else if (state == State::load && loadTimer <= 0.0f &&
			player->pHP > 0 && stageChage == 3) {
			StageNo += 3;
			SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
			return;
		}
		else if (state == State::load && loadTimer <= 0.0f &&
			player->pHP > 0 && stageChage == 4) {
			StageNo += 4;
			SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
			return;
		}
	}
	fontRenderer.EndUpdate();
	textWindow.Update(deltaTime);
}

/**
*シーンを描画する.
*/
void MainGameScene::Render() {

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const glm::vec2 screenSize(window.Width(), window.Height());
	
	glDisable(GL_CULL_FACE);

	lightBuffer.Upload();
	lightBuffer.Bind();

	//影用FBOに描画.
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboShadow->GetFramebuffer());
		auto tex = fboShadow->GetDepthTexture();
		glViewport(0, 0, tex->Width(), tex->Height());
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		
			//ディレクショナル・ライトの向きから影用のビュー行列を作成.
			//視点は、カメラの注視点からライト方向に100m移動した位置に設定する.
			glm::vec3 direction(0, -1, 0);
		for (auto e : lights) {
			if (auto p = std::dynamic_pointer_cast<DirectionalLightActor>(e)) {
				direction = p->direction;
				break;
			}
		}
		const glm::vec3 position = camera.target - direction * 100.0f;
		const glm::mat4 matView = glm::lookAt(position, camera.target, glm::vec3(0, 1, 0));
		
			//平行投影によるプロジェクション行列を作成.
		const float width = 100; // 描画範囲の幅.
		const float height = 100; // 描画範囲の高さ.
		const float near = 10.0f; // 描画範囲の手前側の境界.
		const float far = 200.0f; // 描画範囲の奥側の境界.
			const glm::mat4 matProj =
			glm::ortho<float>(-width / 2, width / 2, -height / 2, height / 2, near, far);
		
			//ビュー・プロジェクション行列を設定してメッシュを描画.
			meshBuffer.SetShadowViewProjectionMatrix(matProj * matView);
			RenderMesh(Mesh::DrawType::shadow);
		}

	// FBOに描画.
	glBindFramebuffer(GL_FRAMEBUFFER, fboMain->GetFramebuffer());
	const auto texMain = fboMain->GetColorTexture();
	glViewport(0, 0, texMain->Width(), texMain->Height());
	glClearColor(0.5f, 0.6f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	const glm::mat4 matView = glm::lookAt(camera.position, camera.target, camera.up);
	const float aspectRatio =
		static_cast<float>(window.Width()) / static_cast<float>(window.Height());
	const glm::mat4 matProj =
		glm::perspective(camera.fov * 0.5f, aspectRatio, camera.near, camera.far);
	meshBuffer.SetViewProjectionMatrix(matProj * matView);
	meshBuffer.SetCameraPosition(camera.position);
	meshBuffer.SetTime(window.Time());
	meshBuffer.BindShadowTexture(fboShadow->GetDepthTexture());

	RenderMesh(Mesh::DrawType::color);
	particleSystem.Draw(matProj, matView);

	meshBuffer.UnbindShadowTexture();
	
	//被写界深度エフェクト.
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fboDepthOfField->GetFramebuffer());
		const auto tex = fboDepthOfField->GetColorTexture();
		glViewport(0, 0, tex->Width(), tex->Height());

		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);

		camera.Update(matView);

		Mesh::FilePtr mesh = meshBuffer.GetFile("RenderTarget");
		Shader::ProgramPtr prog = mesh->materials[0].program;
		prog->Use();
		prog->SetViewInfo(static_cast<float>(window.Width()),
			static_cast<float>(window.Height()), camera.near, camera.far);
		prog->SetCameraInfo(camera.focalPlane, camera.focalLength,
			camera.aperture, camera.sensorSize);
		Mesh::Draw(mesh, glm::mat4(1));
	}

	//ブルーム・エフェクト.
	{
		//明るい部分を取り出す.
			{
			auto tex = fboBloom[0][0]->GetColorTexture();
			glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[0][0]->GetFramebuffer());
			glViewport(0, 0, tex->Width(), tex->Height());
			glClear(GL_COLOR_BUFFER_BIT);
			Mesh::FilePtr mesh = meshBuffer.GetFile("BrightPassFilter");
			mesh->materials[0].texture[0] = fboDepthOfField->GetColorTexture();
			Mesh::Draw(mesh, glm::mat4(1));
			}

			//縮小コピー.
			Mesh::FilePtr simpleMesh = meshBuffer.GetFile("Simple");
			for (int i = 0; i < sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; ++i) {
				auto tex = fboBloom[i + 1][0]->GetColorTexture();
				glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i + 1][0]->GetFramebuffer());
				glViewport(0, 0, tex->Width(), tex->Height());
				glClear(GL_COLOR_BUFFER_BIT);
				simpleMesh->materials[0].texture[0] = fboBloom[i][0]->GetColorTexture();
				Mesh::Draw(simpleMesh, glm::mat4(1));
			}

			//ガウスぼかし.
			Mesh::FilePtr blurMesh = meshBuffer.GetFile("NormalBlur");
			Shader::ProgramPtr progBlur = blurMesh->materials[0].program;
			for (int i = sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; i >= 0; --i) {
				auto tex = fboBloom[i][0]->GetColorTexture();
				glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i][1]->GetFramebuffer());
				glViewport(0, 0, tex->Width(), tex->Height());
				glClear(GL_COLOR_BUFFER_BIT);
				progBlur->Use();
				progBlur->SetBlurDirection(1.0f / static_cast<float>(tex->Width()), 0.0f);
				blurMesh->materials[0].texture[0] = fboBloom[i][0]->GetColorTexture();
				Mesh::Draw(blurMesh, glm::mat4(1));
				
				glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i][0]->GetFramebuffer());
				glClear(GL_COLOR_BUFFER_BIT);
				progBlur->Use();
				progBlur->SetBlurDirection(0.0f, 1.0f / static_cast<float>(tex->Height()));
				blurMesh->materials[0].texture[0] = fboBloom[i][1]->GetColorTexture();
				Mesh::Draw(blurMesh, glm::mat4(1));
			}

			//拡大＆加算合成.
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			for (int i = sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; i > 0; --i) {
				auto tex = fboBloom[i - 1][0]->GetColorTexture();
				glBindFramebuffer(GL_FRAMEBUFFER, fboBloom[i - 1][0]->GetFramebuffer());
				glViewport(0, 0, tex->Width(), tex->Height());
				simpleMesh->materials[0].texture[0] = fboBloom[i][0]->GetColorTexture();
				Mesh::Draw(simpleMesh, glm::mat4(1));
			}
		}
	//全てをデフォルト・フレームバッファに合成描画.
		{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window.Width(), window.Height());
		
		const glm::vec2 screenSize(window.Width(), window.Height());
		spriteRenderer.Draw(screenSize);
		
			//被写界深度エフェクト適用後の画像を描画.
		glDisable(GL_BLEND);
		Mesh::FilePtr simpleMesh = meshBuffer.GetFile("Simple");
		simpleMesh->materials[0].texture[0] = fboDepthOfField->GetColorTexture();
		Mesh::Draw(simpleMesh, glm::mat4(1));
		
			//拡散光を描画.
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		simpleMesh->materials[0].texture[0] = fboBloom[0][0]->GetColorTexture();
		Mesh::Draw(simpleMesh, glm::mat4(1));

		fontRenderer.BeginUpdate();
		//メニュー画面の情報.
		if (state == State::menu) {

			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));

			//プレイヤー名.
			if (player->playerID == 0) {
				fontRenderer.AddString(glm::vec2(90, 200), L"ソウル");
				fontRenderer.AddString(glm::vec2(-130, -90), L"本作の主人公。");
				fontRenderer.AddString(glm::vec2(-130, -130), L"敵軍に実体を奪われ、実体のない魂の存在。");
				fontRenderer.AddString(glm::vec2(-130, -170), L"彼は再び体を取り戻すべく奮闘する。");
				fontRenderer.AddString(glm::vec2(-130, -210), L"しかし移動とジャンプしかできない。");
			}
			else if (player->playerID == 1) {
				fontRenderer.AddString(glm::vec2(90, 200), L"ゴブリン");
				fontRenderer.AddString(glm::vec2(-130, -90), L"ゴブリン族の一人。");
				fontRenderer.AddString(glm::vec2(-130, -130), L"自分の存在をゴブリンと認めたくないと");
				fontRenderer.AddString(glm::vec2(-130, -170), L"同族のゴブリンを絶滅させることを誓う。");
				fontRenderer.AddString(glm::vec2(-130, -210), L"彼の夢は人間の女と結婚することである。");
			}
			else if (player->playerID == 2) {
				fontRenderer.AddString(glm::vec2(90, 200), L"ウィザード");
				fontRenderer.AddString(glm::vec2(-130, -90), L"人間の魔道師。");
				fontRenderer.AddString(glm::vec2(-130, -130), L"魔法の名家で魔道師として育てられた。");
				fontRenderer.AddString(glm::vec2(-130, -170), L"しかし彼は落ちこぼれを絵にかいたような");
				fontRenderer.AddString(glm::vec2(-130, -210), L"人間だった。微力な攻撃魔法と");
				fontRenderer.AddString(glm::vec2(-130, -250), L"主に杖を使った近接戦を得意とする。");
			}
			else if (player->playerID == 3) {
				fontRenderer.AddString(glm::vec2(90, 200), L"スケルトン");
				fontRenderer.AddString(glm::vec2(-130, -90), L"黄泉の国から復活した骸骨騎士。");
				fontRenderer.AddString(glm::vec2(-130, -130), L"世界一とも名高い元エリート騎士。");
				fontRenderer.AddString(glm::vec2(-130, -170), L"それと同じく無類の酒と女好きの");
				fontRenderer.AddString(glm::vec2(-130, -210), L"遊び人だった。彼の死因は泥酔し、王の");
				fontRenderer.AddString(glm::vec2(-130, -250), L"娘へのセクハラ行為により死刑。");
			}

			//ステージNo表示.
			{
				wchar_t str[] = L"STAGE.  ";
				int n = StageNo;
				for (int i = 0; i < 2; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				fontRenderer.AddString(glm::vec2(280, 200), str);
			}
			//レベル表示.
			{
				wchar_t str[] = L"Lv.   ";
				int n = player->pLevel;
				for (int i = 0; i < 3; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				fontRenderer.AddString(glm::vec2(100, 150), str);
			}
			//アビリティレベル.
			{
				wchar_t str[] = L"Ability:  ";
				int n = player->pAbility;
				for (int i = 0; i < 2; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				fontRenderer.AddString(glm::vec2(280, 150), str);
			}
			//体力表示.
			{
				wchar_t str[] = L"    ";
				int n = player->pHP;
				for (int i = 0; i < 4; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				fontRenderer.AddString(glm::vec2(100, 100), L"HP:");
				fontRenderer.AddString(glm::vec2(150, 75), str);
			}

			//最大体力表示.
			{
				wchar_t str[] = L"/    ";
				int n = player->maxHP;
				for (int i = 0; i < 4; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				if (player->maxHP < 1000) {
					fontRenderer.AddString(glm::vec2(200, 75), str);
				}
				else if (player->maxHP >= 1000) {
					fontRenderer.AddString(glm::vec2(220, 75), str);
				}
			}

			//ＭＰ表示.
			{
				wchar_t str[] = L"   ";
				int n = player->pMP;
				for (int i = 0; i < 3; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				fontRenderer.AddString(glm::vec2(100, 50), L"MP:");
				fontRenderer.AddString(glm::vec2(150, 25), str);
			}
			//最大ＭＰ表示.
			{
				wchar_t str[] = L"/   ";
				int n = player->maxMP;
				for (int i = 0; i < 3; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				if (player->maxMP < 100) {
					fontRenderer.AddString(glm::vec2(180, 25), str);
				}
				else if (player->maxMP >= 100) {
					fontRenderer.AddString(glm::vec2(200, 25), str);
				}
			}
			//レベルアップまであと.
			{
				wchar_t str[] = L"EX:LvUPまであと    ";
				int n = player->pExCount;
				for (int i = 0; i < 4; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				fontRenderer.AddString(glm::vec2(99, -10), str);
			}
		}

		//コマンド表.
		if (state == State::cMenu) {
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));

			if (gamePadText) {
				fontRenderer.AddString(glm::vec2(0, 210), L"キーボードコマンド表");
				fontRenderer.AddString(glm::vec2(-100, 150), L"WSAD:移動");
				fontRenderer.AddString(glm::vec2(-100, 100), L"L：ジャンプ");
				fontRenderer.AddString(glm::vec2(-100, 50), L"U：スキル１");
				fontRenderer.AddString(glm::vec2(-100, 0), L"H：スキル２");
				fontRenderer.AddString(glm::vec2(-100, -50), L"K：スキル３");
				fontRenderer.AddString(glm::vec2(-100, -100), L"J：スキル４");
				fontRenderer.AddString(glm::vec2(170, 150), L"Y：カメラズーム");
				fontRenderer.AddString(glm::vec2(170, 100), L"I：カメラアプローチ");
				fontRenderer.AddString(glm::vec2(170, 50), L"SPACE:メニュー画面");
				fontRenderer.AddString(glm::vec2(170, 0), L"ENTER:決定キー");

				fontRenderer.AddString(glm::vec2(-100, -200), L"Yボタン　または　Uボタンで");
				fontRenderer.AddString(glm::vec2(-100, -250), L"ゲームパッド操作説明に切り替えます");
			}
			else {
				fontRenderer.AddString(glm::vec2(0, 210), L"ゲームパッドコマンド表");
				fontRenderer.AddString(glm::vec2(-100, 150), L"十字キー:移動");
				fontRenderer.AddString(glm::vec2(-100, 100), L"RT：ジャンプ");
				fontRenderer.AddString(glm::vec2(-100, 50), L"Y：スキル１");
				fontRenderer.AddString(glm::vec2(-100, 0), L"X：スキル２");
				fontRenderer.AddString(glm::vec2(-100, -50), L"B：スキル３");
				fontRenderer.AddString(glm::vec2(-100, -100), L"A：スキル４");
				fontRenderer.AddString(glm::vec2(170, 150), L"LB：カメラズーム");
				fontRenderer.AddString(glm::vec2(170, 100), L"RB：カメラアプローチ");
				fontRenderer.AddString(glm::vec2(170, 50), L"BACK:メニュー画面");
				fontRenderer.AddString(glm::vec2(170, 0), L"START:決定キー");

				fontRenderer.AddString(glm::vec2(-100, -200), L"Yボタン　または　Uボタンで");
				fontRenderer.AddString(glm::vec2(-100, -250), L"キーボード操作説明に切り替えます");
			}
			

			fontRenderer.AddString(glm::vec2(-500, -200), L"対応コマンドの");
			fontRenderer.AddString(glm::vec2(-500, -250), L"確認ができます");
		}
		//スキル表.
		else if (state == State::sMenu) {
			fontRenderer.AddString(glm::vec2(-510, -190), L"スキルの確認が");
			fontRenderer.AddString(glm::vec2(-510, -230), L"できます。");
			/*fontRenderer.AddString(glm::vec2(-510, -190), L"スキルの確認や");
			fontRenderer.AddString(glm::vec2(-510, -230), L"スキルパネルの");
			fontRenderer.AddString(glm::vec2(-510, -270), L"カスタムができます");
			fontRenderer.AddString(glm::vec2(-110, 200), L"スキル表");*/
			//プレイヤー名.
			if (player->playerID == 0) {
				fontRenderer.AddString(glm::vec2(-110, 200), L"ソウル");
				fontRenderer.AddString(glm::vec2(-125, 150), L"何もできない");
				fontRenderer.AddString(glm::vec2(-125, 100), L"何もできない");
				fontRenderer.AddString(glm::vec2(-125, 50), L"何もできない");
				fontRenderer.AddString(glm::vec2(-125, 0), L"何もできない");
			}
			if (player->playerID == 1) {
				fontRenderer.AddString(glm::vec2(-110, 200), L"ゴブリン");
				int n = player->pAbility;
				
				fontRenderer.AddString(glm::vec2(-105, 150), L"通常攻撃");
				//使えないスキルは半透明.
				fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
				fontRenderer.AddString(glm::vec2(-105, 100), L"早歩き");
				fontRenderer.AddString(glm::vec2(-105, 50), L"ダッシュ");
				fontRenderer.AddString(glm::vec2(-105, 0), L"高速移動");
				
				if (n >= 4) {
					fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
					fontRenderer.AddString(glm::vec2(-105, 0), L"高速移動");
				}
				if (n >= 3) {
					fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
					fontRenderer.AddString(glm::vec2(-105, 50), L"ダッシュ");
				}
				if (n >= 2) {
					fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
					fontRenderer.AddString(glm::vec2(-105, 100), L"早歩き");
				}
			}
			if (player->playerID == 2) {
				fontRenderer.AddString(glm::vec2(-110, 200), L"ウィザード");
				int n = player->pAbility;

				fontRenderer.AddString(glm::vec2(-105, 150), L"通常攻撃");
				//使えないスキルは半透明.
				fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
				fontRenderer.AddString(glm::vec2(-105, 100), L"かぶと割り");
				fontRenderer.AddString(glm::vec2(-105, 50), L"ファイア");
				fontRenderer.AddString(glm::vec2(-105, 0), L"メテオ");

				if (n >= 4) {
					fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
					fontRenderer.AddString(glm::vec2(-105, 0), L"メテオ");
				}
				if (n >= 3) {
					fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
					fontRenderer.AddString(glm::vec2(-105, 50), L"ファイア");
				}
				if (n >= 2) {
					fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
					fontRenderer.AddString(glm::vec2(-105, 100), L"かぶと割り");
				}
			}
			if (player->playerID == 3) {
				fontRenderer.AddString(glm::vec2(-110, 200), L"スケルトン");
				int n = player->pAbility;

				fontRenderer.AddString(glm::vec2(-105, 150), L"通常攻撃");
				//使えないスキルは半透明.
				fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
				fontRenderer.AddString(glm::vec2(-105, 100), L"水平斬り");
				fontRenderer.AddString(glm::vec2(-105, 50), L"かぶと割り");
				fontRenderer.AddString(glm::vec2(-105, 0), L"燕返し");

				if (n >= 4) {
					fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
					fontRenderer.AddString(glm::vec2(-105, 0), L"燕返し");
				}
				if (n >= 3) {
					fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
					fontRenderer.AddString(glm::vec2(-105, 50), L"かぶと割り");
				}
				if (n >= 2) {
					fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
					fontRenderer.AddString(glm::vec2(-105, 100), L"水平斬り");
				}
			}
		}

		//メニュー画面常時.
		if (state == State::menu || state == State::cMenu || state == State::sMenu) {
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-460, 200), L"プレイヤー");
			fontRenderer.AddString(glm::vec2(-450, 150), L"コマンド");
			fontRenderer.AddString(glm::vec2(-440, 100), L"スキル");
			fontRenderer.AddString(glm::vec2(-460, 0), L"ゲーム終了");
			fontRenderer.AddString(glm::vec2(-440, -50), L"閉じる");
		}

		//プレイ画面の情報.
		if (state == State::play || state == State::select) {

			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 0.1f, 5));
			fontRenderer.Scale(glm::vec2(1));

			//体力表示.
			{
				wchar_t str[] = L"    ";
				int n = player->pHP;
				for (int i = 0; i < 4; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				fontRenderer.AddString(glm::vec2(-620, 300), L"HP:");
				fontRenderer.AddString(glm::vec2(-570, 275), str);
			}

			//最大体力表示.
			{
				wchar_t str[] = L"/    ";
				int n = player->maxHP;
				for (int i = 0; i < 4; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				if (player->maxHP < 1000) {
					fontRenderer.AddString(glm::vec2(-520, 275), str);
				}
				else if (player->maxHP >= 1000) {
					fontRenderer.AddString(glm::vec2(-500, 275), str);
				}
			}

			//ＭＰ表示.
			{
				wchar_t str[] = L"   ";
				int n = player->pMP;
				for (int i = 0; i < 3; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				fontRenderer.AddString(glm::vec2(-620, 250), L"MP:");
				fontRenderer.AddString(glm::vec2(-570, 225), str);
			}
			//最大ＭＰ表示.
			{
				wchar_t str[] = L"/   ";
				int n = player->maxMP;
				for (int i = 0; i < 3; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				if (player->maxMP < 100) {
					fontRenderer.AddString(glm::vec2(-540, 225), str);
				}
				else if (player->maxMP >= 100) {
					fontRenderer.AddString(glm::vec2(-520, 225), str);
				}
			}

			//レベル表示.
			{
				wchar_t str[] = L"Lv.   ";
				int n = player->pLevel;
				for (int i = 0; i < 3; ++i) {
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0) {
						break;
					}
				}
				fontRenderer.AddString(glm::vec2(-620, 350), str);
			}
			////ステージ名の表示.
			//{
			//	wchar_t str[] = L"STAGE.  ";
			//	int n = StageNo;
			//	for (int i = 0; i < 2; ++i) {
			//		str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
			//		n /= 10;
			//		if (n <= 0) {
			//			break;
			//		}
			//	}
			//	fontRenderer.AddString(glm::vec2(-620, 350), str);
			//}
			if (StageNo != 1) {
				//敵を倒した数..
				{
					wchar_t str[] = L"   ";
					int n = enemyBlow;
					for (int i = 0; i < 3; ++i) {
						str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
						n /= 10;
						if (n <= 0) {
							break;
						}
					}
					if (enemyBlow >= 100) {
						fontRenderer.AddString(glm::vec2(350, 340), str);
					}
					else if (enemyBlow >= 10) {
						fontRenderer.AddString(glm::vec2(360, 340), str);
					}
					else {
						fontRenderer.AddString(glm::vec2(370, 340), str);
					}
				}
				//敵の残数..
				{
					wchar_t str[] = L"/   ";
					int n = enemyStock;
					for (int i = 0; i < 3; ++i) {
						str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
						n /= 10;
						if (n <= 0) {
							break;
						}
					}
					fontRenderer.AddString(glm::vec2(400, 340), str);
				}
			}
			/*fontRenderer.AddString(glm::vec2(450, 350), L"試練の祭壇");*/
		}

		if (state == State::load) {
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 10));
			fontRenderer.AddString(glm::vec2(280, -280), L"NowLoading...");
			//ロード画面中にランダムでゲームに役立つヒントが表示される.
			int loadHint = stageChage;
			if (loadHint == 1) {
				sprites[4].Scale(glm::vec2(0.4f));
				fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 10));
				fontRenderer.AddString(glm::vec2(-300, -100), L"火がついているのは一度クリアしたステージ。");
				fontRenderer.AddString(glm::vec2(-300, -150), L"だが、何回でも挑戦可能。");
			}
			else if (loadHint == 2) {
				sprites[5].Scale(glm::vec2(0.4f));
				fontRenderer.AddString(glm::vec2(-300, -100), L"主人公の肉体の情報が保管されたトーテム。");
				fontRenderer.AddString(glm::vec2(-300, -150), L"ステージごとにランダムで配置される。");
				fontRenderer.AddString(glm::vec2(-300, -200), L"ステージが始まれば真っ先に場所を把握しよう。");
			}
			else if (loadHint == 3) {
				sprites[6].Scale(glm::vec2(0.4f));
				fontRenderer.AddString(glm::vec2(-300, -100), L"個性豊かなキャラクターたちを使用できる。");
				fontRenderer.AddString(glm::vec2(-300, -150), L"スキルやプロフィールはメニュー画面で確認可能。");
			}
			else if (loadHint == 4) {
				sprites[7].Scale(glm::vec2(0.4f));
				fontRenderer.AddString(glm::vec2(-300, -100), L"敵は大きさにより性質が異なる。");
				fontRenderer.AddString(glm::vec2(-300, -150), L"攻撃力が高いもの、移動が速いものなど。");
				fontRenderer.AddString(glm::vec2(-300, -200), L"倒す順番には優先順位をつけよう。");
			}
		}

		//ステージ移行の有無を選択時.
		if (state == State::select) {
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-30, -85), L"はい");
			fontRenderer.AddString(glm::vec2(-45, -125), L"いいえ");
			fontRenderer.AddString(glm::vec2(-200, -280), L"挑みますか？");

			//if (eventFrag == true) {
				if (stageChage == 1) {
					fontRenderer.AddString(glm::vec2(-270, -240), L"天の声：Aの試練が待っている.");
					fontRenderer.AddString(glm::vec2(20, -280), L"難易度★☆☆☆☆");
					eventFrag = false;
				}
				else if (stageChage == 2) {
					fontRenderer.AddString(glm::vec2(-270, -240), L"天の声：Bの試練が待っている.");
					fontRenderer.AddString(glm::vec2(20, -280), L"難易度★★☆☆☆");
					eventFrag = false;
				}
				else if (stageChage == 3) {
					fontRenderer.AddString(glm::vec2(-270, -240), L"天の声：Cの試練が待っている.");
					fontRenderer.AddString(glm::vec2(20, -280), L"難易度★★★☆☆");
					eventFrag = false;
				}
				else if (stageChage == 4) {
					fontRenderer.AddString(glm::vec2(-270, -240), L"天の声：Dの試練が待っている.");
					fontRenderer.AddString(glm::vec2(20, -280), L"難易度★★★★★");
					eventFrag = false;
				}
		//	}
		}


		fontRenderer.EndUpdate();
		textWindow.Draw();
		spriteRenderer.Draw(screenSize);
		fontRenderer.Draw(screenSize);
	}
	#if 0
	//デバッグのために、影用の深度テクスチャを表示する.
	{
		glDisable(GL_BLEND);
		Mesh::FilePtr simpleMesh = meshBuffer.GetFile("Simple");
		simpleMesh->materials[0].texture[0] = fboShadow->GetDepthTexture();
		glm::mat4 m = glm::scale(glm::translate(
		glm::mat4(1), glm::vec3(-0.45f, 0, 0)), glm::vec3(0.5f, 0.89f, 1));
		Mesh::Draw(simpleMesh, m);
	}
	#endif
}

/**
* お地蔵様に触れたときの処理.
*
* @param id  お地蔵様の番号.
* @param pos お地蔵様の座標.
*
* @retval true  処理成功.
* @retval false すでに戦闘中なので処理しなかった.
*/
bool MainGameScene::HandleJizoEffects(int id, const glm::vec3& pos)
 {
	if (jizoId >= 0) {
		return false;
		
	}
	jizoId = id;
	const size_t oniCount = 3;
	for (size_t i = 0; i < oniCount; i++)
		 {
		glm::vec3 position(pos);
		position.x += std::uniform_real_distribution<float>(-15, 15)(rand);
		position.z += std::uniform_real_distribution<float>(-15, 15)(rand);
		position.y = heightMap.Height(position);
		
			glm::vec3 rotation(0);
		rotation.y = std::uniform_real_distribution<float>(0, 3.14f * 2.0f)(rand);
		const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
		if (position.y >= 4.0f) {
			SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
				mesh, "Kooni", 13, position, rotation);
			p->colLocal = Collision::CreateCapsule(
				glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
			enemies[0].Add(p);
			mHouseFlag = true;
		}
		else {
			--i;
		}
	}
	return true;
}

/**
* ワープに触れたときの処理.
*
* @param id  ワープの番号.
* @param pos ワープの座標.
*
* @retval true  処理成功.
* @retval false すでに戦闘中なので処理しなかった.
*/
bool MainGameScene::HandleWarpEffects(int id, const glm::vec3& pos)
{
	if (warpID >= 0) {
		return false;
		
	}
	warpID = id;
	glm::vec3 position(pos);
	position.x += std::uniform_real_distribution<float>(0, 20)(rand);
	position.z += std::uniform_real_distribution<float>(0, 20)(rand);
	position.y = heightMap.Height(position) + 2;
	player->position = position;

	return true;
}

/**
* コインに触れたときの処理.
*
* @param id  コインの番号.
* @param pos コインの座標.
*
* @retval true  処理成功.
* @retval false すでに戦闘中なので処理しなかった.
*/
bool MainGameScene::HandleCoinEffects(int id, const glm::vec3& pos)
{
	
	if (CoinID >= 0) {
		return false;

	}
		CoinID = id;
		/*glm::vec3 position(pos);
		position.x += std::uniform_real_distribution<float>(-10, 10)(rand);
		position.z += std::uniform_real_distribution<float>(-10, 10)(rand);
		position.y = heightMap.Height(position) + 2;

		glm::vec3 rotation(0);
		rotation.y = std::uniform_real_distribution<float>(0, 3.14f * 2.0f)(rand);
		const Mesh::FilePtr meshCoin = meshBuffer.GetFile("Res/Coin.gltf");
			StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
				meshCoin, "Coin", 1, position, rotation);
			p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
				glm::vec3(0.4f, 0, 0), glm::vec3(0, -0.4f, 0), glm::vec3(0, 0, 0.3f), glm::vec3(0.8f, 0.8f, 0.6f));
			objects.Add(p);*/

			gameClearFlag = true;
			Audio::Engine::Instance().Prepare("Res/Audio/GameClear.mp3")->Play();

	return true;
}

/**
* カメラのパラメータを更新する.
*
* @param matView 更新に使用するビュー行列.
*/
	void MainGameScene::Camera::Update(const glm::mat4& matView)
	 {
	const glm::vec4 pos = matView * glm::vec4(target, 1);
	focalPlane = pos.z * -1000.0f;
	
	const float imageDistance = sensorSize * 0.5f / glm::tan(fov * 0.5f);
	focalLength = 1.0f / ((1.0f / focalPlane) + (1.0f / imageDistance));
	
		aperture = focalLength / fNumber;
	}

	/**
	*メッシュを描画する.
	*
	*@param drawType	描画するデータの種類.
	*/
	void MainGameScene::RenderMesh(Mesh::DrawType drawType)
		{
			Mesh::Draw(meshBuffer.GetFile("Terrain"), glm::mat4(1));

			player->Draw(drawType);
			enemies[0].Draw(drawType);
			enemies[1].Draw(drawType);
			enemies[2].Draw(drawType);
			enemies[3].Draw(drawType);
			trees.Draw(drawType);
			objects.Draw(drawType);
			defencePoint.Draw(drawType);
			walls.Draw(drawType);
			objectives.Draw(drawType);
			effects.Draw(drawType);
			items[0].Draw(drawType);
			items[1].Draw(drawType);
			items[2].Draw(drawType);
			items[3].Draw(drawType);
			warp[0].Draw(drawType);
			warp[1].Draw(drawType);
			warp[2].Draw(drawType);
			warp[3].Draw(drawType);
			bullet[0].Draw(drawType);
			upItem[0].Draw(drawType);
			upItem[1].Draw(drawType);
			upItem[2].Draw(drawType);

			/*if (drawType == Mesh::DrawType::color) {
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				Mesh::Draw(meshBuffer.GetFile("Water"), glm::mat4(1),drawType);
			}*/
	}