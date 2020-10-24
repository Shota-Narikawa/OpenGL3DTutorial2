/**
* @file Particle.cpp
*/
#include "Particle.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <algorithm>
#include <random>
#include <numeric>
#include <iostream>
#include <time.h>

namespace /* unnamed */ {

	///パーティクル用の頂点データ型.
	struct Vertex {
		glm::vec3 center;   // 中心の座標.
		glm::vec4 color;    // 色.
		glm::vec2 texCoord; // テクスチャ座標.
		glm::vec2 offset;   // 中心からの距離.
	};

	///パーティクル用の乱数エンジン.
	std::mt19937 randomEngine(static_cast<int>(time(nullptr)));

	/**
	*int型の乱数を生成する.
	*
	*@param min 生成する乱数の最小値.
	*@param max 生成する乱数の最大値.
	*/
	int RandomInt(int min, int max)
	{
		return std::uniform_int_distribution<>(min, max)(randomEngine);
	}

	/**
	*float型の乱数を生成する.
	*
	*@param min		生成する乱数の最小値.
	*@param max		生成する乱数の最大値.
	*/
	float RandomFloat(float min, float max)
	{
		return std::uniform_real_distribution<float>(min, max)(randomEngine);
	}

} // unnamed namespace

/**
*コンストラクタ.
*
*@param		pp		パーティクルのパラメーター.
*@param		pos		パーティクルの初期座標.
*@param		r		パーティクルのテクスチャ表示範囲を示す矩形.
*/
Particle::Particle(const ParticleParameter& pp, const glm::vec3& pos, const Rect& r) :
	rect(r),
	position(pos),
	lifetime(pp.lifetime),
	velocity(pp.velocity),
	acceleration(pp.acceleration),
	scale(pp.scale),
	rotation(pp.rotation),
	color(pp.color)
{
}

/**
*パーティクルの状態を更新する.
*
*@param		deltaTime	前回の更新からの経過時間(秒).
*/
void Particle::Update(float deltaTime)
{
	lifetime -= deltaTime;
	velocity += acceleration * deltaTime;
	position += velocity * deltaTime;
}

/**
*コンストラクタ.
*
*@param		ep	エミッターの初期化パラメータ.
*@param		pp	パーティクルの初期化パラメータ.
*/
ParticleEmitter::ParticleEmitter(
	const ParticleEmitterParameter& ep, const ParticleParameter& pp) :
	ep(ep),
	pp(pp),
	interval(1.0f / ep.emissionsPerSecond)
{
	texture = Texture::Image2D::Create(ep.imagePath.c_str());
}

/**
*パーティクルを追加する.
*/
void ParticleEmitter::AddParticle()
{
	//放出位置と速度を求めるための行列を計算.
	const float rx = std::sqrt(RandomFloat(0, 1)); // X軸方向の放出位置を0〜1の範囲で表した値.
	const float ry = RandomFloat(0, glm::two_pi<float>()); // 0〜360度の範囲でY軸回転させる値.
	const glm::mat4 matRY = glm::rotate(glm::mat4(1), ry, glm::vec3(0, 1, 0));

	//X -> Z -> Yの順でエミッターの回転を適用.
	const glm::mat4 matRot = glm::rotate(glm::rotate(glm::rotate(glm::mat4(1),
		ep.rotation.y, glm::vec3(0, 1, 0)),
		ep.rotation.z, glm::vec3(0, 0, -1)),
		ep.rotation.x, glm::vec3(1, 0, 0));

	//ベロシティだけ変更したパーティクルの初期化用パラメーターを作成.
	ParticleParameter tmpPP = pp;
	const glm::mat4 matAngle = glm::rotate(matRY, rx * ep.angle, glm::vec3(0, 0, -1));
	tmpPP.velocity = matRot * matAngle * glm::vec4(pp.velocity, 1);

	//テクスチャの表示範囲を作成.
	Rect rect;
	rect.origin = glm::vec2(0);
	if (ep.tiles.x > 1) {
		const int tx = RandomInt(0, ep.tiles.x - 1);
		rect.origin.x = static_cast<float>(tx) / static_cast<float>(ep.tiles.x);
	}
	if (ep.tiles.y > 1) {
		const int ty = RandomInt(0, ep.tiles.y - 1);
		rect.origin.y = static_cast<float>(ty) / static_cast<float>(ep.tiles.y);
	}
	rect.size = glm::vec2(1) / glm::vec2(ep.tiles);

	const glm::vec3 offset = matRot * matRY * glm::vec4(rx * ep.radius, 0, 0, 1);
	Particle p(tmpPP, ep.position + offset, rect);
	particles.push_back(p);
}

