/**
* @file EnemyActor.cpp
*/
#include "EnemyActor.h"
#include "SkeletalMesh.h"
#include "PlayerActor.h"
#include <glm/gtc/matrix_transform.hpp>

/**
*�R���X�g���N�^.
*
*@param hm		�v���C���[�̒��n����Ɏg�p���鍂���}�b�v.
*@param buffer	�v���C���[�̃��b�V���f�[�^�������b�V���o�b�t�@.
*@param pos		�v���C���[�̏������W.
*@param rot		�v���C���[�̏�������.
*/
EnemyActor::EnemyActor(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
	const glm::vec3& pos, const glm::vec3& rot)
	: SkeletalMeshActor(buffer.GetSkeletalMesh("oni_small"), "Enemy", 10, pos, rot),
	heightMap(hm)
{
	colLocal = Collision::CreateSphere(glm::vec3(0, 0.7f, 0), 0.7f);
}

/**
*�X�V.
*
*@param deltaTime	�o�ߎ���.
*/
void EnemyActor::Update(float deltaTime)
{
	//���W�̍X�V.
	SkeletalMeshActor::Update(deltaTime);
	if (attackCollision) {
		attackCollision->Update(deltaTime);
	}
	//�ڒn����.
	static const float gravity = 9.8f;
	const float groundHeight = heightMap->Height(position);
	if (position.y <= groundHeight) {
		position.y = groundHeight;
		velocity.y = 0;
		isInAir = false;
	}
	else if (position.y > groundHeight) {
		//����Ă��镨�̂��痣�ꂽ��󒆔���ɂ���.
		if (boardingActor) {
			Collision::Shape col = colWorld;
			col.s.r += 0.1f; // �Փ˔���������傫������.
			glm::vec3 pa, pb;
			if (!Collision::TestShapeShape(col, boardingActor->colWorld, &pa, &pb)) {
				boardingActor.reset();
			}
		}
		//��������.
		const bool isFloating = position.y > groundHeight + 0.1f; // �n�ʂ��畂���Ă��邩.
		if (!isInAir && isFloating && !boardingActor) {
			isInAir = true;
		}
		//�d�͂�������.
		if (isInAir) {
			velocity.y -= gravity * deltaTime;
		}
	}
	
	//�G���ǂ������Ă���.
	Mesh::SkeletalMeshPtr mesh = GetMesh();

	//���S���������.
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

	//�^�[�Q�b�g(�v���C���[)�̕����𒲂ׂ�.
	const glm::vec3 v = target->position - position;
	const glm::vec3 vTarget = glm::normalize(v);
	float radian = std::atan2(-vTarget.z, vTarget.x) - glm::radians(90.0f);

	if (radian <= 0) {
		radian += glm::radians(360.0f);
	}
	const glm::vec3 vEnemyFront = glm::rotate(
		glm::mat4(1), rotation.y + glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1);

	//�^�[�Q�b�g�����ʂɂ��Ȃ�������A���ʂɂƂ炦��悤�ȍ��E�ɐ���.
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
		velocity = glm::vec3(0);	//����ł���̂ł����ړ����Ȃ�.
	}
	//�\���ɐڋ߂��Ă��Ȃ���Έړ�����B�ڋ߂��Ă���΍U������.
	if (glm::length(v) > 1.5f) {
		velocity = vEnemyFront * moveSpeed;
		if (mesh->GetAnimation() != "Run") {
			mesh->Play("Run");
		}
	}
	else {
		velocity = glm::vec3(0);	//�ڋ߂��Ă���̂ł����ړ����Ȃ�.
		if (mesh->GetAnimation() != "Wait") {
			if ((mesh->GetAnimation() != "Attack" && mesh->GetAnimation() != "Hit") ||
				mesh->IsFinished()) {
				mesh->Play("Wait");
			}
		}						//����I�ɍU����ԂɂȂ�.
		if (isAttacking) {
			isAttacking = false;
			attackingTimer = 3.0f;	//���̍U���͂T�b��.
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
//*���͂���������.
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
//*�ړ��������������.
//*
//*@param gamepad	�Q�[���p�b�h����.
//*/
//void EnemyActor::CheckRun(const GamePad& gamepad)
//{
//	//�󒆂ɂ���Ƃ��͈ړ��ł��Ȃ�.
//	if (isInAir) {
//		return;
//	}
//	//�����L�[����ړ��������v�Z.
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
//	//�ړ����s���Ă�����A�ړ������ɉ����Č����Ƒ��x���X�V.
//	if (glm::dot(move, move)) {
//		//�������X�V.
//		move = glm::normalize(move);
//		rotation.y = std::atan2(-move.z, move.x) + glm::radians(90.0f);
//
//		//���̂ɏ���Ă��Ȃ��Ƃ��͒n�`�̌��z���l�����Ĉړ������𒲐�����.
//		if (!boardingActor) {
//			//�ړ������̒n�`�̌��z(gradient)���v�Z.
//			const float minGradient = glm::radians(-60.0f); // �������Ƃ̂ł�����z�̍ŏ��l.
//			const float maxGradient = glm::radians(60.0f); // �������Ƃ̂ł�����z�̍ő�l.
//			const float frontY =
//				heightMap->Height(position + move * 0.05f) - position.y - 0.01f;
//			const float gradient =
//				glm::clamp(std::atan2(frontY, 0.05f), minGradient, maxGradient);
//
//			//�n�`�ɉ����悤�Ɉړ����x��ݒ�.
//			const glm::vec3 axis = glm::normalize(glm::cross(move, glm::vec3(0, 1, 0)));
//			move = glm::rotate(glm::mat4(1), gradient, axis) * glm::vec4(move, 1.0f);
//		}
//		velocity = move * moveSpeed;
//	}
//	else {
//		//�ړ����Ă��Ȃ��̂ő��x��0�ɂ���.
//		velocity = glm::vec3(0);
//	}
//}
//
///**
//*�W�����v�������������.
//*
//*@param gamepad		�Q�[���p�b�h����.
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
//*�W�����v������.
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
//* �U���������������.
//*
//* @param gamepad �Q�[���p�b�h����.
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
//* �U���������������.
//*
//* @param gamepad �Q�[���p�b�h����.
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
//	//		const float speed = 10.0f;	//�e�̈ړ����x(m/�b).
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
*�v���C���[������Ă��镨�̂�ݒ肷��.
*
*@param p	����Ă��镨��.
*/
void EnemyActor::SetBoardingActor(ActorPtr p)
{
	boardingActor = p;
	if (p) {
		isInAir = false;
	}
}

/**
*�Փ˃n���h��.
*/
void EnemyActor::OnHit(const ActorPtr& b, const glm::vec3& p)
{
	const glm::vec3 v = colWorld.s.center - p;
	//�Փˈʒu�Ƃ̋������߂����Ȃ������ׂ�.
	if (dot(v, v) > FLT_EPSILON) {
		//this��b�ɏd�Ȃ�Ȃ��ʒu�܂ňړ�.
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
		//�ړ���������(�������߂�����ꍇ�̗�O����).
		const float deltaTime = static_cast<float>(GLFWEW::Window::Instance().DeltaTime());
		const glm::vec3 deltaVelocity = velocity * deltaTime;
		position -= deltaVelocity;
		colWorld.s.center -= deltaVelocity;

	}
	SetBoardingActor(b);
}