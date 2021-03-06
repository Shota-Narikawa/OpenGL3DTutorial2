/**
* @file PlayerActor.cpp
*/
#include "PlayerActor.h"
#include "SkeletalMesh.h"
#include <glm/gtc/matrix_transform.hpp>

////アニメーション.
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
//animation[9] = Idle(2sec)
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
*コンストラクタ.
*
*@param hm		プレイヤーの着地判定に使用する高さマップ.
*@param buffer	プレイヤーのメッシュデータを持つメッシュバッファ.
*@param pos		プレイヤーの初期座標.
*@param rot		プレイヤーの初期方向.
*/
PlayerActor::PlayerActor(const Terrain::HeightMap* hm, const Mesh::Buffer& buffer,
	const glm::vec3& pos, const glm::vec3& rot)
	: SkeletalMeshActor(buffer.GetSkeletalMesh("Bikuni"), "Player", 10, pos, rot),
	heightMap(hm)
{
	colLocal = Collision::CreateCapsule(glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
	state = State::idle;
}

/**
*更新.
*
*@param deltaTime	経過時間.
*/
void PlayerActor::Update(float deltaTime)
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
			// 落下判定用の形状は、プレイヤーの衝突判定形状に合わせて調整すること.
			const Collision::Shape col = Collision::CreateCapsule(
			position + glm::vec3(0, 0.4f, 0), position + glm::vec3(0, 1, 0), 0.25f);
			const Collision::Result result =
				Collision::TestShapeShape(col, boardingActor->colWorld);
			if (!result.isHit) {
				boardingActor.reset();
			}
			else{
				//衝突面の法線が真上から30度の範囲になければ落下.
					const float theta = glm::dot(result.nb, glm::vec3(0, 1, 0));
				if (theta < glm::cos(glm::radians(30.0f))) {
					boardingActor.reset();
				}
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

	//スキルインターバル管理.
	if (skillE == true) {
		skillTimerE -= deltaTime;
		if (skillTimerE <= -2.0f) {
			skillE = false;
			dashFrag = false;
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

	if (pep)
	{
		pep->Position(position);
	}

	//アニメーションの更新.
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
			if (attackTimerA >= 0) {
				attackTimerA += deltaTime;
			}
			if (attackTimerB >= 0) {
				attackTimerB += deltaTime;
			}
			if (attackTimerC >= 0) {
				attackTimerC += deltaTime;
			}
			//通常攻撃の範囲.
			if (attackTimerA > 0.4f && attackTimerA < 0.9f) {
				if (!attackCollision) {
					static const float radian = 1.0f;
					const glm::vec3 front = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 1.5f, 1);
					attackCollision = std::make_shared<Actor>("PlayerAttackCollision", 5, position + front + glm::vec3(0, 1, 0), glm::vec3(0), glm::vec3(radian));
					attackCollision->colLocal = Collision::CreateSphere(glm::vec3(0), radian);

				}
			}
			//前方の範囲攻撃（骸骨専用）.
			else if (attackTimerB > 0.7f && attackTimerB < 0.9f) {
				if (!attackCollision) {
					static const float radian = 1.0f;
					const glm::vec3 front = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 5.0f, 1);
					attackCollision = std::make_shared<Actor>("PlayerAttackCollision", 5, position + front + glm::vec3(0, 1, 0), glm::vec3(0), glm::vec3(3.0f));
					attackCollision->colLocal = Collision::CreateSphere(glm::vec3(0), 3.0f);

				}
			}
			//周囲の範囲攻撃（骸骨専用）.
			else if (attackTimerC > 1.0f && attackTimerC < 1.2f) {
				if (!attackCollision) {
					static const float radian = 1.0f;
					const glm::vec3 front = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 0, 1);
					attackCollision = std::make_shared<Actor>("PlayerAttackCollision", 5, position + front + glm::vec3(0, 1, 0), glm::vec3(0), glm::vec3(5.0f));
					attackCollision->colLocal = Collision::CreateSphere(glm::vec3(0), 5.0f);

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
*入力を処理する.
*/
void PlayerActor::ProcessInput(const Camera& camera)
{
	const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad();
	CheckRun(gamepad, camera);
	CheckJump(gamepad);
	CheckAttack(gamepad);
	CheckShot(gamepad);
}

/**
*移動操作を処理する.
*
*@param gamepad	ゲームパッド入力.
*/
void PlayerActor::CheckRun(const GamePad& gamepad, const Camera& camera)
{
	//空中にいるときは移動できない.
	if (isInAir) {
		return;
	}

	////方向キーから移動方向を計算.
	glm::vec3 move(0);
	glm::vec3 front = glm::normalize(camera.target - camera.position);
	front.y = 0;

	// 前後方向に移動
	if (!isAttack) {
		if (gamepad.buttons & GamePad::DPAD_UP) {
				move = front;
		}
		else if (gamepad.buttons & GamePad::DPAD_DOWN) {
			move = -front;
		}
	}

	//移動が行われていたら、移動方向に応じて向きと速度を更新.
	if (glm::dot(move, move)) {
		//向きを更新.
		move = glm::normalize(move);
		rotation.y = std::atan2(-move.z, move.x) + glm::radians(90.0f);

		//物体に乗っていないときは地形の勾配を考慮して移動方向を調整する.
		if (!boardingActor) {
			//移動方向の地形の勾配(gradient)を計算.
			const float minGradient = glm::radians(-60.0f); // 沿うことのできる勾配の最小値.
			const float maxGradient = glm::radians(60.0f); // 沿うことのできる勾配の最大値.
			const float frontY =
				heightMap->Height(position + move * 0.05f) - position.y - 0.01f;
			const float gradient =
				glm::clamp(std::atan2(frontY, 0.05f), minGradient, maxGradient);

			//地形に沿うように移動速度を設定.
			const glm::vec3 axis = glm::normalize(glm::cross(move, glm::vec3(0, 1, 0)));
			move = glm::rotate(glm::mat4(1), gradient, axis) * glm::vec4(move, 1.0f);
		}
		velocity = move * moveSpeed;
	}
	else {
		//移動していないので速度を0にする.
		velocity = glm::vec3(0);
	}
}

/**
*ジャンプ操作を処理する.
*
*@param gamepad		ゲームパッド入力.
*/
void PlayerActor::CheckJump(const GamePad& gamepad)
{
	if (isInAir) {
		return;

	}
	/*if (gamepad.buttonDown & GamePad::R || gamepad.buttons & GamePad::R2 || gamepad.buttons & GamePad::L2) {
		Jump();
		if (playerID == 1) {
			Audio::Engine::Instance().Prepare("Res/Audio/Enemy.mp3")->Play();
		}
		else if (playerID == 2) {
			Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack1.mp3")->Play();
		}
		else if (playerID == 3) {
			Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack1.mp3")->Play();
		}
	}*/
}

/**
*衝突ハンドラ.
*
*@param		b		衝突相手のアクター.
*@param		result	衝突結果.
*/
void PlayerActor::OnHit(const ActorPtr& b, const Collision::Result& result)
{
	//貫通しない位置まで衝突面の法線方向に移動させる.
	const float d = glm::dot(result.nb, result.pb - result.pa);
	const glm::vec3 v = result.nb * (d + 0.01f);
	colWorld.c.seg.a += v;
	colWorld.c.seg.b += v;
	position += v;
	if (!isInAir && !boardingActor) {
		const float newY = heightMap->Height(position);
		colWorld.c.seg.a.y += newY - position.y;
		colWorld.c.seg.b.y += newY - position.y;
		position.y = newY;
	}
	//衝突面の法線が真上から30度の範囲にあれば乗ることができる(角度は要調整).
		const float theta = glm::dot(result.nb, glm::vec3(0, 1, 0));
	if (theta >= cos(glm::radians(30.0f))) {
		SetBoardingActor(b);
	}
}

/**
*ジャンプさせる.
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
* 攻撃操作を処理する.
*
* @param gamepad ゲームパッド入力.
*/
void PlayerActor::CheckAttack(const GamePad& gamepad)
{
	if (isInAir) {
		return;
	}
	//ゴブリン.
	if (playerID == 1) {
		if (gamepad.buttonDown & GamePad::A) {
			Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
			Audio::Engine::Instance().Prepare("Res/Audio/Enemy.mp3")->Play();
			GetMesh()->Play("Attack", false);
			attackTimerA = 0;
			state = State::attack;
		}
		if (skillE == false && pAbility >= 2 && pMP > 0 && gamepad.buttonDown & GamePad::X) {

			skillE = true;
			pMP -= 5;
		}
	}
	//ウィザード.
	else if (playerID == 2) {
		if (pAbility >= 1 && gamepad.buttonDown & GamePad::A) {

			Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
			Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack2.mp3")->Play();
			GetMesh()->Play("Attack.Light", false);
			attackTimerA = 0;
			state = State::attack;
		}
		if (skillE == false && pAbility >= 2 && pMP > 0 && gamepad.buttonDown & GamePad::X) {

			Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
			Audio::Engine::Instance().Prepare("Res/Audio/game_wizard-attack3.mp3")->Play();
			GetMesh()->Play("Attack.Heavy", false);
			attackTimerA = 0;
			skillE = true;
			state = State::attack;
			pMP -= 5;
		}
		if (skillN == false && pAbility >= 3 && pMP > 0 && gamepad.buttonDown & GamePad::Y) {

			Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
			Audio::Engine::Instance().Prepare("Res/Audio/_game_wizard-special1.mp3")->Play();
			GetMesh()->Play("Attack.Heavy", false);
			skillN = true;
			state = State::attack;
			pMP -= 5;
		}
		if (skillW == false && pAbility >= 4 && pMP > 0 && gamepad.buttonDown & GamePad::B) {

			Audio::Engine::Instance().Prepare("Res/Audio/SmallAttack.mp3")->Play();
			Audio::Engine::Instance().Prepare("Res/Audio/_game_wizard-special2.mp3")->Play();
			GetMesh()->Play("Attack.Jump", false);
			skillW = true;
			state = State::attack;
			pMP -= 10;
		}
	}
	//骸骨.
	else if (playerID == 3) {
		if (pAbility >= 1 && gamepad.buttonDown & GamePad::A) {

			Audio::Engine::Instance().Prepare("Res/Audio/sword-gesture1.mp3")->Play();
			Audio::Engine::Instance().Prepare("Res/Audio/_game_swordman-attack1.mp3")->Play();
			GetMesh()->Play("Attack", false);
			attackTimerA = 0;
			state = State::attack;
		}
		if (skillE == false && pAbility >= 2 && pMP > 0 && gamepad.buttonDown & GamePad::X) {

			Audio::Engine::Instance().Prepare("Res/Audio/sword-gesture1.mp3")->Play();
			Audio::Engine::Instance().Prepare("Res/Audio/_game_swordman-special1.mp3")->Play();
			GetMesh()->Play("Attack", false);
			attackTimerA = 0;
			skillE = true;
			state = State::attack;
			pMP -= 5;
		}
		if (skillE == true && skillTimerE <= -1.9f /*&& skillTimerE >= -1.1f && pAbility >= 2 && pMP > 0 && gamepad.buttonDown & GamePad::X*/) {
			Audio::Engine::Instance().Prepare("Res/Audio/sword-gesture1.mp3")->Play();
			GetMesh()->Play("Wakigamae.Attack", false);
			attackTimerA = 0;
			state = State::attack;
		}
		if (skillN == false && pAbility >= 3 && pMP > 0 && gamepad.buttonDown & GamePad::Y) {
			Audio::Engine::Instance().Prepare("Res/Audio/_game_swordman-attack3.mp3")->Play();
			GetMesh()->Play("Attack.Virtical", false);
			attackTimerB = 0;
			skillN = true;
			state = State::attack;
			pMP -= 10;
		}
		if (skillW == false && pAbility >= 4 && pMP > 0 && gamepad.buttonDown & GamePad::B) {
			Audio::Engine::Instance().Prepare("Res/Audio/_game_swordman-special3.mp3")->Play();
			GetMesh()->Play("Attack.Horizontal", false);
			attackTimerC = 0;
			skillW = true;
			state = State::attack;
			pMP -= 10;
		}
	}
}

/**
* 攻撃操作を処理する.
*
* @param gamepad ゲームパッド入力.
*/
void PlayerActor::CheckShot(const GamePad& gamepad)
{
	if (isInAir) {
		return;
	}
}

/**
*プレイヤーが乗っている物体を設定する.
*
*@param p	乗っている物体.
*/
void PlayerActor::SetBoardingActor(ActorPtr p)
{
	boardingActor = p;
	if (p) {
		isInAir = false;
	}
}

/**
*衝突ハンドラ.
*/
void PlayerActor::OnHit(const ActorPtr& b, const glm::vec3& p)
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