/**
* @file StaticMesh.frag
*/
#version 430

layout(location=0) in vec3 inPosition;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inNormal;

out vec4 fragColor;

uniform sampler2D texColor;

struct AmbientLight {
 vec4 color;
};

struct DirectionalLight {
 vec4 color;
 vec4 direction;
};

struct PointLight {
 vec4 color;
 vec4 position;
};

struct SpotLight {
 vec4 color;
 vec4 dirAndCutOff;
 vec4 posAndInnerCutOff;
};

layout(std140) uniform LightUniformBlock
{
 AmbientLight ambientLight;
 DirectionalLight directionalLight;
 PointLight pointLight[100];
 SpotLight spotLight[100];
};

uniform isamplerBuffer texPointLightIndex; // �|�C���g���C�g�̃C���f�b�N�X�o�b�t�@
uniform isamplerBuffer texSpotLightIndex; // �X�|�b�g���C�g�̃C���f�b�N�X�o�b�t�@
const ivec2 mapSize = ivec2(200, 200); // �}�b�v�̑傫��

/**
* �X�v���C�g�p�t���O�����g�V�F�[�_�[.
*/
void main()
{

  vec3 normal = normalize(inNormal);
  vec3 lightColor = ambientLight.color.rgb;
  float power = max(dot(normal, -directionalLight.direction.xyz), 0.0);
  lightColor += directionalLight.color.rgb * power;

  // ���[���h���W����Y�������v�Z.
  int offset = int(inPosition.z) * (mapSize.x - 1) + int(inPosition.x);

  // �Y����offset���g���āA�|�C���g���C�g�̃C���f�b�N�X�o�b�t�@���烉�C�g�ԍ����擾.
  ivec4 pointLightIndex = texelFetch(texPointLightIndex, offset);
    for (int i = 0; i < 4; ++i) {
   int id = pointLightIndex[i];
   if (id < 0) { // �ԍ���0�����̏ꍇ�A����ȏド�C�g�ԍ��͓����Ă��Ȃ�.
      break;
    }
   vec3 lightVector = pointLight[id].position.xyz - inPosition;
   vec3 lightDir = normalize(lightVector);
   float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
   float intensity = 1.0 / (1.0 + dot(lightVector, lightVector));
   lightColor += pointLight[id].color.rgb * cosTheta * intensity;
 }

 // �Y����offset���g���āA�X�|�b�g���C�g�̃C���f�b�N�X�o�b�t�@���烉�C�g�ԍ����擾.
  ivec4 spotLightIndex = texelFetch(texSpotLightIndex, offset);
 for (int i = 0; i < 4; ++i) {
   int id = spotLightIndex[i];
   if (id < 0) { // �ԍ���0�����̏ꍇ�A����ȏド�C�g�ԍ��͓����Ă��Ȃ�.
      break;
    }
   vec3 lightVector = spotLight[id].posAndInnerCutOff.xyz - inPosition;
   vec3 lightDir = normalize(lightVector);
   float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
   float intensity = 1.0 / (1.0 + dot(lightVector, lightVector));
   float spotCosTheta = dot(lightDir, -spotLight[id].dirAndCutOff.xyz);
   float cutOff = smoothstep(spotLight[id].dirAndCutOff.w,
     spotLight[id].posAndInnerCutOff.w, spotCosTheta);
   lightColor += spotLight[id].color.rgb * cosTheta * intensity * cutOff;
 }


  fragColor = texture(texColor, inTexCoord);
  fragColor.rgb *= lightColor;

}