/**
*エミッターの管理下にあるパーティクルの状態を更新する.
*
*@param		deltaTime  前回の更新からの経過時間(秒).
*/
void ParticleEmitter::Update(float deltaTime)
{
	//タイマー更新.
	timer += deltaTime;
	if (timer >= ep.duration) {
		if (ep.loop) {
			timer -= ep.duration;
			emissionTimer -= ep.duration;
		}
		else {
			timer = ep.duration;
		}
	}
	//パーティクルを放出.
	for (; timer - emissionTimer >= interval; emissionTimer += interval) {
		AddParticle();
	}

	//パーティクルを更新.
	for (auto& e : particles) {
		e.acceleration.y -= ep.gravity * deltaTime;
		e.Update(deltaTime);
	}
	//死亡したパーティクルを削除.
	particles.remove_if([](const Particle& p) { return p.IsDead(); });
}

/**
*エミッターの管理下にあるパーティクルを描画する.
*/
void ParticleEmitter::Draw()
{
	if (count) {
		glBlendFunc(ep.srcFactor, ep.dstFactor);
		glBindTexture(GL_TEXTURE_2D, texture->Get());
		glDrawElementsBaseVertex(GL_TRIANGLES, count, GL_UNSIGNED_SHORT,
			reinterpret_cast<const GLvoid*>(0), baseVertex);
	}
}

/**
*パーティクル・システムを初期化する.
*
*@param maxParticleCount	表示可能なパーティクルの最大数.
*/
bool ParticleSystem::Init(size_t maxParticleCount)
{
	if (!vbo.Create(GL_ARRAY_BUFFER, sizeof(Vertex) * maxParticleCount * 4,
		nullptr, GL_STREAM_DRAW)) {
		std::cerr << "[エラー] パーティクル・システムの初期化に失敗.\n";
		return false;
	}

	static const GLushort baseIndices[] = { 0, 1, 2, 2, 3, 0 };
	std::vector<short> indices;
	indices.reserve(4000);
	for (int baseIndex = 0; baseIndex <= std::numeric_limits<GLushort>::max() - 3;
		baseIndex += 4) {
		for (auto i : baseIndices) {
			indices.push_back(static_cast<GLushort>(baseIndex + i));
		}
	}
	if (!ibo.Create(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(),
		indices.data(), GL_STATIC_DRAW)) {
		std::cerr << "[エラー] パーティクル・システムの初期化に失敗.\n";
		return false;
	}
	if (!vao.Create(vbo.Id(), ibo.Id())) {
		std::cerr << "[エラー] パーティクル・システムの初期化に失敗.\n";
		return false;
	}
	vao.Bind();
	vao.VertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, center));
	vao.VertexAttribPointer(
		1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, color));
	vao.VertexAttribPointer(
		2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, texCoord));
	vao.VertexAttribPointer(
		3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, offset));
	vao.Unbind();
	const GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		std::cerr << "[エラー] パーティクル・システムの初期化に失敗(" <<
			std::hex << error << ").\n";
		return false;
	}
	program = Shader::Program::Create("Res/Particle.vert", "Res/Sprite.frag");
	if (!program) {
		std::cerr << "[エラー] パーティクル・システムの初期化に失敗.\n";
		return false;
	}
	return true;
}

