/**
* @file EnemyActor.cpp
*/
#include "EnemyActor.h"
#include "SkeletalMesh.h"
#include "PlayerActor.h"
#include <glm/gtc/matrix_transform.hpp>

/**
*コンストラクタ.
*
*@param hm		プレイヤーの着地判定に使用する高さマップ.
*@param buffer	プレイヤーのメッシュデータを持つメッシュバッファ.
*@param pos		プレイヤーの初期座標.
*@param rot		プレイヤーの初期方向.
*/
EnemyActor::EnemyActor(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
	const glm::vec3& pos, const glm::vec3& rot)
	: SkeletalMeshActor(buffer.GetSkeletalMesh("oni_small"), "Enemy", 10, pos, rot),
	heightMap(hm)
{
	colLocal = Collision::CreateSphere(glm::vec3(0, 0.7f, 0), 0.7f);
}

/**
*更新.
*
*@param deltaTime	経過時間.
*/
void EnemyActor::Update(float deltaTime)
{
	//座標の更新.
	SkeletalMeshActor::Update(deltaTime);
	if (attackCollision) {
		attackCollision->Update(deltaTime);
	}
	//接地判定.
	static const float gravity = 9.8f;
	const float groundHeight = heightMap->Height(position);
	if (position.y <= groundHeight) {
		position.y = groundHeight;
		velocity.y = 0;
		isInAir = false;
	}
	else if (position.y > groundHeight) {
		//乗っている物体から離れたら空中判定にする.
		if (boardingActor) {
			Collision::Shape col = colWorld;
			col.s.r += 0.1f; // 衝突判定を少し大きくする.
			glm::vec3 pa, pb;
			if (!Collision::TestShapeShape(col, boardingActor->colWorld, &pa, &pb)) {
				boardingActor.reset();
			}
		}
		//落下判定.
		const bool isFloating = position.y > groundHeight + 0.1f; // 地面から浮いているか.
		if (!isInAir && isFloating && !boardingActor) {
			isInAir = true;
		}
		//重力を加える.
		if (isInAir) {
			velocity.y -= gravity * deltaTime;
		}
	}
	
	//敵が追いかけてくる.
	Mesh::SkeletalMeshPtr mesh = GetMesh();

	//死亡したら消す.
	if (mesh->GetAnimation() == "Down") {
		if (mesh->IsFinished()) {
			health = 0;
			enemyBlow += 1;
			player->pExPoint -= 100;
			player->pExCount -= 500;
		}
		return;
	}

	const float moveSpeed = baseSpeed * 2.0f;
	const float rotationSpeed = baseSpeed * glm::radians(60.0f);
	const float frontRange = glm::radians(15.0f);

	//ターゲット(プレイヤー)の方向を調べる.
	const glm::vec3 v = target->position - position;
	const glm::vec3 vTarget = glm::normalize(v);
	float radian = std::atan2(-vTarget.z, vTarget.x) - glm::radians(90.0f);

	if (radian <= 0) {
		radian += glm::radians(360.0f);
	}
	const glm::vec3 vEnemyFront = glm::rotate(
		glm::mat4(1), rotation.y + glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);

	//ターゲットが正面にいなかったら、正面にとらえるような左右に旋回.
	if (std::abs(radian - rotation.y) > frontRange) {
		const glm::vec3 vRotDir = glm::cross(vEnemyFront, vTarget);
		if (vRotDir.y >= 0) {
			rotation.y += rotationSpeed * deltaTime;
			if (rotation.y >= glm::radians(360.0f)) {
				rotation.y -= glm::radians(360.0f);
			}
		}
		else {
			rotation.y -= rotationSpeed * deltaTime;
			if (rotation.y < 0) {
				rotation.y += glm::radians(360.0f);
			}
		}
	}
	if (health <= 0) {
		velocity = glm::vec3(0);	//死んでいるのでもう移動しない.
	}
	//十分に接近していなければ移動する。接近していれば攻撃する.
	if (glm::length(v) > 1.5f) {
		velocity = vEnemyFront * moveSpeed;
		if (mesh->GetAnimation() != "Run") {
			mesh->Play("Run");
		}
	}
	else {
		velocity = glm::vec3(0);	//接近しているのでもう移動しない.
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
			}
		}
	}
}

