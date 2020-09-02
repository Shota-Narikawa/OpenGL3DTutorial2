/**
* @file StaticMesh.vert
*/
#version 430

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTexCoord;
layout(location=2) in vec3 vNormal;

layout(location=0) out vec3 outPosition;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outNormal;

uniform mat4 matMVP;
uniform mat4 matModel;
uniform mat4 matShadow;

//�e�N�X�`���T���v��.
uniform sampler2D texHeightMap;
uniform sampler2D texGrassHeightMap;
uniform usamplerBuffer texGrassInstanceData;

const ivec2 mapSize = ivec2(200, 200); // �}�b�v�̑傫��.

const float heightScale = 20; // �����̊g�嗦.
const float baseLevel = 0.5; // ����0�Ƃ݂Ȃ��l.

/**
*�n�ʂ̍������擾����.
*
*@param		pos		�������擾������W.
*
*@return	���Wpos�̒n�ʂ̍���.
*/
float Height(vec3 pos)
{
  // �擾����ʒu���}�b�v�T�C�Y�𒴂��Ȃ��悤�ɂ���.
  // iposMin: 1x1m�u���b�N�̍������W
  // iposMax: 1x1m�u���b�N�̉E����W
  ivec2 iposMin = ivec2(pos.xz);
  iposMin.y = (mapSize.y - 2) - iposMin.y;
  iposMin = max(ivec2(0), iposMin);
  ivec2 iposMax = min(mapSize - 1, iposMin + ivec2(1));

  // ���㒸�_����̑��΍��W���v�Z.
  vec2 offset = fract(pos.xz);

  // 4�_�̍���������Wpos�̍������v�Z.
  // h0 -- h1
  // |  /  |
  // h2 -- h3
  float h1 = texelFetch(texHeightMap, iposMax, 0).r;
  float h2 = texelFetch(texHeightMap, iposMin, 0).r;
  float height;
  if (offset.x + offset.y < 1.0) {
    float h0 = texelFetch(texHeightMap, ivec2(iposMin.x, iposMax.y), 0).r;
    height = h0 + (h1 - h0) * offset.x + (h2 - h0) * offset.y;
  } else {
    float h3 = texelFetch(texHeightMap, ivec2(iposMax.x, iposMin.y), 0).r;
    height = h3 + (h2 - h3) * (1.0 - offset.x) + (h1 - h3) * (1.0 - offset.y);
  }
  return (height - baseLevel) * heightScale;
}

/**
* Terrain vertex shader.
*/
void main()
{
  mat3 matNormal = transpose(inverse(mat3(matModel)));

  outTexCoord = vTexCoord;
  outNormal = normalize(matNormal * vNormal);

  // ���𐶂₷�ʒu���v�Z.
  uvec4 instanceData = texelFetch(texGrassInstanceData, gl_InstanceID);
  float x = float(instanceData.x);
  float z = float(instanceData.y);
  vec3 instancePosition = vPosition + vec3(x, 0, z) + vec3(0.5, 0, 0.5);

   //����}�b�v�ǂݎ��p�e�N�X�`�����W���v�Z.
  // - �n�`��Z�������e�N�X�`�����W�n�ɕϊ�������A�}�b�v�T�C�Y�ŏ��Z����0�`1�ɕϊ�.
  vec2 invMapSize = vec2(1) / vec2(mapSize); // ���Z�����炷���߂̋t���v�Z.
  vec2 tcGrassHeightMap =
    vec2(instancePosition.x, float(mapSize.y) - instancePosition.z) * invMapSize;

  //�����ݒ�.
  float grassScale = texture(texGrassHeightMap, tcGrassHeightMap).g;
  instancePosition.y *= grassScale;

  // �����n�ʂ��琶����悤��Y���W��␳.
  instancePosition.y += Height(instancePosition);

  outPosition = vec3(matModel * vec4(instancePosition, 1.0));

  gl_Position = matMVP * (matModel * vec4(instancePosition, 1.0));
}