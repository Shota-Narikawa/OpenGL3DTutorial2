/**
* @file SkeletalMeshActor.cpp
*/
#include "SkeletalMeshActor.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* �R���X�g���N�^.
*
* @param m        �\�����郁�b�V��.
* @param name     �A�N�^�[�̖��O.
* @param health   �ϋv��.
* @param position �ʒu.
* @param rotation ��].
* @param scale    �g�嗦.
*
* �w�肳�ꂽ���b�V���A���O�A�ϋv�́A�ʒu�A��]�A�g�嗦�ɂ���ăA�N�^�[������������.
*/
SkeletalMeshActor::SkeletalMeshActor(const Mesh::SkeletalMeshPtr& m,
	const std::string& name, int health, const glm::vec3& position,
	const glm::vec3& rotation, const glm::vec3& scale)
	: Actor(name, health, position, rotation, scale), mesh(m)
{
}

/**
* �`����̍X�V.
*
* @param deltaTime �o�ߎ���.
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
* �A�N�^�[�ɉe������|�C���g���C�g�̃C���f�b�N�X��ݒ肷��.
*
* @param v �|�C���g���C�g�̃C���f�b�N�X�z��.
*/
void SkeletalMeshActor::SetPointLightList(const std::vector<int>& v)
{
	pointLightCount = v.size();
	for (int i = 0; i < 8 && i < static_cast<int>(v.size()); ++i) {
		pointLightIndex[i] = v[i];
	}
}

/**
* �A�N�^�[�ɉe������X�|�b�g���C�g�̃C���f�b�N�X��ݒ肷��.
*
* @param v �X�|�b�g���C�g�̃C���f�b�N�X�z��.
*/
void SkeletalMeshActor::SetSpotLightList(const std::vector<int>& v)
{
	spotLightCount = v.size();
	for (int i = 0; i < 8 && i < static_cast<int>(v.size()); ++i) {
		spotLightIndex[i] = v[i];

	}
}

/**
*�`��.
*
*@param drawType	�`�悷��f�[�^�̎��.
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