///**
//*入力を処理する.
//*/
//void EnemyActor::ProcessInput()
//{
//	const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
//	CheckRun(gamepad);
//	CheckJump(gamepad);
//	CheckAttack(gamepad);
//	CheckShot(gamepad);
//}
//
///**
//*移動操作を処理する.
//*
//*@param gamepad	ゲームパッド入力.
//*/
//void EnemyActor::CheckRun(const GamePad& gamepad)
//{
//	//空中にいるときは移動できない.
//	if (isInAir) {
//		return;
//	}
//	//方向キーから移動方向を計算.
//	const glm::vec3 front(0, 0, -2);
//	const glm::vec3 left(-2, 0, 0);
//	glm::vec3 move(0);
//	if (pLevel >= 2) {
//		glm::vec3 move(3);
//	}
//	if (gamepad.buttons & GamePad::DPAD_UP) {
//		move += front;
//
//	}
//	else if (gamepad.buttons & GamePad::DPAD_DOWN) {
//		move -= front;
//
//	}
//	if (gamepad.buttons & GamePad::DPAD_LEFT) {
//		move += left;
//
//	}
//	else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
//		move -= left;
//	}
//	//移動が行われていたら、移動方向に応じて向きと速度を更新.
//	if (glm::dot(move, move)) {
//		//向きを更新.
//		move = glm::normalize(move);
//		rotation.y = std::atan2(-move.z, move.x) + glm::radians(90.0f);
//
//		//物体に乗っていないときは地形の勾配を考慮して移動方向を調整する.
//		if (!boardingActor) {
//			//移動方向の地形の勾配(gradient)を計算.
//			const float minGradient = glm::radians(-60.0f); // 沿うことのできる勾配の最小値.
//			const float maxGradient = glm::radians(60.0f); // 沿うことのできる勾配の最大値.
//			const float frontY =
//				heightMap->Height(position + move * 0.05f) - position.y - 0.01f;
//			const float gradient =
//				glm::clamp(std::atan2(frontY, 0.05f), minGradient, maxGradient);
//
//			//地形に沿うように移動速度を設定.
//			const glm::vec3 axis = glm::normalize(glm::cross(move, glm::vec3(0, 1, 0)));
//			move = glm::rotate(glm::mat4(1), gradient, axis) * glm::vec4(move, 1.0f);
//		}
//		velocity = move * moveSpeed;
//	}
//	else {
//		//移動していないので速度を0にする.
//		velocity = glm::vec3(0);
//	}
//}
//
///**
//*ジャンプ操作を処理する.
//*
//*@param gamepad		ゲームパッド入力.
//*/
//void EnemyActor::CheckJump(const GamePad& gamepad)
//{
//	if (isInAir) {
//		return;
//
//	}
//	if (gamepad.buttonDown & GamePad::R || gamepad.buttons & GamePad::R2 || gamepad.buttons & GamePad::L2) {
//		Jump();
//		if (playerID == 1) {
//			Audio::Engine::Instance().Prepare("Res/Audio/Enemy.mp3")->Play();
//		}
//		else if (playerID == 2) {
//			Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack1.mp3")->Play();
//		}
//	}
//}
//
//
///**
//*ジャンプさせる.
//*/
//void EnemyActor::Jump()
//{
//	velocity.y = 5.0f;
//	if (pJump >= 2) {
//		velocity.y = 7.0f;
//	}
//
//	boardingActor.reset();
//	isInAir = true;
//}
//
///**
//* 攻撃操作を処理する.
//*
//* @param gamepad ゲームパッド入力.
//*/
//void EnemyActor::CheckAttack(const GamePad& gamepad)
//{
//	if (isInAir) {
//		return;
//
//	}
//	if (playerID == 1) {
//		if (gamepad.buttonDown & GamePad::A) {
//			Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
//			Audio::Engine::Instance().Prepare("Res/Audio/Enemy.mp3")->Play();
//			GetMesh()->Play("Attack", false);
//			attackTimer = 0;
//			state = State::attack;
//		}
//	}
//	else if (playerID == 2) {
//		if (pAbility >= 1 && gamepad.buttonDown & GamePad::A) {
//
//			Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
//			Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack2.mp3")->Play();
//			GetMesh()->Play("Attack.Light", false);
//			attackTimer = 0;
//			state = State::attack;
//		}
//		if (pAbility >= 2 && pMP > 0 && gamepad.buttonDown & GamePad::X) {
//
//			Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
//			Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack3.mp3")->Play();
//			GetMesh()->Play("Attack.Heavy", false);
//			attackTimer = 0;
//			state = State::attack;
//			pMP -= 10;
//		}
//	}
//}
//
///**
//* 攻撃操作を処理する.
//*
//* @param gamepad ゲームパッド入力.
//*/
//void EnemyActor::CheckShot(const GamePad& gamepad)
//{
//	if (isInAir) {
//		return;
//	}
//	//const Mesh::FilePtr meshWarpGate = meshBuffer.GetFile("Res/Gate.gltf");
//	//PlayerActorPtr player;
//	//if (playerBulletTimer <= 0) {
//	//	if (gamepad.buttonDown & GamePad::H) {
//	//		StaticMeshActorPtr GateE = std::make_shared<StaticMeshActor>(
//	//			meshWarpGate, "GateE", 100, player->position, glm::vec3(0, 0, 0));
//	//		const float speed = 10.0f;	//弾の移動速度(m/秒).
//	//		const int x[] = { 0,10,-10,20,-20 };
//	//		const glm::mat4 matRotY =
//	//			glm::rotate(glm::mat4(1), player->rotation.y, glm::vec3(0, 1, 0));
//	//		GateE->scale = glm::vec3(1, 1, 1);
//	//		GateE->colLocal = Collision::CreateCapsule(
//	//			glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.3f);
//	//		GateE->velocity = matRotY * glm::vec4(0, 0, -speed, 1);
//
//	//		Audio::Engine::Instance().Prepare("Res/Audio/PlayerShot.xwm")->Play();
//
//	//	}
//	//}
//}