/**
*エミッターを追加する.
*
*@param		ep	エミッターの初期化パラメータ.
*@param		pp	パーティクルの初期化パラメータ.
*
*@return	追加したエミッター.
*/
ParticleEmitterPtr ParticleSystem::Add(
	const ParticleEmitterParameter& ep, const ParticleParameter& pp)
{
	ParticleEmitterPtr p = std::make_shared<ParticleEmitter>(ep, pp);
	emitters.push_back(p);
	return p;
}

/**
*指定されたIDを持つエミッターを検索する.
*
*@param id		検索するID.
*
*@retval nullptr以外	引数idと一致するIDを持つエミッター.
*@retval nullptr		一致するエミッターは存在しない.
*/
ParticleEmitterPtr ParticleSystem::Find(int id) const
{
	auto itr = std::find_if(emitters.begin(), emitters.end(),
		[id](const ParticleEmitterPtr& p) { return p->Id() == id; });
	if (itr != emitters.end()) {
		return *itr;
	}
	return nullptr;
}

/**
*指定されたエミッターを削除する.
*/
void ParticleSystem::Remove(const ParticleEmitterPtr& p)
{
	emitters.remove(p);
}

/**
*すべてのエミッターを削除する.
*/
void ParticleSystem::Clear()
{
	emitters.clear();
}

/**
+*パーティクルの状態を更新する.
*
*@param deltaTime	前回の更新からの経過時間(秒).
*/
void ParticleSystem::Update(float deltaTime)
{
	//エミッターの更新と削除.
	for (auto& e : emitters) {
		e->Update(deltaTime);
	}
	emitters.remove_if([](const ParticleEmitterPtr& e) { return e->IsDead(); });

	//すべてのエミッターのパーティクルの頂点データを作成.
	std::vector<Vertex> vertices;
	vertices.reserve(10000);
	for (auto& e : emitters) {
		e->baseVertex = vertices.size();
		e->count = 0;
		for (auto& particle : e->particles) {
			//座標変換行列を作成.
			const glm::mat4 matR =
				glm::rotate(glm::mat4(1), particle.rotation, glm::vec3(0, 0, 1));
			const glm::mat4 matS = glm::scale(glm::mat4(1), glm::vec3(particle.scale, 1));
			const glm::mat4 transform = matR * matS;

			const Rect& rect = particle.rect; // 読みやすくするための参照を定義.

			Vertex v[4];

			v[0].center = particle.position;
			v[0].color = particle.color;
			v[0].texCoord = rect.origin;
			v[0].offset = transform * glm::vec4(-1, -1, 0, 1);

			v[1].center = particle.position;
			v[1].color = particle.color;
			v[1].texCoord = glm::vec2(rect.origin.x + rect.size.x, rect.origin.y);
			v[1].offset = transform * glm::vec4(1, -1, 0, 1);

			v[2].center = particle.position;
			v[2].color = particle.color;
			v[2].texCoord = rect.origin + rect.size;
			v[2].offset = transform * glm::vec4(1, 1, 0, 1);

			v[3].center = particle.position;
			v[3].color = particle.color;
			v[3].texCoord = glm::vec2(rect.origin.x, rect.origin.y + rect.size.y);
			v[3].offset = transform * glm::vec4(-1, 1, 0, 1);

			vertices.insert(vertices.end(), v, v + 4);
			e->count += 6;
		}
	}
	vbo.BufferSubData(0, vertices.size() * sizeof(Vertex), vertices.data());
}

