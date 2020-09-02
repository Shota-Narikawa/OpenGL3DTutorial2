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

//テクスチャサンプラ.
uniform sampler2D texHeightMap;
uniform sampler2D texGrassHeightMap;
uniform usamplerBuffer texGrassInstanceData;

const ivec2 mapSize = ivec2(200, 200); // マップの大きさ.

const float heightScale = 20; // 高さの拡大率.
const float baseLevel = 0.5; // 高さ0とみなす値.

/**
*地面の高さを取得する.
*
*@param		pos		高さを取得する座標.
*
*@return	座標posの地面の高さ.
*/
float Height(vec3 pos)
{
  // 取得する位置がマップサイズを超えないようにする.
  // iposMin: 1x1mブロックの左下座標
  // iposMax: 1x1mブロックの右上座標
  ivec2 iposMin = ivec2(pos.xz);
  iposMin.y = (mapSize.y - 2) - iposMin.y;
  iposMin = max(ivec2(0), iposMin);
  ivec2 iposMax = min(mapSize - 1, iposMin + ivec2(1));

  // 左上頂点からの相対座標を計算.
  vec2 offset = fract(pos.xz);

  // 4点の高さから座標posの高さを計算.
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

  // 草を生やす位置を計算.
  uvec4 instanceData = texelFetch(texGrassInstanceData, gl_InstanceID);
  float x = float(instanceData.x);
  float z = float(instanceData.y);
  vec3 instancePosition = vPosition + vec3(x, 0, z) + vec3(0.5, 0, 0.5);

   //草丈マップ読み取り用テクスチャ座標を計算.
  // - 地形のZ方向をテクスチャ座標系に変換した後、マップサイズで除算して0～1に変換.
  vec2 invMapSize = vec2(1) / vec2(mapSize); // 除算を減らすための逆数計算.
  vec2 tcGrassHeightMap =
    vec2(instancePosition.x, float(mapSize.y) - instancePosition.z) * invMapSize;

  //草丈を設定.
  float grassScale = texture(texGrassHeightMap, tcGrassHeightMap).g;
  instancePosition.y *= grassScale;

  // 草が地面から生えるようにY座標を補正.
  instancePosition.y += Height(instancePosition);

  outPosition = vec3(matModel * vec4(instancePosition, 1.0));

  gl_Position = matMVP * (matModel * vec4(instancePosition, 1.0));
}