/**
*プレイヤーが乗っている物体を設定する.
*
*@param p	乗っている物体.
*/
void EnemyActor::SetBoardingActor(ActorPtr p)
{
	boardingActor = p;
	if (p) {
		isInAir = false;
	}
}

/**
*衝突ハンドラ.
*/
void EnemyActor::OnHit(const ActorPtr& b, const glm::vec3& p)
{
	const glm::vec3 v = colWorld.s.center - p;
	//衝突位置との距離が近すぎないか調べる.
	if (dot(v, v) > FLT_EPSILON) {
		//thisをbに重ならない位置まで移動.
		const glm::vec3 vn = normalize(v);
		float radiusSum = colWorld.s.r;
		switch (b->colWorld.type) {
		case Collision::Shape::Type::sphere: radiusSum += b->colWorld.s.r; break;
		case Collision::Shape::Type::capsule: radiusSum += b->colWorld.c.r; break;

		}
		const float distance = radiusSum - glm::length(v) + 0.01f;
		position += vn * distance;
		colWorld.s.center += vn * distance;

	}
	else {
		//移動を取り消す(距離が近すぎる場合の例外処理).
		const float deltaTime = static_cast<float>(GLFWEW::Window::Instance().DeltaTime());
		const glm::vec3 deltaVelocity = velocity * deltaTime;
		position -= deltaVelocity;
		colWorld.s.center -= deltaVelocity;

	}
	SetBoardingActor(b);
}