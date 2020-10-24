/**
*@file MainGameScene.cpp
*/
#include "GLFWEW.h"
#include "MainGameScene.h"
#include "EventScene.h"
#include "GameOverScene.h"
#include "ObjectiveActor.h"
#include "TitleScene.h"
#include "Mesh.h"
#include "SkeletalMeshActor.h"
#include "PlayerActor.h"
#include "EnemyActor.h"
#include "Scene.h"
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
int PlayerActor::maxHP = 200;				//最大HP.
int PlayerActor::maxMP = 50;				//最大MP.
int PlayerActor::playerID = 0;		//プレイヤーのメッシュ.
bool MainGameScene::StClearedE = false;
bool MainGameScene::StClearedS = false;
bool MainGameScene::StClearedW = false;
bool MainGameScene::StClearedN = false;

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
	if (dot(v, v) > FLT_EPSILON)
	{
		// aをbに重ならない位置まで移動.
		const glm::vec3 vn = normalize(v);
		float radiusSum = a->colWorld.s.r;
		switch (b->colWorld.type)
		{
		case Collision::Shape::Type::sphere: radiusSum += b->colWorld.s.r; break;
		case Collision::Shape::Type::capsule: radiusSum += b->colWorld.c.r; break;

		}
		const float distance = radiusSum - glm::length(v) + 0.01f;
		a->position += vn * distance;
		a->colWorld.s.center += vn * distance;

		if (a->velocity.y < 0 && vn.y >= glm::cos(glm::radians(60.0f)))
		{
			a->velocity.y = 0;
		}
	}
	else
	{
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

*/void MainGameScene::selectUI(int a, int b, int c, int d, int e, int f)
{
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();

	if (window.GetGamePad().buttonDown & GamePad::DPAD_UP)
	{
		Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
		if (selectCount >= a)
		{
			--selectCount;
			if (selectCount == b)
			{
				selectCount = c;
			}
		}
	}
	//下にずれる.
	else if (window.GetGamePad().buttonDown & GamePad::DPAD_DOWN)
	{
		if (selectCount <= d)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			++selectCount;
			if (selectCount == e)
			{
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
*/
void MainGameScene::skSelectUI(int a, int b, int c, int d, int e, int f)
{
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();

	if (window.GetGamePad().buttonDown & GamePad::DPAD_UP)
	{
		Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
		if (skComCount >= a)
		{
			--skComCount;
			if (skComCount == b)
			{
				skComCount = c;
			}
		}
	}
	//下にずれる.
	else if (window.GetGamePad().buttonDown & GamePad::DPAD_DOWN)
	{
		if (skComCount <= d)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
			++skComCount;
			if (skComCount == e)
			{
				skComCount = f;
			}
		}
	}
}

/**
*敵のターゲット情報を決める.
*
*@param a		ActorListの種類.
*/
void MainGameScene::EnemyTargetID(ActorList& a)
{
	int id = std::uniform_int_distribution<>(0, 2)(rand);
	//ターゲットの方向を調べる.
	for (auto i = a.begin(); i != a.end(); ++i)
	{
		const int index = i - a.begin() - id;
		if (index == 0)
		{
			enemyID = 0;
		}
		if (index == 1)
		{
			enemyID = 1;
		}
		if (index == 2)
		{
			enemyID = 2;
		}
	}
}

/**
*敵の出現.
*/
void MainGameScene::EnemySpawn(ActorList enemy[], glm::vec3 scale, int target, int n)
{
	const size_t enemyCount = 3;

	for (size_t i = 0; i < enemyCount; i++)
	{
		position.x = std::uniform_real_distribution<float>(60, 100)(rand);
		position.z = std::uniform_real_distribution<float>(80, 120)(rand);
		glm::vec3 rotation(0);
		rotation.y = std::uniform_real_distribution<float>(0, 3.14f * 2.0f)(rand);
		position.y = heightMap.Height(position);

		const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
		SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
			mesh, "Kooni", 15, position, rotation);
		p->colLocal = Collision::CreateCapsule(
			glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
		p->scale = scale;
		enemy[n].Add(p);
		target = std::uniform_int_distribution<>(0, 2)(rand);
		EnemyTargetID(enemy[n]);
	}
}

/**
*敵の行動、攻撃、死亡.
*
*@param deltaTime	前回からの更新からの経過時間（秒）.
*@param x			敵のActor.
*@param a			敵のNo.
*@param b			敵のID.
*/
void MainGameScene::EnemyAI(float deltaTime, ActorList& x, int a, int b)
{
	//敵が追いかけてくる.
	for (auto& e : enemies[a])
	{
		SkeletalMeshActorPtr enemy = std::static_pointer_cast<SkeletalMeshActor>(e);
		Mesh::SkeletalMeshPtr mesh = enemy->GetMesh();

		//死亡したら消す.
		if (mesh->GetAnimation() == "Down")
		{
			if (mesh->IsFinished())
			{
				enemy->health = 0;
				enemyBlow += 1;
				player->pExPoint -= 20;
				player->pExCount -= 50;
				enemySpawn += 1;
				combo += 1;
				comboTimer = 0.0f;
				comTimerFlag = true;
			}
			continue;
		}

		const float moveSpeed = baseSpeed * 3.0f - a + 1;
		const float rotationSpeed = baseSpeed * glm::radians(60.0f);
		const float frontRange = glm::radians(15.0f);

		const glm::vec3 v = (*(x.begin() + b))->position - e->position;
		const glm::vec3 vTarget = glm::normalize(v);
		float radian = std::atan2(-vTarget.z, vTarget.x) - glm::radians(90.0f);

		if (radian <= 0)
		{
			radian += glm::radians(360.0f);
		}

		const glm::vec3 vEnemyFront = glm::rotate(
			glm::mat4(1), e->rotation.y + glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);

		//ターゲットが正面にいなかったら、正面にとらえるような左右に旋回.
		if (std::abs(radian - e->rotation.y) > frontRange)
		{
			const glm::vec3 vRotDir = glm::cross(vEnemyFront, vTarget);
			if (vRotDir.y >= 0)
			{
				e->rotation.y += rotationSpeed * deltaTime;
				if (e->rotation.y >= glm::radians(360.0f))
				{
					e->rotation.y -= glm::radians(360.0f);
				}
			}
			else
			{
				e->rotation.y -= rotationSpeed * deltaTime;
				if (e->rotation.y < 0)
				{
					e->rotation.y += glm::radians(360.0f);
				}
			}
		}
		//十分に接近していなければ移動する。接近していれば攻撃する.
		if (glm::length(v) > 3.0f)
		{
			e->velocity = vEnemyFront * moveSpeed;
			if (mesh->GetAnimation() != "Run")
			{
				mesh->Play("Run");
			}
		}
		else if (e->health <= 0)
		{
			e->velocity = glm::vec3(0);	//死んでいるのでもう移動しない.
		}
		else
		{
			e->velocity = glm::vec3(0);
			if (mesh->GetAnimation() != "Wait")
			{
				if ((mesh->GetAnimation() != "Attack" && mesh->GetAnimation() != "Hit") ||
					mesh->IsFinished())
				{
					mesh->Play("Wait");
				}
			}
		}
		//定期的に攻撃状態になる.
		if (isAttacking)
		{
			isAttacking = false;
			attackingTimer = 3.0f;	//次の攻撃は５秒後.
			mesh->Play("Attack", false);
		}
		else
		{
			attackingTimer -= deltaTime;
			if (attackingTimer <= 0)
			{
				isAttacking = true;
				defenceFlag = true;

				defenceLine -= 0.4f / comboBuf * a;
			}
		}
	}
}

/**
*敵の当たり判定.
*
*@param i		敵のID.
*/
void MainGameScene::EnemyDetectCollision(int i)
{
	//炎攻撃.
	DetectCollision(bullet[0], enemies[i],
		[this](const ActorPtr& a, const ActorPtr&b, const glm::vec3& p)
	{
		Audio::Engine::Instance().Prepare("Res/Audio/Enemy.mp3")->Play();
		Audio::Engine::Instance().Prepare("Res/Audio/magic-flame2.mp3")->Play();
		b->health = 0;
		enemyBlow += 1;
		player->pExPoint -= 20;
		player->pExCount -= 100;
		enemySpawn += 1;
		combo += 1;
		comboTimer = 0.0f;
		comTimerFlag = true;

		auto mesh = meshBuffer.GetSkeletalMesh("Effect.Hit");
		mesh->Play(mesh->GetAnimationList()[0].name, false);
		mesh->SetAnimationSpeed(std::uniform_real_distribution<float>(0.75f, 1.0f)(rand) / 1.5f);
		glm::vec3 rot = player->rotation;
		rot.x += std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
		rot.y += std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
		rot.z += std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
		glm::vec3 scale(std::uniform_real_distribution<float>(1.0f, 1.5f)(rand) * 1.5f);
		auto effect = std::make_shared<SkeletalMeshActor>(mesh, "Effect.Hit", 1, p, rot, scale * b->scale);
		effects.Add(effect);
	});

	//氷攻撃.
	DetectCollision(bullet[1], enemies[i],
		[this](const ActorPtr& a, const ActorPtr&b, const glm::vec3& p)
	{
		Audio::Engine::Instance().Prepare("Res/Audio/Enemy.mp3")->Play();
		Audio::Engine::Instance().Prepare("Res/Audio/magic-ice2.mp3")->Play();
		b->health = 0;
		enemyBlow += 1;
		player->pExPoint -= 20;
		player->pExCount -= 100;
		enemySpawn += 1;
		combo += 1;
		comboTimer = 0.0f;
		comTimerFlag = true;

		auto mesh = meshBuffer.GetSkeletalMesh("Effect.Hit");
		mesh->Play(mesh->GetAnimationList()[0].name, false);
		mesh->SetAnimationSpeed(std::uniform_real_distribution<float>(0.75f, 1.0f)(rand) / 1.5f);
		glm::vec3 rot = player->rotation;
		rot.x += std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
		rot.y += std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
		rot.z += std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
		glm::vec3 scale(std::uniform_real_distribution<float>(1.0f, 1.5f)(rand) * 1.5f);
		auto effect = std::make_shared<SkeletalMeshActor>(mesh, "Effect.Hit", 1, p, rot, scale * b->scale);
		effects.Add(effect);
	});

	// プレイヤーの攻撃判定.
	ActorPtr attackCollision = player->GetAttackCollision();
	if (attackCollision)
	{
		bool hit = false;
		DetectCollision(attackCollision, enemies[i],
			[this, &hit](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
		{
			SkeletalMeshActorPtr bb = std::static_pointer_cast<SkeletalMeshActor>(b);
			bb->health -= a->health * player->pLevel;
			float scaleFactor = 1;
			const glm::vec3 playerFront = glm::rotate(
				glm::mat4(1), player->rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);

			if (bb->health <= 0)
			{
				bb->colLocal = Collision::Shape{};
				bb->health = 1;
				bb->GetMesh()->Play("Down", false);
				scaleFactor = 1.5f;
			}
			else
			{
				bb->GetMesh()->Play("Hit", false);
			}

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
		});

		if (hit)
		{
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
bool MainGameScene::Initialize()
{
	spriteRenderer.Init(1000, "Res/Sprite.vert", "Res/Sprite.frag");
	sprites.reserve(100);
	meshBuffer.Init(1'000'000 * sizeof(Mesh::Vertex), 3'000'000 * sizeof(GLushort));
	lightBuffer.Init(1);
	lightBuffer.BindToShader(meshBuffer.GetStaticMeshShader());
	lightBuffer.BindToShader(meshBuffer.GetTerrainShader());
	/*lightBuffer.BindToShader(meshBuffer.GetWaterShader());*/
	lightBuffer.BindToShader(meshBuffer.GetGrassShader());
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
	meshBuffer.LoadMesh("Res/MP.gltf");
	meshBuffer.LoadMesh("Res/Bomb.gltf");
	meshBuffer.LoadMesh("Res/grass.gltf");
	meshBuffer.LoadSkeletalMesh("Res/effect_hit_normal.gltf");
	meshBuffer.LoadSkeletalMesh("Res/effect_curse.gltf");

	//ゲーム内に使用している画像データ.
	spriteRenderer.GameSceneUI(sprites);

	progLighting.Reset(Shader::BuildFromFile("Res/FragmentLighting.vert", "Res/FragmentLighting.frag"));
	fontRenderer.Init(1000);
	fontRenderer.LoadFromFile("Res/font.fnt");

	heightMap.SetupGrassShader(meshBuffer, "Res/grass.gltf");

	//パーティクル・システムを初期化する.
	particleSystem.Init(1000);

	// FBOを作成する.
	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	fboMain = FramebufferObject::Create(window.Width(), window.Height(), GL_RGBA16F);
	Mesh::FilePtr rt = meshBuffer.AddPlane("RenderTarget");
	if (rt)
	{
		rt->materials[0].program = Shader::Program::Create(
			"Res/DepthOfField.vert", "Res/DepthOfField.frag");
		rt->materials[0].texture[0] = fboMain->GetColorTexture();
		rt->materials[0].texture[1] = fboMain->GetDepthTexture();

	}
	if (!rt || !rt->materials[0].program)
	{
		return false;
	}

	//DoF描画用のFBOを作る.
	fboDepthOfField = FramebufferObject::Create(window.Width(), window.Height(), GL_RGBA16F);

	//元解像度の縦横1/2(面積が1/4)の大きさのブルーム用FBOを作る.
	int w = window.Width();
	int h = window.Height();
	for (int j = 0; j < sizeof(fboBloom) / sizeof(fboBloom[0]); ++j)
	{
		w /= 2;
		h /= 2;
		for (int i = 0; i < sizeof(fboBloom[0]) / sizeof(fboBloom[0][0]); ++i)
		{
			fboBloom[j][i] = FramebufferObject::Create(w, h, GL_RGBA16F, FrameBufferType::colorOnly);
			if (!fboBloom[j][i])
			{
				return false;
			}
		}
	}

	//ブルーム・エフェクト用の平面ポリゴンメッシュを作成する.
	if (Mesh::FilePtr mesh = meshBuffer.AddPlane("BrightPassFilter"))
	{
		Shader::ProgramPtr p = Shader::Program::Create("Res/Simple.vert", "Res/BrightPassFilter.frag");
		p->Use();
		p->SetViewProjectionMatrix(glm::mat4(1));
		mesh->materials[0].program = p;
	}
	if (Mesh::FilePtr mesh = meshBuffer.AddPlane("NormalBlur"))
	{
		Shader::ProgramPtr p = Shader::Program::Create("Res/Simple.vert", "Res/NormalBlur.frag");
		p->Use();
		p->SetViewProjectionMatrix(glm::mat4(1));
		mesh->materials[0].program = p;
	}
	if (Mesh::FilePtr mesh = meshBuffer.AddPlane("Simple"))
	{
		Shader::ProgramPtr p = Shader::Program::Create("Res/Simple.vert", "Res/Simple.frag");
		p->Use();
		p->SetViewProjectionMatrix(glm::mat4(1));
		mesh->materials[0].program = p;
	}
	if (glGetError())
	{
		std::cout << "[エラー]" << __func__ << ": ブルーム用メッシュの作成に失敗.\n";
		return false;
	}

	//デプスシャドウマッピング用のFBOを作成する.
	{
		fboShadow = FramebufferObject::Create(
			4096, 4096, GL_NONE, FrameBufferType::depthOnly);
		if (glGetError())
		{
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
	if (!heightMap.LoadFromFile("Res/WhiteFront.tga", 20.0f, 0.5f))
	{
		return false;
	}

	if (!heightMap.CreateMesh(meshBuffer, "Terrain"))
	{
		return false;
	}

	//if (!heightMap.CreateWaterMesh(meshBuffer, "Water", 0.0)){ // 水面の高さは要調整.
	//	return false;
	//}

	if (StageNo >= 1)
	{
		if (StageNo == 1)
		{
			bgm = Audio::Engine::Instance().Prepare("Res/Audio/game_maoudamashii_4_field03.mp3");
		}
		else if (StageNo == 2)
		{
			enemyStock = 20;
			enemySpawn = 5;
			bgm = Audio::Engine::Instance().Prepare("Res/Audio/game_maoudamashii_7_rock01.mp3");
		}
		else if (StageNo == 3)
		{
			enemyStock = 30;
			enemySpawn = 10;
			bgm = Audio::Engine::Instance().Prepare("Res/Audio/game_maoudamashii_7_rock11.mp3");
		}
		else if (StageNo == 4)
		{
			enemyStock = 40;
			enemySpawn = 20;
			bgm = Audio::Engine::Instance().Prepare("Res/Audio/game_maoudamashii_7_rock20.mp3");
		}
		else if (StageNo == 5)
		{
			enemyStock = 100;
			enemySpawn = 100;
			bgm = Audio::Engine::Instance().Prepare("Res/Audio/game_maoudamashii_7_rock30.mp3");
		}
		bgm->Play(Audio::Flag_Loop);
	}

	glm::vec3 startPos(79, 0, 100);	//プレイヤーのスタート位置.
	startPos.y = heightMap.Height(startPos);
	player = std::make_shared<PlayerActor>(&heightMap, meshBuffer, startPos);

	if (player->playerID == 0)
	{
		player->SetStaticMesh(meshBuffer.GetFile("Res/Blackshadow.gltf"), 0);
	}
	else if (player->playerID == 1)
	{
		player->SetMesh(meshBuffer.GetSkeletalMesh("oni_small"), 0);
		player->GetMesh()->Play("Idle.LookAround");
	}
	else if (player->playerID == 2)
	{
		player->SetMesh(meshBuffer.GetSkeletalMesh("Bikuni"), 0);
		player->GetMesh()->Play("Idle");
	}
	else if (player->playerID == 3)
	{
		player->SetMesh(meshBuffer.GetSkeletalMesh("Skeleton"), 0);
		player->GetMesh()->Play("Idle");
	}

	// ライトを配置
	lights.Add(std::make_shared<DirectionalLightActor>
		("DLight", glm::vec3(1.0f, 0.94f, 0.91f), glm::normalize(glm::vec3(1, -1, -1))));

	lights.Update(0);
	lightBuffer.Update(lights, glm::vec3(0.1f, 0.05f, 0.15f));
	heightMap.UpdateLightIndex(lights);

	//ディフェンスポイントを配置.
	if (StageNo != 1)
	{
		const Mesh::FilePtr meshDefencePoint = meshBuffer.GetFile("Res/MP.gltf");
		for (int i = 0; i < 3; ++i)
		{
			glm::vec3 position(0);
			position.x = static_cast<float>(std::uniform_int_distribution<>(60, 100)(rand));
			position.z = static_cast<float>(std::uniform_int_distribution<>(80, 120)(rand));
			position.y = heightMap.Height(position) + 2.0f;
			glm::vec3 rotation(0);
			rotation.y = std::uniform_real_distribution<float>(0.0f, 3.14f * 2.0f)(rand);
			StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
				meshDefencePoint, "DefencePoint", 100, position, glm::vec3(0, 0, 0));
			p->colLocal = Collision::CreateCapsule(
				glm::vec3(0, 0, 0), glm::vec3(0, 1.5f, 0), 1.5f);
			p->scale = glm::vec3(1, 2, 1); // 見つけやすいように拡大.
			particleSystem.Crystal(position);
			defencePoint.Add(p);
		}

		//試練のステージの石壁を配置.
		{
			/*wall(walls, meshBuffer.GetFile("Res/Skeltal.gltf"),15,45,)*/
			//下側の壁.
			const size_t wallCount = 15;
			walls.Reserve(wallCount);
			const Mesh::FilePtr meshStoneWall = meshBuffer.GetFile("Res/Skeltal.gltf");

			//下側の壁.
			for (size_t i = 0; i < wallCount; ++i)
			{
				const float posX = 45 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(posX, 2, 35);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, 1), glm::vec3(10, 20, 1));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
			//上側の壁.
			for (size_t i = 0; i < wallCount; ++i)
			{
				const float posX = 45 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(posX, 2, -35);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, 11), glm::vec3(10, 20, 1));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
			//右側の壁.
			for (size_t i = 0; i < wallCount; ++i)
			{
				const float posZ = 45 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(35, 2, posZ);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(-1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, -1), glm::vec3(1, 20, 10));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
			//左側の壁.
			for (size_t i = 0; i < wallCount; ++i)
			{
				const float posZ = 45 - i * 6.0f;
				glm::vec3 position = startPos + glm::vec3(-35, 2, posZ);
				StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
					meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
				p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
					glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, -1), glm::vec3(1, 20, 10));
				p->scale = glm::vec3(1); // 見つけやすいように拡大.
				walls.Add(p);
			}
		}
	}

	//初期ステージ石壁を配置.

	if (StageNo == 1)
	{
		//下側の壁.
		const size_t wallCount = 9;
		walls.Reserve(wallCount);
		const Mesh::FilePtr meshStoneWall = meshBuffer.GetFile("Res/Skeltal.gltf");

		//下側の壁.
		for (size_t i = 0; i < wallCount; ++i)
		{
			const float posX = 25 - i * 6.0f;
			glm::vec3 position = startPos + glm::vec3(posX, 2, 25);
			StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
				meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
			p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
				glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, 1), glm::vec3(10, 20, 1));
			p->scale = glm::vec3(1); // 見つけやすいように拡大.
			walls.Add(p);
		}
		//上側の壁.
		for (size_t i = 0; i < wallCount; ++i)
		{
			const float posX = 25 - i * 6.0f;
			glm::vec3 position = startPos + glm::vec3(posX, 2, -23);
			StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
				meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
			p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
				glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, 11), glm::vec3(10, 20, 1));
			p->scale = glm::vec3(1); // 見つけやすいように拡大.
			walls.Add(p);
		}
		//右側の壁.
		for (size_t i = 0; i < wallCount; ++i)
		{
			const float posZ = 25 - i * 6.0f;
			glm::vec3 position = startPos + glm::vec3(25, 2, posZ);
			StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
				meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
			p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
				glm::vec3(-1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, -1), glm::vec3(1, 20, 10));
			p->scale = glm::vec3(1); // 見つけやすいように拡大.
			walls.Add(p);
		}
		//左側の壁.
		for (size_t i = 0; i < wallCount; ++i)
		{
			const float posZ = 25 - i * 6.0f;
			glm::vec3 position = startPos + glm::vec3(-23, 2, posZ);
			StaticMeshActorPtr p = std::make_shared<StaticMeshActor>(
				meshStoneWall, "Skeltal", 100, position, glm::vec3(0, 0, 0));
			p->colLocal = Collision::CreateOBB(glm::vec3(0, 0, 0),
				glm::vec3(1, 0, 0), glm::vec3(0, -10, 0), glm::vec3(0, 0, -1), glm::vec3(1, 20, 10));
			p->scale = glm::vec3(1); // 見つけやすいように拡大.
			walls.Add(p);
		}

		{
			//ワープゲートの配置.
			//上から東西南北の順.
			Gate(warp, objects, meshBuffer.GetFile("Res/Gate.gltf"), meshBuffer.GetFile("Res/GateBlock.gltf"),
				player->position, glm::vec3(-15, 0, -15), 0);
			Gate(warp, objects, meshBuffer.GetFile("Res/Gate.gltf"), meshBuffer.GetFile("Res/GateBlock.gltf"),
				player->position, glm::vec3(-5, 0, -15), 1);
			Gate(warp, objects, meshBuffer.GetFile("Res/Gate.gltf"), meshBuffer.GetFile("Res/GateBlock.gltf"),
				player->position, glm::vec3(5, 0, -15), 2);
			Gate(warp, objects, meshBuffer.GetFile("Res/Gate.gltf"), meshBuffer.GetFile("Res/GateBlock.gltf"),
				player->position, glm::vec3(15, 0, -15), 3);
		}

		{
			//乗り移れる体(初級)の配置.
			ActorChange(meshBuffer.GetSkeletalMesh("oni_small"), items, startPos, glm::vec3(-5, 0, -5), 1);
			//乗り移れる体(中級)の配置.
			ActorChange(meshBuffer.GetSkeletalMesh("Bikuni"), items, startPos, glm::vec3(0, 0, -5), 2);
			//乗り移れる体(上級)の配置.
			ActorChange(meshBuffer.GetSkeletalMesh("Skeleton"), items, startPos, glm::vec3(5, 0, -5), 3);
		}


		//ステージクリアしたゲートの上に火のパーティクルを表示.
		if (StClearedE)
		{
			particleSystem.Gate(player->position, glm::vec3(-15, 4, -15));
		}
		if (StClearedW)
		{
			particleSystem.Gate(player->position, glm::vec3(5, 4, -15));
		}
		if (StClearedS)
		{
			particleSystem.Gate(player->position, glm::vec3(-5, 4, -15));
		}
		if (StClearedN)
		{
			particleSystem.Gate(player->position, glm::vec3(15, 4, -15));
		}
	}

	{
		effects.Reserve(100);
		for (const auto& e : objectives)
		{
			for (size_t i = 0; i < 1; ++i)
			{
				const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("CurseEffect");
				const std::vector<Mesh::Animation>& animList = mesh->GetAnimationList();
				if (!animList.empty())
				{
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

	//オープニングスクリプトを実行.
	player->Update(0);
	if (StageNo >= 1 && StageNo <= 5)
	{
		camera.target = player->position + glm::vec3(0, 0, -1);
		camera.position = camera.target + glm::vec3(0, 5, 7);

		if (!StClearedE && !StClearedN && !StClearedS && !StClearedW && StageNo == 1 && eventFrag == false)
		{
			SceneStack::Instance().Push(std::make_shared<EventScene>("Res/Event/OpeningScript.txt"));
		}
		if (StageNo == 2)
		{
			SceneStack::Instance().Push(std::make_shared<EventScene>("Res/Event/Stage2.txt"));
		}
		else if (StageNo == 3)
		{
			SceneStack::Instance().Push(std::make_shared<EventScene>("Res/Event/Stage3.txt"));
		}
		else if (StageNo == 4)
		{
			SceneStack::Instance().Push(std::make_shared<EventScene>("Res/Event/Stage4.txt"));
		}
		else if (StageNo == 5)
		{
			SceneStack::Instance().Push(std::make_shared<EventScene>("Res/Event/Stage5.txt"));
		}
	}
	return true;
}

/**
*プレイヤーの入力を処理する.
*/
void MainGameScene::ProcessInput()
{
	GLFWEW::Window& window = GLFWEW::Window::Instance();
	const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();

	if (state == State::play)
	{
		player->ProcessInput(camera);
	}
	//デバック用ボタン.
	//クリア条件.
	if (window.GetGamePad().buttonDown & GamePad::Z)
	{
		enemyBlow = enemyStock;
	}
	//防衛ラインを減らす.
	if (window.GetGamePad().buttonDown & GamePad::XX)
	{
		defenceLine -= 10;
	}
	//HP減少.
	if (window.GetGamePad().buttonDown & GamePad::C)
	{
		player->pHP -= 10;
	}
	//MP減少.
	if (window.GetGamePad().buttonDown & GamePad::V)
	{
		player->pMP -= 5;
	}
	//レベルアップ.
	if (window.GetGamePad().buttonDown & GamePad::BB)
	{
		player->pExPoint = 0;
		player->pExCount = 0;
	}
	if (window.GetGamePad().buttonDown & GamePad::N)
	{
		combo += 1;
	}

	if (window.GetGamePad().buttonDown & GamePad::M)
	{

	}

	//メニュー画面の表示.
	if (state == State::play &&
		gameClearFlag == false && gameOverFlag == false && nextStateFlag == false &&
		window.GetGamePad().buttonDown & GamePad::SPACE)
	{
		Audio::Engine::Instance().Prepare("Res/Audio/system.mp3")->Play();
		state = State::menu;
		selectCount = 1;
		spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 0);
		spriteRenderer.SprRootChange(sprites, player->playerID, 0);
	}
	//スペースボタンでpも閉じれる.
	else if (window.GetGamePad().buttonDown & GamePad::SPACE)
	{
		Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
		state = State::play;
		selectCount = 0;
		spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 1);
		spriteRenderer.SprRootChange(sprites, StageNo, 1);
	}

	//スキルセット画面時の選択アイコンを動かす.
	if (state == State::sMenu && skComCount > -1)
	{
		//スキル画面操作.
		if (window.GetGamePad().buttonDown & GamePad::DPAD_RIGHT)
		{
			selectCount = -1;
			skComCount = 1;
		}
		else if (window.GetGamePad().buttonDown & GamePad::DPAD_LEFT)
		{
			selectCount = 3;
			skComCount = 0;
		}
		skSelectUI(1, 0, 4, 4, 5, 1);
		//スキル表の時.
		spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 2);
	}

	//メニュー画面時の選択アイコンを動かす.
	if (state == State::menu || state == State::cMenu || state == State::sMenu && selectCount > -1)
	{
		selectUI(1, 0, 5, 5, 6, 1);
		//選択ボタンの操作.
		spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 3);
	}

	//ステージ移行の有無時の選択アイコンを動かす.
	if (state == State::select)
	{
		if (timer < 0.1f)
		{
			selectUI(0, -1, 1, 1, 2, 0);
		}
		//”はい”でステージ移行、”いいえ”でプレイに戻る.
		if (selectCount == 0 && window.GetGamePad().buttonDown & GamePad::START)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();

			player->position += glm::vec3(0, 100, 0);
			bgm->Stop();
			timer = 2.0f;
			selectCount = 0;
		}
		else if (selectCount == 1 && window.GetGamePad().buttonDown & GamePad::START)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			state = State::play;
			player->position = glm::vec3(79, 0, 100);
			selectCount = 0;
			spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 4);
		}

		//ステージ移行の有無の選択ボタンの操作.
		spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 5);
	}

	//リザルト画面時の選択アイコンを動かす.
	if (state == State::result)
	{
		selectUI(0, -1, 1, 1, 2, 0);
		spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 6);

		//リザルト画面から次のステージ移行.
		if (selectCount == 0 && window.GetGamePad().buttonDown & GamePad::START)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			state = State::play;
			nextStateFlag = false;
			StageNo = 1;
			SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
			return;
		}
		else if (selectCount == 1 && window.GetGamePad().buttonDown & GamePad::START)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			loadTimer = 0.5f;
		}
	}

	//選択アイコン使用結果.
	//プレイヤー情報画面.
	if (nextStateFlag == false && window.GetGamePad().buttonDown & GamePad::START)
	{
		if (selectCount == 1)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			state = State::menu;
			spriteRenderer.SprRootChange(sprites, StageNo, 2);
		}

		//コマンド表.
		else if (selectCount == 2)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			gamePadText = true;
			state = State::cMenu;
			spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 7);
		}
		//スキル表.
		else if (selectCount == 3)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			state = State::sMenu;
			spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 7);
		}
		//タイトルに戻る.
		else if (selectCount == 4)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			bgm->Stop();
			StageNo = 1;
			eventFrag = true;
			exit(0);
		}
		//プレイに戻る.
		else if (selectCount == 5)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 8);

			if (StageNo != 1)
			{
				sprites[26].Scale(glm::vec2(2));
				sprites[27].Scale(glm::vec2(1));
			}
			sprites[28].Scale(glm::vec2(0));
			state = State::play;
			selectCount = 0;
		}
	}

	if (state == State::cMenu)
	{
		if (gamePadText && window.GetGamePad().buttonDown & GamePad::Y)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			gamePadText = false;
		}
		else if (!gamePadText && window.GetGamePad().buttonDown & GamePad::Y)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/OK.mp3")->Play();
			gamePadText = true;
		}
	}

	//ミニマップ表示.
	if (window.GetGamePad().buttons & GamePad::M || window.GetGamePad().buttons & GamePad::YY)
	{
		miniMapFlag = true;
	}
	else
	{
		miniMapFlag = false;
	}

	//視点切り替え.
	if (state == State::play)
	{
		if (window.GetGamePad().buttons & GamePad::DPAD_LEFT || window.GetGamePad().buttons & GamePad::L2)
		{
			cameraFar = true;
		}
		else {
			cameraFar = false;
		}
		if (window.GetGamePad().buttons & GamePad::DPAD_RIGHT || window.GetGamePad().buttons & GamePad::R2)
		{
			cameraNear = true;
		}
		else {
			cameraNear = false;
		}

		//範囲攻撃用フラグ立て.
		if (player->playerID == 3 && window.GetGamePad().buttons & GamePad::B)
		{
			shotTimerFlagB = true;
			shotTimerFlagC = true;
		}

		//スキルセット攻撃用フラグ立て.
		//A、Jボタン攻撃用フラグ立て.
		if (window.GetGamePad().buttons & GamePad::A)
		{
			sCommand = true;
		}
		else
		{
			sCommand = false;
		}

		//X、Hボタン攻撃用フラグ立て.
		if (wCommand == false && window.GetGamePad().buttons & GamePad::X)
		{
			wCommand = true;
		}

		//B、Kボタン攻撃用フラグ立て.
		if (eCommand == false && window.GetGamePad().buttons & GamePad::B)
		{
			if (player->playerID == 3 && player->pMP > 0)
			{
				particleFlagB = true;
			}
			if (player->playerID == 2 && player->pMP > 0)
			{
				chargeShotFlagA = true;
			}
			eCommand = true;
		}

		//Y、Uボタン攻撃用フラグ立て.
		if (nCommand == false && window.GetGamePad().buttons & GamePad::Y)
		{
			if (player->playerID == 3 && player->pMP > 0)
			{
				particleFlagY = true;
			}
			if (player->playerID == 2 && player->pMP > 0)
			{
				shotTimerFlagA = true;
			}
			if (player->playerID == 1)
			{
				bombFlag = true;
			}
			nCommand = true;
		}

		//砂埃のパーティクル制御.
		if (player->playerID != 0)
		{
			if (window.GetGamePad().buttons & GamePad::DPAD_UP ||
				window.GetGamePad().buttons & GamePad::DPAD_DOWN)
			{
				walkParticleFlag = true;
			}
			else
			{
				walkParticleFlag = false;
			}
		}
	}
}

