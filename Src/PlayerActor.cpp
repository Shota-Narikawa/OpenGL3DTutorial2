/**
* @file PlayerActor.cpp
*/
#include "PlayerActor.h"
#include "SkeletalMesh.h"
#include <glm/gtc/matrix_transform.hpp>

////�A�j���[�V����.
//mesh[0] = oni_small playerID == 1
//animation[0] = Run(0.933333sec)
//animation[1] = Hit(0.666667sec)
//animation[2] = Attack.Heavy(1.6sec)
//animation[3] = Down(1sec)
//animation[4] = Attack.Kick(1sec)
//animation[5] = Attack(1sec)
//animation[6] = GetUp(1.5sec)
//animation[7] = Idle.ScrachButt(2sec)
//animation[8] = Idle.LookAround(3.5sec)
//animation[9] = Wait(2sec)
//animation[10] = DownWithMove(1sec)
//animation[11] = Attack(1sec)
//
//mesh[0] = Bikuni playerID == 2
//animation[0] = Guard(0.4sec)
//animation[1] = Attack.Light(0.6sec)
//animation[2] = Attack.Heavy(1sec)
//animation[3] = Fall(0.4sec)
//animation[4] = Attack.Jump(0.466667sec)
//animation[5] = Idle(1sec)
//animation[6] = Rest(2sec)
//animation[7] = Hit(1.06667sec)
//animation[8] = Run(0.666667sec)
//animation[9] = Down(1sec)
//animation[10] = Jump(0.4sec)
//animation[11] = Idle(0.6sec)
//
//mesh[0] = Skeleton playerID == 3
//animation[0] = Wakigamae.FromIdle(0.666667sec)
//animation[1] = Attack(2sec)
//animation[2] = Hassou.ToIdle(0.666667sec)
//animation[3] = Attack.Horizontal(1.83333sec)
//animation[4] = Wakigamae.Idle(1sec)
//animation[5] = Wakigamae.Walk(0.666667sec)
//animation[6] = Wakigamae.ToIdle(0.666667sec)
//animation[7] = Hassou.Attack(1sec)
//animation[8] = Walk(1.4sec)
//animation[9] = Hassou.Walk(0.666667sec)
//animation[10] = Hassou.Idle(1sec)
//animation[11] = Wakigamae.Attack(1sec)
//animation[12] = Idle(3sec)
//animation[13] = Attack.Virtical(1.5sec)
//animation[14] = Hit(1sec)
//animation[15] = Down(1sec)
//animation[16] = Hassou.FromIdle(0.666667sec)

/**
*�R���X�g���N�^.
*
*@param hm		�v���C���[�̒��n����Ɏg�p���鍂���}�b�v.
*@param buffer	�v���C���[�̃��b�V���f�[�^�������b�V���o�b�t�@.
*@param pos		�v���C���[�̏������W.
*@param rot		�v���C���[�̏�������.
*/
PlayerActor::PlayerActor(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
	const glm::vec3& pos, const glm::vec3& rot)
	: SkeletalMeshActor(buffer.GetSkeletalMesh("Bikuni"), "Player", 10, pos, rot),
	heightMap(hm)
{
	colLocal = Collision::CreateSphere(glm::vec3(0, 0.7f, 0), 0.7f);
	state = State::idle;
}

