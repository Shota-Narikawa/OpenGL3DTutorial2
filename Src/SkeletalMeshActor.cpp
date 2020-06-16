/**
* @file SkeletalMeshActor.cpp
*/
#include "SkeletalMeshActor.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* コンストラクタ.
*
* @param m        表示するメッシュ.
* @param name     アクターの名前.
* @param health   耐久力.
* @param position 位置.
* @param rotation 回転.
* @param scale    拡大率.
*
* 指定されたメッシュ、名前、耐久力、位置、回転、拡大率によってアクターを初期化する.
*/
SkeletalMeshActor::SkeletalMeshActor(const Mesh::SkeletalMeshPtr& m,
	const std::string& name, int health, const glm::vec3& position,
	const glm::vec3& rotation, const glm::vec3& scale)
	: Actor(name, health, position, rotation, scale), mesh(m)
{
}

/**
* 描画情報の更新.
*
* @param deltaTime 経過時間.
*/
void SkeletalMeshActor::UpdateDrawData(float deltaTime)
{
	if (mesh) {
		const glm::mat4 matT = glm::translate(glm::mat4(1), position);
		const glm::mat4 matR_Y = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0));
		const glm::mat4 matR_ZY = glm::rotate(matR_Y, rotation.z, glm::vec3(0, 0, -1));
		const glm::mat4 matR_XZY = glm::rotate(matR_ZY, rotation.x, glm::vec3(1, 0, 0));
		const glm::mat4 matS = glm::scale(glm::mat4(1), scale);
		const glm::mat4 matModel = matT * matR_XZY * matS;
		mesh->Update(deltaTime, matModel, glm::vec4(1));
	}
}

/**
* アクターに影響するポイントライトのインデックスを設定する.
*
* @param v ポイントライトのインデックス配列.
*/
void SkeletalMeshActor::SetPointLightList(const std::vector<int>& v)
{
	pointLightCount = v.size();
	for (int i = 0; i < 8 && i < static_cast<int>(v.size()); ++i) {
		pointLightIndex[i] = v[i];
	}
}

/**
* アクターに影響するスポットライトのインデックスを設定する.
*
* @param v スポットライトのインデックス配列.
*/
void SkeletalMeshActor::SetSpotLightList(const std::vector<int>& v)
{
	spotLightCount = v.size();
	for (int i = 0; i < 8 && i < static_cast<int>(v.size()); ++i) {
		spotLightIndex[i] = v[i];

	}
}

/**
*描画.
*
*@param drawType	描画するデータの種類.
*/
void SkeletalMeshActor::Draw(Mesh::DrawType drawType)
{
	if (mesh) {
		mesh->Draw(drawType);
	}
	else if (sMesh) {
		const glm::mat4 matT = glm::translate(glm::mat4(1), position);
		const glm::mat4 matR_Y = glm::rotate(glm::mat4(1), rotation.y, glm::vec3(0, 1, 0));
		const glm::mat4 matR_ZY = glm::rotate(matR_Y, rotation.z, glm::vec3(0, 0, -1));
		const glm::mat4 matR_XZY = glm::rotate(matR_ZY, rotation.x, glm::vec3(1, 0, 0));
		const glm::mat4 matS = glm::scale(glm::mat4(1), scale);
		const glm::mat4 matModel = matT * matR_XZY * matS;

		if (drawType == Mesh::DrawType::color && !sMesh->materials.empty()) {
			const Shader::ProgramPtr p = sMesh->materials[0].program;
			if (p) {
				p->Use();
				p->SetPointLightIndex(pointLightCount, pointLightIndex);
				p->SetSpotLightIndex(spotLightCount, spotLightIndex);
			}
		}
		Mesh::Draw(sMesh, matModel, drawType);
	}
}