/**
*パーティクルを描画する.
*
*@param  matProj 描画に使用するプロジェクション行列.
*@param  matView 描画に使用するビュー行列.
*/
void ParticleSystem::Draw(const glm::mat4& matProj, const glm::mat4& matView)
{
	vao.Bind();

	//描画設定.
	glEnable(GL_DEPTH_TEST);
	glDepthMask(false); // 深度バッファへの書き込みを止める.
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//シェーダーにパラメーターを設定.
	program->Use();
	program->SetInverseViewRotationMatrix(matView);
	program->SetViewProjectionMatrix(matProj * matView);
	glActiveTexture(GL_TEXTURE0);

	//エミッターをカメラから遠い順に並べる.
	struct A {
		float z;
		ParticleEmitterPtr p;
	};
	std::vector<A> sortedList;
	sortedList.reserve(emitters.size());
	for (auto& e : emitters) {
		const glm::vec3 pos = matView * glm::vec4(e->Position(), 1);
		sortedList.push_back({ pos.z, e });
	}
	std::sort(sortedList.begin(), sortedList.end(),
		[](const A& a, const A& b) { return a.z < b.z; });

	//エミッターを描画.
	for (auto& e : sortedList) {
		e.p->Draw();
	}
	//描画設定の後始末.
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glDepthMask(true); // 深度バッファへの書き込みを許可.

	vao.Unbind();
}

/*
*砂埃のパーティクル.
*/
void ParticleSystem::Dust(glm::vec3 pos)
{
	ParticleEmitterParameter ep;
	ep.imagePath = "Res/DiskParticle.tga";
	ep.tiles = glm::ivec2(1, 1);
	ep.position = pos;
	ep.position.y += 0.0f;
	ep.emissionsPerSecond = 10.0f;
	ep.duration = 0.1f;
	ep.dstFactor = GL_ONE; // 加算合成.
	ep.gravity = 5.0f;
	ep.angle = glm::radians(0.0f);//
	ep.loop = false;
	ParticleParameter pp;
	pp.acceleration = glm::vec3(0);//
	pp.lifetime = 0.2f;
	pp.velocity = glm::vec3(0, 2, 0);
	pp.scale = glm::vec2(0.3f);
	pp.color = glm::vec4(0.1f, 0.1f, 0.1f, 1.0f);
	Add(ep, pp);
}

/*
*魂のパーティクル.
*/
void ParticleSystem::Soul(glm::vec3 pos)
{
	ParticleEmitterParameter ep;
	ep.imagePath = "Res/FireParticle.tga";
	ep.tiles = glm::ivec2(2, 2);
	ep.position = pos;
	ep.position.y += 1.0f;
	ep.duration = 0.1f;
	ep.emissionsPerSecond = 15.0f;
	ep.dstFactor = GL_ONE; // 加算合成.
	ep.gravity = 0.0f;
	ep.angle = glm::radians(0.0f);//
	ep.loop = false;
	ParticleParameter pp;
	pp.acceleration = glm::vec3(0);//
	pp.velocity = glm::vec3(0, 1, 0);
	pp.lifetime = 0.1f;
	pp.scale = glm::vec2(1.0f);
	pp.color = glm::vec4(0.1f, 0.1f, 0.8f, 1.0f);
	Add(ep, pp);
}

/*
*骸骨のY、Uボタン攻撃のパーティクル.
*/
void ParticleSystem::BoneAttackY1(glm::vec3 pos, glm::vec3 rot)
{
	const glm::vec3 vPlayerFront = glm::rotate(
		glm::mat4(1), rot.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 0.5f, 5, 1);
	ParticleEmitterParameter ep;
	ep.imagePath = "Res/FireParticle.tga";
	ep.tiles = glm::ivec2(2, 2);
	ep.position = pos + vPlayerFront;
	ep.position.y += 0.0f;
	ep.duration = 0.2f;
	ep.emissionsPerSecond = 200.0f;
	ep.dstFactor = GL_ONE; // 加算合成.
	ep.gravity = 0.0f;
	ep.angle = glm::radians(70.0f);//
	ep.loop = false;
	ParticleParameter pp;
	pp.acceleration = glm::vec3(0);//
	pp.velocity = glm::vec3(0, 15, 0);
	pp.lifetime = 0.3f;
	pp.scale = glm::vec2(1.0f);
	pp.color = glm::vec4(0.1f, 0.7f, 0.9f, 1.0f);
	Add(ep, pp);
}