/**
*�X�V.
*
*@param deltaTime	�o�ߎ���.
*/
void PlayerActor::Update(float deltaTime)
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

	//�X�L���C���^�[�o���Ǘ�.
	if (skillE == true) {
		skillTimerE -= deltaTime;
		if (skillTimerE <= -2.0f) {
			skillE = false;
		}
	}
	else {
		skillTimerE = 0.0f;
	}
	if (skillN == true) {
		skillTimerN -= deltaTime;
		if (skillTimerN <= -3.0f) {
			skillN = false;
		}
	}
	else {
		skillTimerN = 0.0f;
	}

	if (skillW == true) {
		skillTimerW -= deltaTime;
		if (skillTimerW <= -4.0f) {
			skillW = false;
		}
	}
	else {
		skillTimerW = 0.0f;
	}

	//�A�j���[�V�����̍X�V.
	if (playerID == 1 || playerID == 2 || playerID == 3) {
		switch (state) {
		case State::run:
			if (isInAir) {
				GetMesh()->Play("Jump");
				state = State::jump;
			}
			else {
				const float horizontalSpeed = velocity.x * velocity.x + velocity.z * velocity.z;
				if (horizontalSpeed == 0) {
					if (playerID == 1) {
						GetMesh()->Play("Idle.ScrachButt");
					}
					else if (playerID == 2) {
						GetMesh()->Play("Idle");
					}
					else if (playerID == 3) {
						GetMesh()->Play("Wakigamae.Idle");
					}
					state = State::idle;
				}
			}
			break;

		case State::idle:
			if (isInAir) {
				GetMesh()->Play("Jump");
				state = State::jump;
			}
			else {
				const float horizontalSpeed = velocity.x * velocity.x + velocity.z * velocity.z;
				if (horizontalSpeed != 0) {
					if (playerID == 1 || playerID == 2) {
						GetMesh()->Play("Run");
					}
					else if (playerID == 3) {
						GetMesh()->Play("Wakigamae.Walk");
					}
					state = State::run;
				}
			}
			break;

		case State::jump:
			if (!isInAir) {
				if (playerID == 1) {
					GetMesh()->Play("Idle.ScrachButt");
				}
				else if (playerID == 2) {
					GetMesh()->Play("Idle");
				}
				else if (playerID == 3) {
					GetMesh()->Play("Wakigamae.Idle");
				}
				state = State::idle;
			}
			break;

		case State::shot:
			playerBulletTimer += deltaTime;
			if (playerBulletTimer > 0.01f && playerBulletTimer < 0.2f) {
				if (!BulletCollision) {
					static const float radian = 1.0f;
					const glm::vec3 front = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1.5f, 1);
					BulletCollision = std::make_shared<Actor>("PlayerBulletCollision", 5, position + front + glm::vec3(0, 1, 0), glm::vec3(0), glm::vec3(radian));
					BulletCollision->colLocal = Collision::CreateSphere(glm::vec3(0), radian);

				}
			}
			else {
				BulletCollision.reset();

			}
			if (GetMesh()->IsFinished()) {
				BulletCollision.reset();
				GetMesh()->Play("Idle");
				state = State::idle;

			}
			break;

		case State::attack:
			isAttack = true;
			attackTimer += deltaTime;
			if (attackTimer > 0.7f && attackTimer < 1.0f) {
				if (!attackCollision) {
					static const float radian = 1.0f;
					const glm::vec3 front = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1.5f, 1);
					attackCollision = std::make_shared<Actor>("PlayerAttackCollision", 5, position + front + glm::vec3(0, 1, 0), glm::vec3(0), glm::vec3(radian));
					attackCollision->colLocal = Collision::CreateSphere(glm::vec3(0), radian);

				}
			}
			else {
				attackCollision.reset();

			}
			if (GetMesh()->IsFinished()) {
				isAttack = false;
				attackCollision.reset();
				if (playerID == 1) {
					GetMesh()->Play("Idle.ScrachButt");
				}
				else if (playerID == 2) {
					GetMesh()->Play("Idle");
				}
				else if (playerID == 3) {
					GetMesh()->Play("Wakigamae.Idle");
				}
				state = State::idle;
			}
			break;
		}
	}
}

	/**
	*���͂���������.
	*/
		void PlayerActor::ProcessInput()
		{
		const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
		CheckRun(gamepad);
		CheckJump(gamepad);
		CheckAttack(gamepad);
		CheckShot(gamepad);
		}

		/**
		*�ړ��������������.
		*
		*@param gamepad	�Q�[���p�b�h����.
		*/
			void PlayerActor::CheckRun(const GamePad& gamepad)
			{
				//�󒆂ɂ���Ƃ��͈ړ��ł��Ȃ�.
				if (isInAir) {
					return;
				}
				//�����L�[����ړ��������v�Z.
				const glm::vec3 front(0, 0, -2);
				const glm::vec3 left(-2, 0, 0);

				glm::vec3 move(0);

				if (!isAttack) {
					if (gamepad.buttons & GamePad::DPAD_UP) {
						move += front;

					}
					else if (gamepad.buttons & GamePad::DPAD_DOWN) {
						move -= front;

					}
					if (gamepad.buttons & GamePad::DPAD_LEFT) {
						move += left;

					}
					else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
						move -= left;
					}
				}
				//�ړ����s���Ă�����A�ړ������ɉ����Č����Ƒ��x���X�V.
				if (glm::dot(move, move)) {
					//�������X�V.
						move = glm::normalize(move);
					rotation.y = std::atan2(-move.z, move.x) + glm::radians(90.0f);
					
						//���̂ɏ���Ă��Ȃ��Ƃ��͒n�`�̌��z���l�����Ĉړ������𒲐�����.
						if (!boardingActor) {
						//�ړ������̒n�`�̌��z(gradient)���v�Z.
						const float minGradient = glm::radians(-60.0f); // �������Ƃ̂ł�����z�̍ŏ��l.
						const float maxGradient = glm::radians(60.0f); // �������Ƃ̂ł�����z�̍ő�l.
						const float frontY =
							heightMap->Height(position + move * 0.05f) - position.y - 0.01f;
						const float gradient =
							glm::clamp(std::atan2(frontY, 0.05f), minGradient, maxGradient);
						
							//�n�`�ɉ����悤�Ɉړ����x��ݒ�.
							const glm::vec3 axis = glm::normalize(glm::cross(move, glm::vec3(0, 1, 0)));
						move = glm::rotate(glm::mat4(1), gradient, axis) * glm::vec4(move, 1.0f);
					}
					velocity = move * moveSpeed;
				}
				else {
					//�ړ����Ă��Ȃ��̂ő��x��0�ɂ���.
					velocity = glm::vec3(0);
				}
			}

			/**
			*�W�����v�������������.
			*
			*@param gamepad		�Q�[���p�b�h����.
			*/
				void PlayerActor::CheckJump(const GamePad& gamepad)
				{
				if (isInAir) {
					return;
					
				}
				if (gamepad.buttonDown & GamePad::R || gamepad.buttons & GamePad::R2 || gamepad.buttons & GamePad::L2) {
					Jump();
					if (playerID == 1) {
						Audio::Engine::Instance().Prepare("Res/Audio/Enemy.mp3")->Play();
					}else if (playerID == 2) {
						Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack1.mp3")->Play();
					}
					else if (playerID == 3) {
						Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack1.mp3")->Play();
					}
				}
			}


	/**
	*�W�����v������.
	*/
		void PlayerActor::Jump()
		{
		velocity.y = 5.0f;
		if (pJump >= 2) {
			velocity.y = 7.0f;
		}

		boardingActor.reset();
		isInAir = true;
		}

		void PlayerActor::StatusUp() {

			if (enemyBlow <= 0) {

				pLevel = 1;
				pJump = 1;
				pSpeed = 1;
			}
			else if (enemyBlow >= 3) {

				pLevel = 2;
				pJump = 2;
				pSpeed = 2;
			}
		}

		/**
		* �U���������������.
		*
		* @param gamepad �Q�[���p�b�h����.
		*/
		void PlayerActor::CheckAttack(const GamePad& gamepad)
			 {
			if (isInAir) {
				return;
				
			}
				if (playerID == 1) {
					if (gamepad.buttonDown & GamePad::A) {
						Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
						Audio::Engine::Instance().Prepare("Res/Audio/Enemy.mp3")->Play();
						GetMesh()->Play("Attack", false);
						attackTimer = 0;
						state = State::attack;
					}
					if (pAbility >= 1 && pMP > 0 && gamepad.buttons & GamePad::X) {

						/*GetMesh()->Play("Attack.Heavy", false);
						attackTimer = 0;
						state = State::attack;*/
						/*pMP -= 1;*/
					}
				}
				else if (playerID == 2) {
					if (pAbility >= 1 && gamepad.buttonDown & GamePad::A) {

						Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
						Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack2.mp3")->Play();
						GetMesh()->Play("Attack.Light", false);
						attackTimer = 0;
						state = State::attack;
					}
					if (skillE == false && pAbility >= 2 && pMP > 0 && gamepad.buttonDown & GamePad::X) {

						Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
						Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack3.mp3")->Play();
						GetMesh()->Play("Attack.Heavy", false);
						attackTimer = 0;
						skillE = true;
						state = State::attack;
						pMP -= 5;
					}
					if (skillN == false && pAbility >= 3 && pMP > 0 && gamepad.buttonDown & GamePad::Y) {

						Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
						Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack3.mp3")->Play();
						GetMesh()->Play("Attack.Heavy", false);
						attackTimer = 0;
						skillN = true;
						state = State::attack;
						pMP -= 5;
					}
					if (skillW == false && pAbility >= 4 && pMP > 0 && gamepad.buttonDown & GamePad::B) {

						Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
						Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack3.mp3")->Play();
						GetMesh()->Play("Attack.Jump", false);
						attackTimer = 0;
						skillW = true;
						state = State::attack;
						pMP -= 10;
					}
				}
				else if (playerID == 3) {
					if (pAbility >= 1 && gamepad.buttonDown & GamePad::A) {

						Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
						Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack2.mp3")->Play();
						GetMesh()->Play("Attack", false);
						attackTimer = 0;
						state = State::attack;
					}
					if (skillE == false && pAbility >= 2 && pMP > 0 && gamepad.buttonDown & GamePad::X) {

						Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
						Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack3.mp3")->Play();
						GetMesh()->Play("Attack", false);
						attackTimer = 0;
						skillE = true;
						state = State::attack;
						pMP -= 5;
					}
					if (skillE == true && skillTimerE <= -1.9f /*&& skillTimerE >= -1.1f && pAbility >= 2 && pMP > 0 && gamepad.buttonDown & GamePad::X*/) {
						Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
						Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack3.mp3")->Play();
						GetMesh()->Play("Wakigamae.Attack", false);
						attackTimer = 0;
						state = State::attack;
					}
					if (skillN == false && pAbility >= 3 && pMP > 0 && gamepad.buttonDown & GamePad::Y) {

						Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
						Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack3.mp3")->Play();
						GetMesh()->Play("Attack.Virtical", false);
						attackTimer = 0;
						skillN = true;
						state = State::attack;
						pMP -= 10;
					}
					if (skillW == false && pAbility >= 4 && pMP > 0 && gamepad.buttonDown & GamePad::B) {

						Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
						Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack3.mp3")->Play();
						GetMesh()->Play("Attack.Horizontal", false);
						attackTimer = 0;
						skillW = true;
						state = State::attack;
						pMP -= 10;
					}
				}
		}

		/**
		* �U���������������.
		*
		* @param gamepad �Q�[���p�b�h����.
		*/
		void PlayerActor::CheckShot(const GamePad& gamepad)
		{
			if (isInAir) {
				return;
			}
			//const Mesh::FilePtr meshWarpGate = meshBuffer.GetFile("Res/Gate.gltf");
			//PlayerActorPtr player;
			//if (playerBulletTimer <= 0) {
			//	if (gamepad.buttonDown & GamePad::H) {
			//		StaticMeshActorPtr GateE = std::make_shared<StaticMeshActor>(
			//			meshWarpGate, "GateE", 100, player->position, glm::vec3(0, 0, 0));
			//		const float speed = 10.0f;	//�e�̈ړ����x(m/�b).
			//		const int x[] = { 0,10,-10,20,-20 };
			//		const glm::mat4 matRotY =
			//			glm::rotate(glm::mat4(1), player->rotation.y, glm::vec3(0, 1, 0));
			//		GateE->scale = glm::vec3(1, 1, 1);
			//		GateE->colLocal = Collision::CreateCapsule(
			//			glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.3f);
			//		GateE->velocity = matRotY * glm::vec4(0, 0, -speed, 1);

			//		Audio::Engine::Instance().Prepare("Res/Audio/PlayerShot.xwm")->Play();

			//	}
			//}
		}

		/**
		*�v���C���[������Ă��镨�̂�ݒ肷��.
		*
		*@param p	����Ă��镨��.
		*/
		void PlayerActor:: SetBoardingActor(ActorPtr p)
			{
			boardingActor = p;
			if (p) {
				isInAir = false;
			}
		}

		/**
		*�Փ˃n���h��.
		*/
			void PlayerActor::OnHit(const ActorPtr& b, const glm::vec3& p)
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