/**
*シーンを更新する.
*
*@param deltaTime	前回からの更新からの経過時間（秒）.
*/
void MainGameScene::Update(float deltaTime)
{
	const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
	spriteRenderer.BeginUpdate();

	for (const Sprite& e : sprites)
	{
		spriteRenderer.AddVertices(e);
	}
	spriteRenderer.EndUpdate();

	const GLFWEW::Window& window = GLFWEW::Window::Instance();
	const float w = window.Width();
	const float h = window.Height();
	const float lineHeight = fontRenderer.LineHeight();

	// カメラの状態を更新.
	{
		//カメラ機能追加項目.
		const glm::vec3 vCameraFront = glm::rotate(
			glm::mat4(1), camera.rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 1, 15, 1);
		//元になる行列、回転・角度、回転の軸

		if (state != State::select)
		{
			camera.target = player->position;
			camera.target.y += 2.0f;
			camera.position = camera.target + vCameraFront;
		}
		if (cameraFar == true)
		{
			camera.rotation.y += glm::radians(70.0f) * deltaTime;
		}
		if (cameraNear == true)
		{
			camera.rotation.y -= glm::radians(70.0f) * deltaTime;
		}
	}

	//ステートがプレイの時のみ.
	if (state == State::play && nextStateFlag == false)
	{
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
		bullet[1].Update(deltaTime);
		bullet[2].Update(deltaTime);
	}

	DetectCollision(player, trees);
	DetectCollision(player, objects);
	DetectCollision(player, objectives);
	DetectCollision(player, walls);

	if (state == State::play)
	{
		//敵の出現.
		if (StageNo != 1)
		{
			DetectCollision(player, enemies[0]);
			DetectCollision(player, enemies[1]);
			DetectCollision(player, enemies[2]);
			DetectCollision(player, enemies[3]);
			DetectCollision(player, defencePoint);
			DetectCollision(defencePoint, enemies[0]);
			DetectCollision(defencePoint, enemies[1]);
			DetectCollision(defencePoint, enemies[2]);
			DetectCollision(defencePoint, enemies[3]);

			enemyPopTimerA += deltaTime;
			enemyPopTimerB += deltaTime;
			enemyPopTimerC += deltaTime;

			if (enemyPopTimerA >= 6.0f)
			{
				if (enemySpawn >= 0)
				{
					EnemySpawn(enemies, glm::vec3(1.0f), randTarget, 0);
				}
				enemyPopTimerA = 0.0f;
				enemySpawn -= 3;
			}
			if (enemyPopTimerB >= 9.0f)
			{
				if (enemySpawn >= 0)
				{
					EnemySpawn(enemies, glm::vec3(1.5f), randTarget, 1);
				}
				enemyPopTimerB = 0.0f;
				enemySpawn -= 3;
			}
			if (enemyPopTimerC >= 12.0f)
			{
				if (enemySpawn >= 0)
				{
					EnemySpawn(enemies, glm::vec3(2.0f), randTarget, 2);
				}
				enemyPopTimerC = 0.0f;
				enemySpawn -= 3;
			}

			EnemyAI(deltaTime, defencePoint, 0, 0);
			EnemyAI(deltaTime, defencePoint, 1, 1);
			EnemyAI(deltaTime, defencePoint, 2, 2);

			//敵がプレイヤーに向かって追いかけてくる.
			for (auto& e : enemies[3])
			{
				SkeletalMeshActorPtr enemy = std::static_pointer_cast<SkeletalMeshActor>(e);
				Mesh::SkeletalMeshPtr mesh = enemy->GetMesh();

				//死亡したら消す.
				if (mesh->GetAnimation() == "Down")
				{
					if (mesh->IsFinished())
					{
						enemy->health = 0;
						enemyBlow += 1;
						player->pExPoint -= 20;
						player->pExCount -= 50;
						enemySpawn += 1;
						combo += 1;
						comboTimer = 0.0f;
						comTimerFlag = true;
					}
					continue;
				}

				const float moveSpeed = baseSpeed * 3.0f;
				const float rotationSpeed = baseSpeed * glm::radians(60.0f);
				const float frontRange = glm::radians(15.0f);

				const glm::vec3 v = player->position - e->position;
				const glm::vec3 vTarget = glm::normalize(v);
				float radian = std::atan2(-vTarget.z, vTarget.x) - glm::radians(90.0f);

				if (radian <= 0)
				{
					radian += glm::radians(360.0f);
				}

				const glm::vec3 vEnemyFront = glm::rotate(
					glm::mat4(1), e->rotation.y + glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);
				//元になる行列、回転・角度、回転の軸

				//ターゲットが正面にいなかったら、正面にとらえるような左右に旋回.
				if (std::abs(radian - e->rotation.y) > frontRange)
				{
					const glm::vec3 vRotDir = glm::cross(vEnemyFront, vTarget);
					if (vRotDir.y >= 0)
					{
						e->rotation.y += rotationSpeed * deltaTime;
						if (e->rotation.y >= glm::radians(360.0f))
						{
							e->rotation.y -= glm::radians(360.0f);
						}
					}
					else
					{
						e->rotation.y -= rotationSpeed * deltaTime;
						if (e->rotation.y < 0)
						{
							e->rotation.y += glm::radians(360.0f);
						}
					}
				}
				if (e->health <= 0)
				{
					e->velocity = glm::vec3(0);	//死んでいるのでもう移動しない.
				}
				//十分に接近していなければ移動する。接近していれば攻撃する.
				if (glm::length(v) > 2.5f)
				{
					e->velocity = vEnemyFront * moveSpeed;
					if (mesh->GetAnimation() != "Run")
					{
						mesh->Play("Run");
					}
				}
				else
				{
					e->velocity = glm::vec3(0);	//接近しているのでもう移動しない.
					if (mesh->GetAnimation() != "Wait")
					{
						if ((mesh->GetAnimation() != "Attack" && mesh->GetAnimation() != "Hit") ||
							mesh->IsFinished())
						{
							mesh->Play("Wait");
						}
					}
					//定期的に攻撃状態になる.
					if (isAttacking)
					{
						isAttacking = false;
						attackingTimer = 1.0f;	//次の攻撃は1秒後.
						mesh->Play("Attack", false);
					}
					else
					{
						attackingTimer -= deltaTime;
						if (attackingTimer <= 0)
						{
							isAttacking = true;
							player->pHP -= 20;
						}
					}
				}
			}
			EnemyDetectCollision(0);
			EnemyDetectCollision(1);
			EnemyDetectCollision(2);
			EnemyDetectCollision(3);
		}

		////砂埃のパーティクル.
		if (walkParticleFlag == true)
		{
			walkParticleTimer -= deltaTime;
			{
				particleSystem.Dust(player->position);
			}
			if (walkParticleTimer <= 0.1f)
			{
				Audio::Engine::Instance().Prepare("Res/Audio/asioto.mp3")->Play();
				walkParticleTimer = 100.0f;
			}
			else if (player->playerID == 1 && walkParticleTimer <= 99.5f)
			{
				walkParticleTimer = 0.0f;
			}
			else if (player->playerID >= 2 && walkParticleTimer <= 99.7f)
			{
				walkParticleTimer = 0.0f;
			}
		}

		//魂風のパーティクル.
		if (player->playerID == 0)
		{
			particleSystem.Soul(player->position);
		}

		//骸骨のY、Uボタン攻撃のパーティクル.
		const Mesh::FilePtr meshShot = meshBuffer.GetFile("Res/Triangle.gltf");

		if (particleFlagY == true)
		{
			if (player->playerID == 3 && player->pAbility >= 3)
			{
				particleTimerA -= deltaTime;

				if (particleTimerA <= -1.4f)
				{
					StaticMeshActorPtr Shot = std::make_shared<StaticMeshActor>(
						meshShot, "Shot", 100, player->position, glm::vec3(0, 0, 0));
					Shot->scale = glm::vec3(0);
					bullet[0].Add(Shot);
					particleTimerA = 100.0f;
					{
						particleSystem.BoneAttackY1(player->position, player->rotation);
					}
					Audio::Engine::Instance().Prepare("Res/Audio/magic-flame1.mp3")->Play();
				}
				else if (particleTimerA <= 99.0f)
				{
					for (ActorPtr& e : bullet[0])
					{
						e->health = 0;
						particleFlagY = false;
						particleTimerA = 0.0f;
					}
				}
			}
		}

		////骸骨のY、Uボタン攻撃のパーティクル.
		if (particleFlagY == true)
		{
			particleTimerA -= deltaTime;
			if (particleTimerA <= 0.1f)
			{
				{
					particleSystem.BoneAttackY2(player->position);
				}
			}
		}
		//骸骨のB、Kボタン攻撃のパーティクル.
		if (particleFlagB == true)
		{
			if (player->playerID == 3 && player->pAbility >= 4)
			{
				particleTimerB -= deltaTime;
				if (particleTimerB <= -2.0f)
				{
					StaticMeshActorPtr Shot = std::make_shared<StaticMeshActor>(
						meshShot, "Shot", 100, player->position, glm::vec3(0, 0, 0));
					Shot->scale = glm::vec3(0);
					bullet[0].Add(Shot);
					particleTimerB = 100.0f;
					{
						particleSystem.BoneAttackB(player->position, 0);
					}
					Audio::Engine::Instance().Prepare("Res/Audio/katana-slash5.mp3")->Play();
				}
				else if (particleTimerB <= 99.0f)
				{
					for (ActorPtr& e : bullet[0])
					{
						e->health = 0;
						particleFlagB = false;
						particleTimerB = 0.0f;
					}
				}
			}
		}

		////骸骨のB、Kボタン攻撃のパーティクル.
		if (particleFlagB == true)
		{
			particleTimerB -= deltaTime;
			if (particleTimerB <= 0.1f)
			{
				{
					particleSystem.BoneAttackB(player->position, 1);
				}
			}
		}

		//ウィザードのY、Uボタンの炎攻撃.
		if (shotTimerFlagA == true)
		{
			if (player->playerID == 2 && player->pAbility >= 3)
			{
				playerBulletTimerA -= deltaTime;

				if (playerBulletTimerA <= -0.3f)
				{
					StaticMeshActorPtr Shot = std::make_shared<StaticMeshActor>(
						meshShot, "Shot", 100, player->position, glm::vec3(0, 0, 0));
					const float speed = 5.0f;	//弾の移動速度(m/秒).
					const glm::mat4 matRotY = glm::rotate(
						glm::mat4(1), player->rotation.y, glm::vec3(0, 1, 0));
					const glm::vec3 matRotZ = glm::rotate(
						glm::mat4(1), player->rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 3, 1);
					glm::vec3 rot = player->rotation;
					rot.y += std::uniform_real_distribution<float>(0, glm::radians(360.0f))(rand);
					Shot->rotation += rot.y;
					Shot->scale = glm::vec3(0);
					Shot->colLocal = Collision::CreateSphere(
						glm::vec3(0, 0.5f, 0), 1.5f);
					Shot->velocity = matRotY * glm::vec4(0, 0, speed, 1);
					bullet[0].Add(Shot);
					playerBulletTimerA = 100.0f;
					{
						particleSystem.WizardAttackY(player->position, player->rotation);
					}
					Audio::Engine::Instance().Prepare("Res/Audio/magic-flame2.mp3")->Play();
				}
				else if (playerBulletTimerA <= 99.0f)
				{
					for (ActorPtr& e : bullet[0])
					{
						e->health = 0;
						shotTimerFlagA = false;
						playerBulletTimerA = 0.0f;
					}
				}
			}
		}

		//ウィザードのB、Kボタン攻撃の氷攻撃.
		if (chargeShotFlagA == true)
		{
			if (player->playerID == 2 && player->pAbility >= 4)
			{
				playerBulletTimerB -= deltaTime;
				const Mesh::FilePtr meshMeteo = meshBuffer.GetFile("Res/Skeltal.gltf");
				const float speed = 10.0f;	//弾の移動速度(m/秒).
				const glm::vec3 matRotY = glm::rotate(
					glm::mat4(1), player->rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 7, 3, 1);

				if (playerBulletTimerB <= -0.5f)
				{
					for (size_t i = 0; i < 10; ++i)
					{
						const glm::vec3 matRotZ = glm::rotate(
							glm::mat4(1), player->rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(0, i, i, 1);
						glm::vec3 setPosition = player->position + matRotY + matRotZ;
						StaticMeshActorPtr Meteo = std::make_shared<StaticMeshActor>(
							meshMeteo, "Shot", 100, setPosition, glm::vec3(0, 0, 0));
						Meteo->scale = glm::vec3(0);
						Meteo->velocity = glm::vec3(0, -speed, 0);
						Meteo->colLocal = Collision::CreateSphere(
							glm::vec3(0, 0.2f, 0), 2.0f);
						bullet[1].Add(Meteo);
						playerBulletTimerB = 100.0f;
						{
							particleSystem.WizardAttackB(player->position, player->rotation);
						}
					}
					Audio::Engine::Instance().Prepare("Res/Audio/magic-ice2.mp3")->Play();
				}
				else if (playerBulletTimerB <= 98.0f)
				{
					for (ActorPtr& e : bullet[1])
					{
						e->health = 0;
						chargeShotFlagA = false;
						playerBulletTimerB = 0.0f;
					}
				}
			}
		}
	}

	//右のステージ移行.
	if (state == State::play)
	{
		//右のステージ移行.
		DetectCollision(player, warp[0],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
		{
			b->health = 1;
			stageChage = 1;
			allTimer = 0.01f;
		});

		//左のステージ移行.
		DetectCollision(player, warp[1],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
		{
			b->health = 1;
			stageChage = 2;
			allTimer = 0.01f;
		});

		//下のステージ移行.
		DetectCollision(player, warp[2],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
		{
			b->health = 1;
			stageChage = 3;
			allTimer = 0.01f;
		});

		//上のステージ移行.
		DetectCollision(player, warp[3],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
		{
			b->health = 1;
			stageChage = 4;
			allTimer = 0.01f;
		});
	}

	//魂(ソウル).
	if (player->playerID != 0)
	{
		DetectCollision(player, items[0],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
		{
			b->health = 0;
			Audio::Engine::Instance().Prepare("Res/Audio/CharacterGet.mp3")->Play();
			player->SetStaticMesh(meshBuffer.GetFile("Res/Blackshadow.gltf"), 0);
			player->playerID = 0;
		});
	}

	//ゴブリン(初級).
	if (player->playerID != 1)
	{
		DetectCollision(player, items[1],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/CharacterGet.mp3")->Play();
			player->SetMesh(meshBuffer.GetSkeletalMesh("oni_small"), 0);
			player->GetMesh()->Play("Run");
			player->playerID = 1;
		});
	}

	//人間(中級).
	if (player->playerID != 2)
	{
		DetectCollision(player, items[2],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/CharacterGet.mp3")->Play();
			player->SetMesh(meshBuffer.GetSkeletalMesh("Bikuni"), 0);
			player->GetMesh()->Play("Run");
			player->playerID = 2;
		});
	}

	//スケルトン(上級).
	if (player->playerID != 3)
	{
		DetectCollision(player, items[3],
			[this](const ActorPtr& a, const ActorPtr& b, const glm::vec3& p)
		{
			Audio::Engine::Instance().Prepare("Res/Audio/CharacterGet.mp3")->Play();
			player->SetMesh(meshBuffer.GetSkeletalMesh("Skeleton"), 0);
			player->GetMesh()->Play("Wakigamae.Walk");
			player->playerID = 3;
		}
		);
	}

	effects.Update(deltaTime);
	for (auto& e : effects)
	{
		Mesh::SkeletalMeshPtr mesh = std::static_pointer_cast<SkeletalMeshActor>(e)->GetMesh();

		if (mesh->IsFinished())
		{
			e->health = 0;
		}
		else if (e->name == "Effect.Hit")
		{
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

	for (auto e : objects)
	{
		const std::vector<ActorPtr> neighborhood = lights.FindNearbyActors(e->position, 20);
		std::vector<int> pointLightIndex;
		std::vector<int> spotLightIndex;
		pointLightIndex.reserve(neighborhood.size());
		spotLightIndex.reserve(neighborhood.size());

		for (auto light : neighborhood)
		{
			if (PointLightActorPtr p = std::dynamic_pointer_cast<PointLightActor>(light))
			{
				if (pointLightIndex.size() < 8)
				{
					pointLightIndex.push_back(p->index);
				}
			}
			else if (SpotLightActorPtr p = std::dynamic_pointer_cast<SpotLightActor>(light))
			{
				if (spotLightIndex.size() < 8)
				{
					spotLightIndex.push_back(p->index);
				}
			}
		}
		StaticMeshActorPtr p = std::static_pointer_cast<StaticMeshActor>(e);
		p->SetPointLightList(pointLightIndex);
		p->SetSpotLightList(spotLightIndex);
	}

	//経験値でレベルアップ.
	if (player->pExPoint <= 0)
	{
		Audio::Engine::Instance().Prepare("Res/Audio/LVUP.mp3")->Play();
		player->pLevel += 1;
		player->pAbility += 1;
		player->pExPoint = 100;
		player->pExCount = 500;
		player->maxHP += 100;
		player->maxMP += 10;
		player->pHP = player->maxHP;
		player->pMP = player->maxMP;

		particleSystem.LevelUp(player->position);
	}

	//HPが０か防衛ラインのHPが０になったらゲームオーバーフラグが立つ.
	if (player->pHP <= 0 || defenceLine <= 0)
	{
		if (gameClearFlag == false && nextStateFlag == false)
		{
			gameOverFlag = true;
		}
	}

	//敵を全滅させたらゲームクリア.
	if (player->pHP > 0 && defenceLine > 0 && enemyStock <= enemyBlow)
	{
		if (gameOverFlag == false && nextStateFlag == false)
		{
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
	bullet[1].UpdateDrawData(deltaTime);
	bullet[2].UpdateDrawData(deltaTime);

	fontRenderer.BeginUpdate();

	//ゲームオーバー時.
	if (gameOverFlag == true)
	{
		bgm->Stop();
		Audio::Engine::Instance().Prepare("Res/Audio/GameOver.mp3")->Play();
		sprites[10].Scale(glm::vec2(1));
		gameOverFlag = false;
		nextStateFlag = true;
		defenceFlag = false;
		overTimer = 3.0f;
	}
	//ゲームクリア時.
	else if (gameClearFlag == true)
	{
		bgm->Stop();
		Audio::Engine::Instance().Prepare("Res/Audio/GameClear.mp3")->Play();
		sprites[11].Scale(glm::vec2(1));
		gameClearFlag = false;
		nextStateFlag = true;
		defenceFlag = false;
		clearTimer = 3.0f;
	}

	//ゲームステージのみに描画するUI.
	//ミニマップやクリスタルの体力ゲージ.
	if (StageNo != 1)
	{
		if (state == State::play)
		{
			spriteRenderer.MiniMap(sprites, defencePoint, miniMapFlag, player->position);

			spriteRenderer.DefenceUI(defenceLine, sprites);
		}
		else
		{
			spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 9);
		}

		spriteRenderer.comboUI(combo, comboTimer, comboBuf, sprites);
	}

	//HP,MPバーの表示.
	if (player->pHP >= 0 || player->pMP >= 0)
	{
		if (state == State::menu)
		{
			sprites[16].Scale(glm::vec2(5.0f * player->pHP / player->maxHP, 0.4f));
			sprites[16].Position(glm::vec3((80 * 2.5f * player->pHP / player->maxHP) / 2 + 150, 110, 0));

			sprites[17].Scale(glm::vec2(2.5f * player->pMP / player->maxMP, 0.4f));
			sprites[17].Position(glm::vec3((40 * 2.5f * player->pMP / player->maxMP) / 2 + 150, 60, 0));
		}
	}

	//各キャラのスキルコマンドのアイコン.
	if (state == State::play)
	{
		spriteRenderer.pCommandUI(player->playerID, player->pAbility, sprites);
	}

	//アイコンの使用可、不可.
	//攻撃のインターバル画像を重ねる.
	if (state == State::play)
	{
		/*spriteRenderer.comIntUI(sprites,player->pAbility,player->playerID,
			sCommand,eCommand,nCommand,wCommand,eIntTimer,nIntTimer,wIntTimer,deltaTime);*/

			//初期攻撃したかどうか.
		if (sCommand == true)
		{
			//下.
			sprites[36].Scale(glm::vec2(0));
		}
		else if (sCommand == false)
		{
			sprites[36].Scale(glm::vec2(1.55f, 1.55f));
			sprites[36].Position(glm::vec3(500, -310, 0));
		}
		//右.
		if (player->pAbility >= 4)
		{
			if (eCommand == true)
			{
				eIntTimer += deltaTime;
				sprites[34].Scale(glm::vec2(1.55f, 1.55f * eIntTimer / 4.0f));
				sprites[34].Position(glm::vec3(580, 24.8f * 2.5f * eIntTimer / 4.0f / 2 - 261, 0));
			}
			else if (eIntTimer == false)
			{
				sprites[34].Scale(glm::vec2(1.55f, 1.55f));
				sprites[34].Position(glm::vec3(580, -230, 0));
			}
		}
		//上.
		if (player->pAbility >= 3)
		{
			if (nCommand == true)
			{
				nIntTimer += deltaTime;
				sprites[37].Scale(glm::vec2(1.55f, 1.55f * nIntTimer / 3.0f));
				sprites[37].Position(glm::vec3(500, 24.8f * 2.5f * nIntTimer / 3.0f / 2 - 181, 0));
			}
			else if (nCommand == false)
			{
				sprites[37].Scale(glm::vec2(1.55f, 1.55f));
				sprites[37].Position(glm::vec3(500, -150, 0));
			}
		}
		//左.
		if (player->pAbility >= 2 && player->playerID != 1)
		{
			if (wCommand == true)
			{
				wIntTimer += deltaTime;
				sprites[35].Scale(glm::vec2(1.55f, 1.55f * wIntTimer / 2.0f));
				sprites[35].Position(glm::vec3(420, 24.8f * 2.5f * wIntTimer / 2.0f / 2 - 261, 0));
			}
			else if (wCommand == false)
			{
				sprites[35].Scale(glm::vec2(1.55f, 1.55f));
				sprites[35].Position(glm::vec3(420, -230, 0));
			}
		}

		if (player->playerID == 1)
		{
			sprites[34].Scale(glm::vec2(0));
			sprites[35].Scale(glm::vec2(0));
			sprites[37].Scale(glm::vec2(0));
		}
		else if (player->playerID == 0)
		{
			sprites[34].Scale(glm::vec2(0));
			sprites[35].Scale(glm::vec2(0));
			sprites[37].Scale(glm::vec2(0));
		}

		if (wIntTimer >= 2.0f)
		{
			wCommand = false;
			wIntTimer = 0.0f;
		}
		if (eIntTimer >= 4.0f)
		{
			eCommand = false;
			eIntTimer = 0.0f;
		}
		if (nIntTimer >= 3.0f)
		{
			nCommand = false;
			nIntTimer = 0.0f;
		}
	}

	//コンボタイマー管理.
	if (comTimerFlag == true)
	{
		comboTimer += deltaTime;
		if (comboTimer >= 10.0f)
		{
			comTimerFlag = false;
			combo = 0;
			comboTimer = 0.0f;
		}
	}

	if (timer > 0.0f)
	{
		timer -= deltaTime;

		//ロード画面を挟む.
		if (timer <= 0.0f && player->pHP > 0)
		{
			bgm->Stop();
			spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 10);
			state = State::load;
			loadTimer = 4.0f;
		}
	}

	//ステージ移行の有無.
	if (allTimer > 0.0f)
	{
		allTimer -= deltaTime;

		if (allTimer <= 0.0f)
		{
			state = State::select;
			eventFrag = true;

			if (player->playerID != 0)
			{
				player->GetMesh()->Play("Idle");
			}
			spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 11);
		}
		allTimer = 0.0f;
	}

	//シーン切り替え待ち.
	if (overTimer > 0.0f)
	{
		overTimer -= deltaTime;

		//ゲームオーバー画面へ.
		if (overTimer <= 0.0f)
		{
			if (player->pHP <= 0 || defenceLine <= 0)
			{
				StageNo = 1;
				player->pLevel = 1;		//プレイヤーレベル.
				player->pAbility = 1;	//アビリティレベル.
				player->pExPoint = 100;	//経験値.
				player->pExCount = 500;	//レベルアップまで.
				player->pMP = 50;		//MP.
				player->pHP = 200;		//HP.
				player->maxMP = 50;		//maxMP.
				player->maxHP = 200;	//maxHP.
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
	if (clearTimer > 0.0f)
	{
		clearTimer -= deltaTime;
		if (clearTimer <= 0.0f)
		{
			if (nextStateFlag == true && player->pHP > 0 && defenceLine > 0 && enemyStock <= enemyBlow)
			{
				if (player->playerID != 0)
				{
					player->GetMesh()->Play("Idle");
				}
				spriteRenderer.SpriteChange(sprites, selectCount, skComCount, 12);
				state = State::result;

				if (StageNo == 2)
				{
					StClearedE = true;
				}
				else if (StageNo == 3)
				{
					StClearedS = true;
				}
				else if (StageNo == 4)
				{
					StClearedW = true;
				}
				else if (StageNo == 5)
				{
					StClearedN = true;
				}
			}
		}
	}


	//タイトルへロード画面中.
	if (loadTimer > 0.0f)
	{
		loadTimer -= deltaTime;

		if (loadTimer <= 0.0f && player->pHP > 0)
		{
			if (state == State::result)
			{
				SceneStack::Instance().Replace(std::make_shared<TitleScene>());
				return;
			}
			else if (state == State::menu)
			{
				SceneStack::Instance().Replace(std::make_shared<TitleScene>());
				return;
			}
		}
		if (state == State::load && loadTimer <= 0.0f && player->pHP > 0)
		{
			if (stageChage == 1)
			{
				StageNo += 1;
				SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
				return;
			}
			else if (stageChage == 2)
			{
				StageNo += 2;
				SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
				return;
			}
			else if (stageChage == 3)
			{
				StageNo += 3;
				SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
				return;
			}
			else if (stageChage == 4)
			{
				StageNo += 4;
				SceneStack::Instance().Replace(std::make_shared<MainGameScene>());
				return;
			}
		}
	}
	fontRenderer.EndUpdate();
	textWindow.Update(deltaTime);
}

/**
*シーンを描画する.
*/
void MainGameScene::Render()
{

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
		for (auto e : lights)
		{
			if (auto p = std::dynamic_pointer_cast<DirectionalLightActor>(e))
			{
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
		RenderMesh(nullptr, Mesh::DrawType::shadow);
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
		glm::perspective(camera.fov, aspectRatio, camera.near, camera.far);
	meshBuffer.SetViewProjectionMatrix(matProj * matView);
	meshBuffer.SetCameraPosition(camera.position);
	meshBuffer.SetTime(window.Time());
	meshBuffer.BindShadowTexture(fboShadow->GetDepthTexture());

	const Collision::Frustum viewFrustum = Collision::CreateFrustum(camera);
	heightMap.UpdateGrassInstanceData(viewFrustum);
	RenderMesh(&viewFrustum, Mesh::DrawType::color);
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
		for (int i = 0; i < sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; ++i)
		{
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
		for (int i = sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; i >= 0; --i)
		{
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
		for (int i = sizeof(fboBloom) / sizeof(fboBloom[0]) - 1; i > 0; --i)
		{
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
		if (state == State::menu)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			//プレイヤー名.
			if (player->playerID == 0)
			{
				GameSceneFont(fontRenderer, 0);
			}
			else if (player->playerID == 1)
			{
				GameSceneFont(fontRenderer, 1);
			}
			else if (player->playerID == 2)
			{
				GameSceneFont(fontRenderer, 2);
			}
			else if (player->playerID == 3)
			{
				GameSceneFont(fontRenderer, 3);
			}

			//ステージNo表示.
			NumFont(fontRenderer, StageNo, 2, 0);

			//レベル表示.
			NumFont(fontRenderer, player->pLevel, 3, 1);

			//アビリティレベル.
			{
				wchar_t str[] = L"Ability:  ";
				int n = player->pAbility;
				for (int i = 0; i < 2; ++i)
				{
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0)
					{
						break;
					}
				}
				fontRenderer.AddString(glm::vec2(280, 150), str);
			}
			//体力表示.
			{
				wchar_t str[] = L"    ";
				int n = player->pHP;
				for (int i = 0; i < 4; ++i)
				{
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0)
					{
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
				for (int i = 0; i < 4; ++i)
				{
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0)
					{
						break;
					}
				}
				if (player->maxHP < 1000)
				{
					fontRenderer.AddString(glm::vec2(200, 75), str);
				}
				else if (player->maxHP >= 1000)
				{
					fontRenderer.AddString(glm::vec2(220, 75), str);
				}
			}

			//ＭＰ表示.
			{
				wchar_t str[] = L"   ";
				int n = player->pMP;
				for (int i = 0; i < 3; ++i)
				{
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0)
					{
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
				for (int i = 0; i < 3; ++i)
				{
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0)
					{
						break;
					}
				}
				if (player->maxMP < 100)
				{
					fontRenderer.AddString(glm::vec2(180, 25), str);
				}
				else if (player->maxMP >= 100)
				{
					fontRenderer.AddString(glm::vec2(200, 25), str);
				}
			}
			//レベルアップまであと.
			{
				wchar_t str[] = L"EX:LvUPまであと    ";
				int n = player->pExCount;
				for (int i = 0; i < 4; ++i)
				{
					str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
					n /= 10;
					if (n <= 0)
					{
						break;
					}
				}
				fontRenderer.AddString(glm::vec2(99, -10), str);
			}
		}

		//コマンド表.
		if (state == State::cMenu)
		{
			GameSceneFont(fontRenderer, 4);
			if (gamePadText)
			{
				GameSceneFont(fontRenderer, 5);
			}
			else
			{
				GameSceneFont(fontRenderer, 6);
			}
		}
		//スキル表.
		else if (state == State::sMenu)
		{
			GameSceneFont(fontRenderer, 7);
			//ソウルのスキル説明.
			if (player->playerID == 0)
			{
				GameSceneFont(fontRenderer, 8);
			}
			//ゴブリンのスキル説明.
			if (player->playerID == 1)
			{
				PlayerFont(fontRenderer, skComCount, player->pAbility, 0);
			}
			//ウィザードのスキル説明.
			if (player->playerID == 2)
			{
				PlayerFont(fontRenderer, skComCount, player->pAbility, 1);
			}
			//骸骨のスキル説明.
			if (player->playerID == 3)
			{
				PlayerFont(fontRenderer, skComCount, player->pAbility, 2);
			}
		}

		//メニュー画面常時.
		if (state == State::menu || state == State::cMenu || state == State::sMenu)
		{
			GameSceneFont(fontRenderer, 9);
		}

		//プレイ画面の情報.
		if (state == State::play || state == State::select)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 0.1f, 5));
			fontRenderer.Scale(glm::vec2(1));

			if (StageNo != 1) {
				//敵を倒した数..
				{
					wchar_t str[] = L"   ";
					int n = enemyBlow;
					for (int i = 0; i < 3; ++i)
					{
						str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
						n /= 10;
						if (n <= 0)
						{
							break;
						}
					}
					if (enemyBlow >= 100)
					{
						fontRenderer.AddString(glm::vec2(310, 340), str);
					}
					else if (enemyBlow >= 10)
					{
						fontRenderer.AddString(glm::vec2(320, 340), str);
					}
					else
					{
						fontRenderer.AddString(glm::vec2(330, 340), str);
					}
				}
				//敵の残数..
				{
					wchar_t str[] = L"/   ";
					int n = enemyStock;
					for (int i = 0; i < 3; ++i)
					{
						str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
						n /= 10;
						if (n <= 0)
						{
							break;
						}
					}
					fontRenderer.AddString(glm::vec2(360, 340), str);
				}

				if (comboTimer > 0.0f)
				{
					//コンボ数.
					{
						wchar_t str[] = L"   ";
						int n = combo;
						for (int i = 0; i < 3; ++i)
						{
							str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
							n /= 10;
							if (n <= 0)
							{
								break;
							}
						}
						fontRenderer.AddString(glm::vec2(340, 290), L"combo");
						fontRenderer.AddString(glm::vec2(300, 290), str);
					}
					//コンボバフ数.
					if (combo < 10)
					{
						{
							wchar_t str[] = L"  ";
							int n = combo;
							for (int i = 0; i < 2; ++i)
							{
								str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
								n /= 10;
								if (n <= 0)
								{
									break;
								}
							}
							fontRenderer.AddString(glm::vec2(430, 290), L"×1.");
							fontRenderer.AddString(glm::vec2(490, 290), str);
						}
					}
					else if (combo >= 10 && comboTimer > 0.0f)
					{
						fontRenderer.AddString(glm::vec2(340, 290), L"combo");
						fontRenderer.AddString(glm::vec2(430, 290), L"×2.0");
					}
				}
			}
		}

		//ロード画面中にランダムでゲームに役立つヒントが表示される.
		if (state == State::load)
		{
			StChangeFont(fontRenderer, sprites, stageChage, false, 0);
		}

		//ステージ移行の有無を選択時.
		if (state == State::select)
		{
			StChangeFont(fontRenderer, sprites, stageChage, eventFrag, 1);
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
*メッシュを描画する.
*
*@param drawType	描画するデータの種類.
*/
void MainGameScene::RenderMesh(const Collision::Frustum* pFrustum, Mesh::DrawType drawType)
{

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	Mesh::Draw(meshBuffer.GetFile("Terrain"), glm::mat4(1));

	if (pFrustum)
	{
		if (Collision::Test(*pFrustum, player->position))
		{
			player->Draw(drawType);
		}
		enemies[0].Draw(*pFrustum, drawType);
		enemies[1].Draw(*pFrustum, drawType);
		enemies[2].Draw(*pFrustum, drawType);
		enemies[3].Draw(*pFrustum, drawType);
		trees.Draw(*pFrustum, drawType);
		objects.Draw(*pFrustum, drawType);
		defencePoint.Draw(*pFrustum, drawType);
		walls.Draw(*pFrustum, drawType);
		effects.Draw(*pFrustum, drawType);
		items[0].Draw(*pFrustum, drawType);
		items[1].Draw(*pFrustum, drawType);
		items[2].Draw(*pFrustum, drawType);
		items[3].Draw(*pFrustum, drawType);
		warp[0].Draw(*pFrustum, drawType);
		warp[1].Draw(*pFrustum, drawType);
		warp[2].Draw(*pFrustum, drawType);
		warp[3].Draw(*pFrustum, drawType);
		bullet[0].Draw(*pFrustum, drawType);
		bullet[1].Draw(*pFrustum, drawType);
		bullet[2].Draw(*pFrustum, drawType);
	}
	else
	{
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
		bullet[1].Draw(drawType);
		bullet[2].Draw(drawType);
	}

	// 草を描画.
	Mesh::Draw(meshBuffer.GetFile("Res/grass.gltf"), glm::mat4(1), drawType,
		heightMap.GetGrassInstanceCount());

	/*if (drawType == Mesh::DrawType::color) {
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		Mesh::Draw(meshBuffer.GetFile("Water"), glm::mat4(1),drawType);
	}*/
}