/*
*骸骨のY、Uボタン攻撃のパーティクル.
*/
void ParticleSystem::BoneAttackY2(glm::vec3 pos)
{
	ParticleEmitterParameter ep;
	ep.imagePath = "Res/DiskParticle.tga";
	ep.tiles = glm::ivec2(1, 1);
	ep.position = pos;
	ep.position.y += 3.0f;
	ep.emissionsPerSecond = 100.0f;
	ep.duration = 0.1f;
	ep.dstFactor = GL_ONE; // 加算合成.
	ep.gravity = 0;
	ep.angle = glm::radians(0.0f);//
	ep.loop = false;
	ParticleParameter pp;
	pp.acceleration = glm::vec3(0);//
	pp.lifetime = 0.05f;
	pp.velocity = glm::vec3(0, 10, 0);
	pp.scale = glm::vec2(0.3f, 1.0f);
	pp.color = glm::vec4(0.1f, 0.7f, 0.9f, 1.0f);
	Add(ep, pp);
}

/*
*骸骨のB、Kボタン攻撃のパーティクル.
*/
void ParticleSystem::BoneAttackB(glm::vec3 pos, int id)
{
	if (id == 0)
	{
		ParticleEmitterParameter ep;
		ep.imagePath = "Res/FireParticle.tga";
		ep.tiles = glm::ivec2(2, 2);
		ep.position = pos;
		ep.position.y += 0.5f;
		ep.emissionsPerSecond = 400.0f;
		ep.duration = 0.1f;
		ep.dstFactor = GL_ONE; // 加算合成.
		ep.gravity = 0;
		ep.angle = glm::radians(90.0f);//
		ep.loop = false;
		ParticleParameter pp;
		pp.acceleration = glm::vec3(0, 0, 0);//
		pp.lifetime = 0.3f;
		pp.velocity = glm::vec3(0, 0, 20);
		pp.scale = glm::vec2(1.0f);
		pp.color = glm::vec4(0.1f, 0.3f, 0.9f, 1.0f);
		Add(ep, pp);
	}
	else if (id == 1)
	{
		ParticleEmitterParameter ep;
		ep.imagePath = "Res/FireParticle.tga";
		ep.tiles = glm::ivec2(1, 1);
		ep.position = pos;
		ep.position.y += 0.0f;
		ep.emissionsPerSecond = 50.0f;
		ep.duration = 0.1f;
		ep.dstFactor = GL_ONE; // 加算合成.
		ep.gravity = 0;
		ep.angle = glm::radians(90.0f);//
		ep.loop = false;
		ParticleParameter pp;
		pp.acceleration = glm::vec3(0);//
		pp.lifetime = 0.2f;
		pp.velocity = glm::vec3(0, 3, 0);
		pp.scale = glm::vec2(0.5f);
		pp.color = glm::vec4(0.1f, 0.3f, 0.9f, 1.0f);
		Add(ep, pp);
	}
}

/*
*ウィザードのB、Kボタン攻撃の氷攻撃.
*/
void ParticleSystem::WizardAttackB(glm::vec3 pos ,glm::vec3 rot)
{
	const glm::vec3 matRotY = glm::rotate(
		glm::mat4(1), rot.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 7, 3, 1);
	for (size_t i = 0; i < 10; ++i)
	{
		const glm::vec3 matRotZ = glm::rotate(
			glm::mat4(1), rot.y, glm::vec3(0, 1, 0)) * glm::vec4(0, i, i, 1);
		ParticleEmitterParameter ep;
		ep.imagePath = "Res/DiskParticle.tga";
		ep.tiles = glm::ivec2(1, 1);
		ep.position = pos + matRotY + matRotZ;;
		ep.position.y += 0.0f;
		ep.duration = 0.2f;
		ep.emissionsPerSecond = 15.0f;
		ep.dstFactor = GL_ONE; // 加算合成.
		ep.gravity = 0.0f;
		ParticleParameter pp;
		pp.acceleration = glm::vec3(0);//
		ep.angle = glm::radians(0.0f);//
		ep.loop = false;
		pp.velocity = glm::vec3(0, -15, 0);
		pp.lifetime = 1.5f;
		pp.scale = glm::vec2(0.1f, 1.5f);
		pp.color = glm::vec4(0.1f, 0.3f, 1.0f, 1.0f);
		Add(ep, pp);
	}
}

