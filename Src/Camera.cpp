/**
*@file	Camera.cpp
*/
#include "Camera.h"

/**
*�J�����̃p�����[�^���X�V����.
*
*@param		matView		�X�V�Ɏg�p����r���[�s��.
*/
void Camera::Update(const glm::mat4& matView)
{
	const glm::vec4 pos = matView * glm::vec4(target, 1);
	focalPlane = pos.z * -1000.0f;

	const float imageDistance = sensorSize * 0.5f / glm::tan(fov * 0.5f);
	focalLength = 1.0f / ((1.0f / focalPlane) + (1.0f / imageDistance));

	aperture = focalLength / fNumber;
}