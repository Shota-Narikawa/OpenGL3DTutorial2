/**
* @file EnemyActor.cpp
*/
#include "EnemyActor.h"
#include "MainGameScene.h"
#include "SkeletalMesh.h"
#include "PlayerActor.h"
#include <random>
#include <glm/gtc/matrix_transform.hpp>

/**
*コンストラクタ.
*
*@param hm		プレイヤーの着地判定に使用する高さマップ.
*@param buffer	プレイヤーのメッシュデータを持つメッシュバッファ.
*@param pos		プレイヤーの初期座標.
*@param rot		プレイヤーの初期方向.
*/
EnemyActor::EnemyActor(MainGameScene* mainGameScene, const Mesh::SkeletalMeshPtr& m,
	const std::string& name, int health, const glm::vec3& position,
	const glm::vec3& rotation, const glm::vec3& scale)
	: SkeletalMeshActor(m, name, health, position, rotation, scale), mainGameScene(mainGameScene)
{
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

/**
*敵の出現.
*/
void EnemyActor::EnemySpawn(ActorList enemy[], int count,float deltaTime,float timer, int spawn,int target, int n)
{
	if (deltaTime >= timer)
	{
		if (spawn >= 0)
		{
			for (size_t i = 0; i < count; i++)
			{
				position.x = std::uniform_real_distribution<float>(60, 100)(mainGameScene->rand);
				position.z = std::uniform_real_distribution<float>(80, 120)(mainGameScene->rand);
				glm::vec3 rotation(0);
				rotation.y = std::uniform_real_distribution<float>(0, 3.14f * 2.0f)(mainGameScene->rand);
				position.y = heightMap->Height(position);

				const Mesh::SkeletalMeshPtr mesh = meshBuffer.GetSkeletalMesh("oni_small");
				SkeletalMeshActorPtr p = std::make_shared<SkeletalMeshActor>(
					mesh, "Kooni", 15, position, rotation);
				p->colLocal = Collision::CreateCapsule(
					glm::vec3(0, 0.5f, 0), glm::vec3(0, 1, 0), 0.5f);
				enemy[n].Add(p);
				target = std::uniform_int_distribution<>(0, 2)(rand);
				EnemyTarget(enemy[n],0);
			}
			timer = 0.0f;
			spawn -= 3;
		}
	}
}

/**
*敵のターゲット情報を決める.
*
*@param a		ActorListの種類.
*/
void EnemyActor::EnemyTarget(ActorList& a, int enemyID)
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