/*
*ウィザードのB、Kボタン攻撃の氷攻撃.
*/
void ParticleSystem::WizardAttackY(glm::vec3 pos, glm::vec3 rot)
{
	const glm::mat4 matRotY = glm::rotate(
		glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
	const glm::vec3 matRotZ = glm::rotate(
		glm::mat4(1), rot.y, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, 3, 1);

	ParticleEmitterParameter ep;
	ep.imagePath = "Res/FireParticle.tga";
	ep.tiles = glm::ivec2(1, 1);
	ep.position = pos + matRotZ;
	ep.position.y += 1.0f;
	ep.emissionsPerSecond = 50.0f;
	ep.dstFactor = GL_ONE; // 加算合成.
	ep.gravity = 5.0f;
	ep.angle = glm::radians(90.0f);//
	ep.loop = false;
	ParticleParameter pp;
	pp.acceleration = matRotY * glm::vec4(0, 1, 5.0f, 1);
	pp.scale = glm::vec2(0.5f);
	pp.color = glm::vec4(0.9f, 0.3f, 0.1f, 1.0f);
	Add(ep, pp);
}

/*
*プレイヤーのレベルアップ.
*/
void ParticleSystem::LevelUp(glm::vec3 pos)
{
	//レベルアップ時のパーティクル.
	ParticleEmitterParameter ep;
	ep.imagePath = "Res/DiskParticle.tga";
	ep.tiles = glm::ivec2(2, 2);
	ep.position = pos;
	ep.position.y += 0.0f;
	ep.duration = 0.2f;
	ep.emissionsPerSecond = 80.0f;
	ep.dstFactor = GL_ONE; // 加算合成.
	ep.gravity = 0.0f;
	ep.angle = glm::radians(0.0f);//
	ep.loop = false;
	ParticleParameter pp;
	pp.acceleration = glm::vec3(0);//
	pp.velocity = glm::vec3(0, 15, 0);
	pp.lifetime = 0.2f;
	pp.scale = glm::vec2(0.1f, 1.0f);
	pp.color = glm::vec4(1.0f, 1.0f, 0.9f, 1.0f);
	Add(ep, pp);
}

/*
*クリスタルのパーティクル.
*/
void ParticleSystem::Crystal(glm::vec3 pos)
{
	ParticleEmitterParameter ep;
	ep.imagePath = "Res/DiskParticle.tga";
	ep.tiles = glm::ivec2(1, 1);
	ep.position = pos;
	ep.position.y = pos.y + 1.0f;
	ep.emissionsPerSecond = 40.0f;
	ep.duration = 1.0f;
	ep.dstFactor = GL_ONE; // 加算合成.
	ep.gravity = 0;
	ep.angle = glm::radians(60.0f);//
	ParticleParameter pp;
	pp.acceleration = glm::vec3(0, 5, 0);//
	pp.lifetime = 0.8f;
	pp.velocity = glm::vec3(0, 2, 0);
	pp.scale = glm::vec2(0.05f, 1.0f);
	pp.color = glm::vec4(0.1f, 0.1f, 0.9f, 1.0f);
	Add(ep, pp);
}

/*
*ゲートのパーティクル.
*/
void ParticleSystem::Gate(glm::vec3 pos,glm::vec3 epPos)
{
	ParticleEmitterParameter ep;
	ep.imagePath = "Res/FireParticle.tga";
	ep.tiles = glm::ivec2(2, 2);
	ep.position = pos + epPos;
	ep.emissionsPerSecond = 20.0f;
	ep.dstFactor = GL_ONE; // 加算合成.
	ep.gravity = 0;
	ParticleParameter pp;
	pp.scale = glm::vec2(0.5f);
	pp.color = glm::vec4(1.0f, 0.1f, 0.1f, 1.0f);
	Add(ep, pp);
}