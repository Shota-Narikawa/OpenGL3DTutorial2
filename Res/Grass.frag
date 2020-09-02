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

uniform isamplerBuffer texPointLightIndex; // ポイントライトのインデックスバッファ
uniform isamplerBuffer texSpotLightIndex; // スポットライトのインデックスバッファ
const ivec2 mapSize = ivec2(200, 200); // マップの大きさ

/**
* スプライト用フラグメントシェーダー.
*/
void main()
{

  vec3 normal = normalize(inNormal);
  vec3 lightColor = ambientLight.color.rgb;
  float power = max(dot(normal, -directionalLight.direction.xyz), 0.0);
  lightColor += directionalLight.color.rgb * power;

  // ワールド座標から添え字を計算.
  int offset = int(inPosition.z) * (mapSize.x - 1) + int(inPosition.x);

  // 添え字offsetを使って、ポイントライトのインデックスバッファからライト番号を取得.
  ivec4 pointLightIndex = texelFetch(texPointLightIndex, offset);
    for (int i = 0; i < 4; ++i) {
   int id = pointLightIndex[i];
   if (id < 0) { // 番号が0未満の場合、それ以上ライト番号は入っていない.
      break;
    }
   vec3 lightVector = pointLight[id].position.xyz - inPosition;
   vec3 lightDir = normalize(lightVector);
   float cosTheta = clamp(dot(normal, lightDir), 0.0, 1.0);
   float intensity = 1.0 / (1.0 + dot(lightVector, lightVector));
   lightColor += pointLight[id].color.rgb * cosTheta * intensity;
 }

 // 添え字offsetを使って、スポットライトのインデックスバッファからライト番号を取得.
  ivec4 spotLightIndex = texelFetch(texSpotLightIndex, offset);
 for (int i = 0; i < 4; ++i) {
   int id = spotLightIndex[i];
   if (id < 0) { // 番号が0未満の場合、それ以上ライト番号は入っていない.
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