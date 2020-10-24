/**
*@file Sprite.cpp
*/

#include "Sprite.h"
#include <vector>
#include <iostream>
#include <glm\gtc\matrix_transform.hpp>

/**
*Spriteコンストラクタ.
*
*@param tex	スプライトとして表示するテクスチャ.
*/
Sprite::Sprite(const Texture::Image2DPtr& tex) :
	texture(tex),
	rect(Rect{ glm::vec2(),glm::vec2(tex->Width(),tex->Height()) }) {

}

/**
*描画に使用するテクスチャを指定する.
*
*@param tex	描画に使用するテクスチャ.
*/

void Sprite::Texture(const Texture::Image2DPtr& tex) {

	texture = tex;
	Rectangle(Rect{ glm::vec2(0),glm::vec2(tex->Width(),tex->Height()) });
}

/**
*スプライト描画クラスを初期化する.
*
*@param maxSpriteCount	描画可能な最大スプライト数.
*@param vsPath			頂点シェーダーファイル名.
*@param fsPath			フラグメントシェーダーファイル名.
*
*@retval true	初期化成功.
*@retval false	初期化失敗.
*/

bool SpriteRenderer::Init(size_t maxSpriteCount, const char* vsPath, const char* fsPath) {

	vbo.Create(GL_ARRAY_BUFFER, sizeof(Vertex) * maxSpriteCount * 4,
		nullptr, GL_STREAM_DRAW);

	//四角形をmaxSpriteCount個作る.
	std::vector<GLushort> indices;
	indices.resize(maxSpriteCount * 6);	//四角形ごとにインデックスは6個必要.
	for (GLushort i = 0; i < maxSpriteCount; ++i) {

		indices[i * 6 + 0] = (i * 4) + 0;
		indices[i * 6 + 1] = (i * 4) + 1;
		indices[i * 6 + 2] = (i * 4) + 2;
		indices[i * 6 + 3] = (i * 4) + 2;
		indices[i * 6 + 4] = (i * 4) + 3;
		indices[i * 6 + 5] = (i * 4) + 0;

	}

	ibo.Create(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort),
		indices.data(), GL_STATIC_DRAW);

	//Vertex構造体に合わせて頂点アトリビュートを設定.
	vao.Create(vbo.Id(), ibo.Id());
	vao.Bind();
	vao.VertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, position));
	vao.VertexAttribPointer(
		1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, color));
	vao.VertexAttribPointer(
		2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, texCoord));
	vao.Unbind();

	program = Shader::Program::Create(vsPath, fsPath);

	primitives.reserve(64);	//32個では足りないことがあるかもしれないので64個予約.

	//初期化過程のどれかのオブジェクトの作成に失敗していたら、この関数自体も失敗とする.
	if (!vbo.Id() || !ibo.Id() || !vao.Id() || program->IsNull()) {

		return false;
	}

	return true;
}

/**
*頂点データの作成を開始する.
*/
void SpriteRenderer::BeginUpdate() {

	primitives.clear();
	vertices.clear();
	vertices.reserve(vbo.Size() / sizeof(Vertex));
}

/**
*頂点データを追加する.
*
*@param sprite	頂点データの元になるスプライト.
*
*@retval true	追加成功.
*@retval false	頂点バッファが満杯で追加できない.
*/

bool SpriteRenderer::AddVertices(const Sprite& sprite) {

	if (vertices.size() * sizeof(Vertex) >= static_cast<size_t>(vbo.Size())) {

		std::cerr << "[警告]" << __func__ << ":最大表示数を超えています.\n";
		return false;
	}

	const Texture::Image2DPtr& texture = sprite.Texture();
	const glm::vec2 reciprocalSize(
		glm::vec2(1) / glm::vec2(texture->Width(), texture->Height()));

	//短形を0.0～1.0の範囲に変換.
	Rect rect = sprite.Rectangle();
	rect.origin *= reciprocalSize;
	rect.size *= reciprocalSize;

	//中心からの大きさを計算.
	const glm::vec2 halfSize = sprite.Rectangle().size * 0.5f;

	//座標変換行列を作成.
	const glm::mat4 matT = glm::translate(glm::mat4(1), sprite.Position());
	const glm::mat4 matR = glm::rotate(glm::mat4(1), sprite.Rotation(), glm::vec3(0, 0, 1));
	const glm::mat4 matS = glm::scale(glm::mat4(1), glm::vec3(sprite.Scale(), 1));
	const glm::mat4 transform = matT * matR * matS;

	Vertex v[4];

	v[0].position = transform * glm::vec4(-halfSize.x, -halfSize.y, 0, 1);
	v[0].color = sprite.Color();
	v[0].texCoord = rect.origin;

	v[1].position = transform * glm::vec4(halfSize.x, -halfSize.y, 0, 1);
	v[1].color = sprite.Color();
	v[1].texCoord = glm::vec2(rect.origin.x + rect.size.x, rect.origin.y);

	v[2].position = transform * glm::vec4(halfSize.x, halfSize.y, 0, 1);
	v[2].color = sprite.Color();
	v[2].texCoord = rect.origin + rect.size;

	v[3].position = transform * glm::vec4(-halfSize.x, halfSize.y, 0, 1);
	v[3].color = sprite.Color();
	v[3].texCoord = glm::vec2(rect.origin.x, rect.origin.y + rect.size.y);

	vertices.insert(vertices.end(), v, v + 4);

	if (primitives.empty()) {

		//最初のプリミティブを作成する.
		primitives.push_back({ 6,0,texture });

	}
	else {
		//同じテクスチャを使っているならインデックス数と四角形ひとつぶん（インデックス6個）増やす.
		//テクスチャが使う場合は新しいプリミティブを作成する.
		Primitive& data = primitives.back();

		if (data.texture == texture) {
			data.count += 6;
		}
		else {
			primitives.push_back({ 6,data.offset + data.count * sizeof(GLushort),texture });
		}
	}

	return true;
}

/**
*頂点データの作成を終了する.
*/

void SpriteRenderer::EndUpdate() {

	vbo.BufferSubData(0, vertices.size() * sizeof(Vertex), vertices.data());
	vertices.clear();
	vertices.shrink_to_fit();
}

/**
*スプライトを描画する.
*
*@param texture		描画に使用するテクスチャ.
*@paraam screenSize	画面サイズ.
*/

void SpriteRenderer::Draw(const glm::vec2& screenSize)const {

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	vao.Bind();
	program->Use();

	//平行投影、原点は画面の中心.
	const glm::vec2 halfScreenSize = screenSize * 0.5f;
	const glm::mat4x4 matProj = glm::ortho(
		-halfScreenSize.x, halfScreenSize.x, -halfScreenSize.y, halfScreenSize.y, 1.0f, 1000.0f);
	const glm::mat4x4 matView = glm::lookAt(
		glm::vec3(0, 0, 100), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	program->SetViewProjectionMatrix(matProj * matView);

	for (const Primitive& primitive : primitives) {
		program->BindTexture(0, primitive.texture->Get());
		glDrawElements(GL_TRIANGLES, primitive.count, GL_UNSIGNED_SHORT,
			reinterpret_cast<const GLvoid*>(primitive.offset));
	}
	program->BindTexture(0, 0);
	vao.Unbind();
}

/*
*同じスプライトIDを削除する.
*/
void DeleteSpriteA(std::vector<Sprite>& sprites, int id) {

	for (auto i = sprites.begin(); i != sprites.end();) {
		if (i->id == id) {
			i = sprites.erase(i);
		}
		else {
			++i;
		}
	}
}

/*
*該当するスプライトIDを全て削除する.
*/
void DeleteSprite(std::vector<Sprite>& sprites, int i[]) {

	sprites[*i].Scale(glm::vec2(0));
}

/*
*ゲーム内で使用する画像データをまとめて管理.
*
*@param	sprites	表示するスプライト.
*/
void SpriteRenderer::GameSceneUI(std::vector<Sprite>& sprites) {

	//ゲーム内に表示させる画像リスト.
	//変数の最後にNoを入れてわかりやすくしておく.
	//ロード画面.
	Sprite road0(Texture::Image2D::Create("Res/Black.tga"));
	road0.Scale(glm::vec2(0));
	sprites.push_back(road0);

	//メイン情報(右).
	Sprite Menu1(Texture::Image2D::Create("Res/MenuWindow5.tga"));
	Menu1.Position(glm::vec3(160, 0, 0));
	Menu1.Scale(glm::vec2(0));
	sprites.push_back(Menu1);

	//メニュー一覧(左上).
	Sprite Menu2(Texture::Image2D::Create("Res/MenuWindow5.tga"));
	Menu2.Position(glm::vec3(-380, 85, 0));
	Menu2.Scale(glm::vec2(0));
	sprites.push_back(Menu2);

	//その他(左下).
	Sprite Menu3(Texture::Image2D::Create("Res/MenuWindow5.tga"));
	Menu3.Position(glm::vec3(-380, -220, 0));
	Menu3.Scale(glm::vec2(0));
	sprites.push_back(Menu3);

	//ロード画面中の画像.
	Sprite LoadA4(Texture::Image2D::Create("Res/LoadA.tga"));
	LoadA4.Position(glm::vec3(0, 130, 0));
	LoadA4.Scale(glm::vec2(0));
	sprites.push_back(LoadA4);

	Sprite LoadB5(Texture::Image2D::Create("Res/LoadB.tga"));
	LoadB5.Position(glm::vec3(0, 130, 0));
	LoadB5.Scale(glm::vec2(0));
	sprites.push_back(LoadB5);

	Sprite LoadC6(Texture::Image2D::Create("Res/LoadC.tga"));
	LoadC6.Position(glm::vec3(0, 130, 0));
	LoadC6.Scale(glm::vec2(0));
	sprites.push_back(LoadC6);

	Sprite LoadD7(Texture::Image2D::Create("Res/LoadD.tga"));
	LoadD7.Position(glm::vec3(0, 100, 0));
	LoadD7.Scale(glm::vec2(0));
	sprites.push_back(LoadD7);

	//リザルト画面のロゴ.
	Sprite NextLogo8(Texture::Image2D::Create("Res/Next.tga"));
	NextLogo8.Position(glm::vec3(100, 120, 0));
	NextLogo8.Scale(glm::vec2(0));
	sprites.push_back(NextLogo8);

	Sprite ToTitle9(Texture::Image2D::Create("Res/ToTitle.tga"));
	ToTitle9.Position(glm::vec3(100, -120, 0));
	ToTitle9.Scale(glm::vec2(0));
	sprites.push_back(ToTitle9);

	//ゲームオーバー時.
	Sprite GameOver10(Texture::Image2D::Create("Res/Over.tga"));
	GameOver10.Scale(glm::vec2(0));
	GameOver10.Position(glm::vec3(0, 100, 0));
	sprites.push_back(GameOver10);

	//ゲームクリア時.
	Sprite GameClear11(Texture::Image2D::Create("Res/Clear.tga"));
	GameClear11.Scale(glm::vec2(0));
	GameClear11.Position(glm::vec3(0, 100, 0));
	sprites.push_back(GameClear11);

	//メニュー画面のプレイヤーアイコン.
	Sprite pSoul12(Texture::Image2D::Create("Res/pSoul.tga"));
	pSoul12.Position(glm::vec3(-50, 110, 0));
	pSoul12.Scale(glm::vec2(0));
	sprites.push_back(pSoul12);

	Sprite pGoblin13(Texture::Image2D::Create("Res/pGoblin.tga"));
	pGoblin13.Position(glm::vec3(-50, 110, 0));
	pGoblin13.Scale(glm::vec2(0));
	sprites.push_back(pGoblin13);

	Sprite pWizard14(Texture::Image2D::Create("Res/pWizard.tga"));
	pWizard14.Position(glm::vec3(-50, 110, 0));
	pWizard14.Scale(glm::vec2(0));
	sprites.push_back(pWizard14);

	Sprite pGaikotsu15(Texture::Image2D::Create("Res/pGaikotsu.tga"));
	pGaikotsu15.Position(glm::vec3(-50, 110, 0));
	pGaikotsu15.Scale(glm::vec2(0));
	sprites.push_back(pGaikotsu15);

	//HP.
	Sprite barHP16(Texture::Image2D::Create("Res/Green.tga"));
	barHP16.Scale(glm::vec2(0));
	sprites.push_back(barHP16);

	//MP.
	Sprite barMP17(Texture::Image2D::Create("Res/Blue.tga"));
	barMP17.Scale(glm::vec2(0));
	sprites.push_back(barMP17);

	//ウィザードと骸骨の攻撃コマンド.
	Sprite AttackNormal18(Texture::Image2D::Create("Res/AttackTest.dds"));
	AttackNormal18.Position(glm::vec3(500, -310, 0));	//下アイコン初期値.
	AttackNormal18.Scale(glm::vec2(0));
	sprites.push_back(AttackNormal18);

	Sprite AttackStrong19(Texture::Image2D::Create("Res/Attack2.tga"));
	AttackStrong19.Position(glm::vec3(420, -230, 0));	//左アイコン初期値.
	AttackStrong19.Scale(glm::vec2(0));
	sprites.push_back(AttackStrong19);

	//ウィザードの攻撃コマンド.
	Sprite Magic20(Texture::Image2D::Create("Res/Slowing.tga"));
	Magic20.Position(glm::vec3(500, -150, 0));	//上アイコン初期値.
	Magic20.Scale(glm::vec2(0));
	sprites.push_back(Magic20);

	Sprite Meteo21(Texture::Image2D::Create("Res/Meteo.tga"));
	Meteo21.Position(glm::vec3(580, -230, 0));	//右アイコン初期値.
	Meteo21.Scale(glm::vec2(0));//Meteo33.Scale(glm::vec2(0.12f));
	sprites.push_back(Meteo21);

	//選択アイコン.
	Sprite IconBlue22(Texture::Image2D::Create("Res/select.tga"));
	IconBlue22.Position(glm::vec3(-380, 210, 0));
	//IconBlue22.Scale(glm::vec2(0.8f , 0.15f));
	IconBlue22.Scale(glm::vec2(0));	//IconBlue36.Scale(glm::vec2(0.28f,0.22f));
	sprites.push_back(IconBlue22);

	//選択アイコン.
	Sprite selectIcon23(Texture::Image2D::Create("Res/selectA.tga"));
	selectIcon23.Position(glm::vec3(-490, 210, 0));
	selectIcon23.Scale(glm::vec2(0));
	sprites.push_back(selectIcon23);

	//選択アイコン.
	Sprite selectIcon24(Texture::Image2D::Create("Res/selectA.tga"));
	selectIcon24.Position(glm::vec3(-490, 210, 0));
	selectIcon24.Scale(glm::vec2(0));
	sprites.push_back(selectIcon24);

	//防衛ラインのHPバー.
	Sprite defenceHP25(Texture::Image2D::Create("Res/Green.tga"));
	defenceHP25.Scale(glm::vec2(0));
	sprites.push_back(defenceHP25);

	//防衛ラインのアイコン.
	Sprite defenceIcon26(Texture::Image2D::Create("Res/Crystal.tga"));
	defenceIcon26.Position(glm::vec3(-440, 345, 0));
	defenceIcon26.Scale(glm::vec2(0));
	sprites.push_back(defenceIcon26);

	//敵の残数アイコン.
	Sprite enemyIcon27(Texture::Image2D::Create("Res/Gobrin.tga"));
	enemyIcon27.Position(glm::vec3(290, 350, 0));
	enemyIcon27.Scale(glm::vec2(0));
	sprites.push_back(enemyIcon27);

	//防衛ラインのHP背景.
	Sprite defenceHP28(Texture::Image2D::Create("Res/HPBar.tga"));
	defenceHP28.Scale(glm::vec2(0));
	sprites.push_back(defenceHP28);

	//骸骨の攻撃アイコン.
	Sprite AttackBladeA29(Texture::Image2D::Create("Res/AttackBlade1.tga"));
	AttackBladeA29.Position(glm::vec3(500, -150, 0));	//上アイコン初期値.
	AttackBladeA29.Scale(glm::vec2(0));
	sprites.push_back(AttackBladeA29);

	Sprite AttackBladeB30(Texture::Image2D::Create("Res/AttackBlade2.tga"));
	AttackBladeB30.Position(glm::vec3(580, -230, 0));	//右アイコン初期値.
	AttackBladeB30.Scale(glm::vec2(0));
	sprites.push_back(AttackBladeB30);

	//ゴブリンのアイコン.
	Sprite GobAttack31(Texture::Image2D::Create("Res/GobAttack.tga"));
	GobAttack31.Position(glm::vec3(500, -310, 0));	//下アイコン初期値.
	GobAttack31.Scale(glm::vec2(0));
	sprites.push_back(GobAttack31);

	Sprite GobDash32(Texture::Image2D::Create("Res/StatusUp.tga"));
	GobDash32.Position(glm::vec3(420, -230, 0));	//左アイコン初期値.
	GobDash32.Scale(glm::vec2(0));
	sprites.push_back(GobDash32);

	//何も使えないアイコン.
	Sprite NoAttack33(Texture::Image2D::Create("Res/NoAttack.tga"));
	NoAttack33.Position(glm::vec3(500, -310, 0));	//下アイコン初期値.
	NoAttack33.Scale(glm::vec2(0));//NoAttack50.Scale(glm::vec2(0.2f));
	sprites.push_back(NoAttack33);

	//各コマンドのクールタイム.
	Sprite IconGreen34(Texture::Image2D::Create("Res/IconGreen.tga"));
	IconGreen34.Position(glm::vec3(0, 0, 0));
	IconGreen34.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
	sprites.push_back(IconGreen34);

	Sprite IconGreen35(Texture::Image2D::Create("Res/IconGreen.tga"));
	IconGreen35.Position(glm::vec3(0, 0, 0));
	IconGreen35.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
	sprites.push_back(IconGreen35);

	Sprite IconGreen36(Texture::Image2D::Create("Res/IconGreen.tga"));
	IconGreen36.Position(glm::vec3(0, 0, 0));
	IconGreen36.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
	sprites.push_back(IconGreen36);

	Sprite IconGreen37(Texture::Image2D::Create("Res/IconGreen.tga"));
	IconGreen37.Position(glm::vec3(0, 0, 0));
	IconGreen37.Scale(glm::vec2(0));	//IconGreen35.Scale(glm::vec2(1.55f));
	sprites.push_back(IconGreen37);

	Sprite MiniMap38(Texture::Image2D::Create("Res/MiniMap.tga"));
	MiniMap38.Position(glm::vec3(530, 270, 0));
	MiniMap38.Scale(glm::vec2(0));
	sprites.push_back(MiniMap38);

	Sprite PMiniIcon39(Texture::Image2D::Create("Res/PMiniIcon.tga"));
	PMiniIcon39.Scale(glm::vec2(0)); //PMiniIcon39.Scale(glm::vec2(0.02f, 0.02f));
	sprites.push_back(PMiniIcon39);

	//ミニマップ上の防衛ラインのアイコン.
	Sprite DMiniIcon40(Texture::Image2D::Create("Res/Crystal.tga"));
	DMiniIcon40.Position(glm::vec3(-250, 350, 0));
	DMiniIcon40.Scale(glm::vec2(0));
	sprites.push_back(DMiniIcon40);

	Sprite DMiniIcon41(Texture::Image2D::Create("Res/Crystal.tga"));
	DMiniIcon41.Scale(glm::vec2(0));
	sprites.push_back(DMiniIcon41);

	Sprite DMiniIcon42(Texture::Image2D::Create("Res/Crystal.tga"));
	DMiniIcon42.Scale(glm::vec2(0));
	sprites.push_back(DMiniIcon42);

	//シールドアイコン.
	Sprite Shield43(Texture::Image2D::Create("Res/Shield.tga"));
	Shield43.Position(glm::vec3(-500, 345, 0));
	Shield43.Scale(glm::vec2(0));
	sprites.push_back(Shield43);

	Sprite ShieldUP44(Texture::Image2D::Create("Res/ShieldUP.tga"));
	ShieldUP44.Position(glm::vec3(-480, 340, 0));
	ShieldUP44.Scale(glm::vec2(0));
	sprites.push_back(ShieldUP44);

	//コンボ発生時.
	Sprite Combo45(Texture::Image2D::Create("Res/Combo.tga"));
	Combo45.Scale(glm::vec2(0));
	sprites.push_back(Combo45);

	Sprite ComboMes46(Texture::Image2D::Create("Res/ComboMesseage.tga"));
	ComboMes46.Position(glm::vec3(0, -70, 0));
	ComboMes46.Scale(glm::vec2(0));
	sprites.push_back(ComboMes46);

	Sprite ComboMax47(Texture::Image2D::Create("Res/Max.tga"));
	ComboMax47.Position(glm::vec3(560, 305, 0));
	ComboMax47.Scale(glm::vec2(0));
	sprites.push_back(ComboMax47);
}

/*
*ゲーム内のUIのサイズ変更.
*
*@param	sprites	スプライト描画.
*@param	id		どの場面で変更するかのID.
*/
void SpriteRenderer::SpriteChange(std::vector<Sprite>& sprites, int selCou, int skCou, int id)
{
	if (id == 0)
	{
		sprites[1].Scale(glm::vec2(1, 3.5f));
		sprites[1].Position(glm::vec3(160, 0, 0));
		sprites[2].Scale(glm::vec2(0.4f, 2.5f));
		sprites[3].Scale(glm::vec2(0.4f, 0.9f));
		sprites[2].Position(glm::vec3(-380, 85, 0));
		sprites[3].Position(glm::vec3(-380, -220, 0));

		sprites[22].Scale(glm::vec2(1.08f, 0.11f));
		sprites[23].Scale(glm::vec2(0.1f));
		sprites[24].Scale(glm::vec2(0.1f));

		sprites[16].Scale(glm::vec2(0));
		sprites[17].Scale(glm::vec2(0));
		sprites[18].Scale(glm::vec2(0));
		sprites[19].Scale(glm::vec2(0));
		sprites[20].Scale(glm::vec2(0));
		sprites[21].Scale(glm::vec2(0));
		sprites[26].Scale(glm::vec2(0));
		sprites[27].Scale(glm::vec2(0));
		sprites[28].Scale(glm::vec2(0));
		sprites[31].Scale(glm::vec2(0));
		sprites[32].Scale(glm::vec2(0));
		sprites[29].Scale(glm::vec2(0));
		sprites[30].Scale(glm::vec2(0));
		sprites[33].Scale(glm::vec2(0));
		sprites[34].Scale(glm::vec2(0));
		sprites[35].Scale(glm::vec2(0));
		sprites[36].Scale(glm::vec2(0));
		sprites[37].Scale(glm::vec2(0));
		sprites[38].Scale(glm::vec2(0));
		sprites[39].Scale(glm::vec2(0));
		sprites[40].Scale(glm::vec2(0));
		sprites[41].Scale(glm::vec2(0));
		sprites[42].Scale(glm::vec2(0));
		sprites[43].Scale(glm::vec2(0));
		sprites[44].Scale(glm::vec2(0));
		sprites[45].Scale(glm::vec2(0));
		sprites[46].Scale(glm::vec2(0));
		sprites[47].Scale(glm::vec2(0));
	}
	else if (id == 1)
	{
		sprites[1].Scale(glm::vec2(0));
		sprites[2].Scale(glm::vec2(0));
		sprites[3].Scale(glm::vec2(0));
		sprites[12].Scale(glm::vec2(0));
		sprites[13].Scale(glm::vec2(0));
		sprites[14].Scale(glm::vec2(0));
		sprites[15].Scale(glm::vec2(0));
		sprites[16].Scale(glm::vec2(0));
		sprites[17].Scale(glm::vec2(0));
		sprites[22].Scale(glm::vec2(0));
		sprites[23].Scale(glm::vec2(0));
		sprites[24].Scale(glm::vec2(0));
		sprites[28].Scale(glm::vec2(0));
	}
	else if (id == 2)
	{
		if (skCou == 1)
		{
			//選択アイコン.
			sprites[22].Position(glm::vec3(-30, 160, 0));
			sprites[23].Position(glm::vec3(80, 160, 0));
			sprites[24].Position(glm::vec3(-140, 160, 0));
		}
		else if (skCou == 2)
		{
			//選択アイコン.
			sprites[22].Position(glm::vec3(-30, 110, 0));
			sprites[23].Position(glm::vec3(80, 110, 0));
			sprites[24].Position(glm::vec3(-140, 110, 0));
		}
		else if (skCou == 3)
		{
			//選択アイコン.
			sprites[22].Position(glm::vec3(-30, 60, 0));
			sprites[23].Position(glm::vec3(80, 60, 0));
			sprites[24].Position(glm::vec3(-140, 60, 0));
		}
		else if (skCou == 4)
		{
			//選択アイコン.
			sprites[22].Position(glm::vec3(-30, 10, 0));
			sprites[23].Position(glm::vec3(80, 10, 0));
			sprites[24].Position(glm::vec3(-140, 10, 0));
		}
	}
	else if (id == 3)
	{
		if (selCou == 1)
		{
			//選択アイコン.
			sprites[22].Position(glm::vec3(-380, 210, 0));
			sprites[23].Position(glm::vec3(-270, 210, 0));
			sprites[24].Position(glm::vec3(-490, 210, 0));
		}
		else if (selCou == 2)
		{
			//選択アイコン.
			sprites[22].Position(glm::vec3(-380, 160, 0));
			sprites[23].Position(glm::vec3(-270, 160, 0));
			sprites[24].Position(glm::vec3(-490, 160, 0));
		}
		else if (selCou == 3)
		{
			//選択アイコン.
			sprites[22].Position(glm::vec3(-380, 110, 0));
			sprites[23].Position(glm::vec3(-270, 110, 0));
			sprites[24].Position(glm::vec3(-490, 110, 0));
		}
		else if (selCou == 4)
		{
			//選択アイコン.
			sprites[22].Position(glm::vec3(-380, 10, 0));
			sprites[23].Position(glm::vec3(-270, 10, 0));
			sprites[24].Position(glm::vec3(-490, 10, 0));
		}
		else if (selCou == 5)
		{
			//選択アイコン.
			sprites[22].Position(glm::vec3(-380, -40, 0));
			sprites[23].Position(glm::vec3(-270, -40, 0));
			sprites[24].Position(glm::vec3(-490, -40, 0));
		}
	}
	else if(id == 4)
	{
		sprites[2].Scale(glm::vec2(0));
		sprites[3].Scale(glm::vec2(0));
		sprites[22].Scale(glm::vec2(0));
		sprites[23].Scale(glm::vec2(0));
		sprites[24].Scale(glm::vec2(0));
	}
	else if(id == 5)
	{
		if (selCou == 0)
		{
			//選択アイコン.
			sprites[24].Position(glm::vec3(82, -75, 0));
			sprites[22].Position(glm::vec3(0, -75, 0));
			sprites[23].Position(glm::vec3(-83, -75, 0));
		}
		else if (selCou == 1)
		{
			//選択アイコン.
			sprites[24].Position(glm::vec3(82, -115, 0));
			sprites[22].Position(glm::vec3(0, -115, 0));
			sprites[23].Position(glm::vec3(-83, -115, 0));
		}
	}
	else if (id == 6)
	{
		if (selCou == 0)
		{
			//選択アイコン.
			sprites[22].Scale(glm::vec2(1.61f, 0.22f));
			sprites[23].Scale(glm::vec2(0.2f));
			sprites[24].Scale(glm::vec2(0.2f));
			sprites[24].Position(glm::vec3(260, 120, 0));
			sprites[22].Position(glm::vec3(100, 120, 0));
			sprites[23].Position(glm::vec3(-60, 120, 0));
		}
		else if (selCou == 1)
		{
			//選択アイコン.
			sprites[22].Scale(glm::vec2(1.61f, 0.22f));
			sprites[23].Scale(glm::vec2(0.2f));
			sprites[24].Scale(glm::vec2(0.2f));
			sprites[24].Position(glm::vec3(260, -120, 0));
			sprites[22].Position(glm::vec3(100, -120, 0));
			sprites[23].Position(glm::vec3(-60, -125, 0));
		}
	}
	else if (id == 7)
	{
		sprites[12].Scale(glm::vec2(0));
		sprites[13].Scale(glm::vec2(0));
		sprites[14].Scale(glm::vec2(0));
		sprites[15].Scale(glm::vec2(0));

		sprites[16].Scale(glm::vec2(0));
		sprites[17].Scale(glm::vec2(0));
	}
	else if (id == 8)
	{
	sprites[1].Scale(glm::vec2(0));
	sprites[2].Scale(glm::vec2(0));
	sprites[3].Scale(glm::vec2(0));
	sprites[12].Scale(glm::vec2(0));
	sprites[13].Scale(glm::vec2(0));
	sprites[14].Scale(glm::vec2(0));
	sprites[15].Scale(glm::vec2(0));
	sprites[16].Scale(glm::vec2(0));
	sprites[17].Scale(glm::vec2(0));
	sprites[22].Scale(glm::vec2(0));
	sprites[23].Scale(glm::vec2(0));
	sprites[24].Scale(glm::vec2(0));
	}
	else if (id == 9)
	{
	sprites[25].Scale(glm::vec2(0));
	sprites[26].Scale(glm::vec2(0));
	sprites[27].Scale(glm::vec2(0));
	sprites[43].Scale(glm::vec2(0));
	sprites[44].Scale(glm::vec2(0));
	}
	else if (id == 10)
	{
	sprites[0].Scale(glm::vec2(6.0f, 4.0f));
	sprites[1].Scale(glm::vec2(1.5f, 4.0f));
	sprites[1].Position(glm::vec3(1));

	sprites[2].Scale(glm::vec2(0));
	sprites[3].Scale(glm::vec2(0));
	sprites[16].Scale(glm::vec2(0));
	sprites[17].Scale(glm::vec2(0));
	sprites[18].Scale(glm::vec2(0));
	sprites[19].Scale(glm::vec2(0));
	sprites[20].Scale(glm::vec2(0));
	sprites[21].Scale(glm::vec2(0));
	sprites[22].Scale(glm::vec2(0));
	sprites[23].Scale(glm::vec2(0));
	sprites[24].Scale(glm::vec2(0));
	sprites[26].Scale(glm::vec2(0));
	sprites[27].Scale(glm::vec2(0));
	sprites[28].Scale(glm::vec2(0));
	sprites[31].Scale(glm::vec2(0));
	sprites[32].Scale(glm::vec2(0));
	sprites[29].Scale(glm::vec2(0));
	sprites[30].Scale(glm::vec2(0));
	sprites[33].Scale(glm::vec2(0));
	sprites[34].Scale(glm::vec2(0));
	sprites[35].Scale(glm::vec2(0));
	sprites[36].Scale(glm::vec2(0));
	sprites[37].Scale(glm::vec2(0));
	sprites[38].Scale(glm::vec2(0));
	sprites[39].Scale(glm::vec2(0));
	}
	else if (id == 11)
	{
	sprites[2].Scale(glm::vec2(0.3f, 0.7f));
	sprites[3].Scale(glm::vec2(0.9f, 0.7f));
	sprites[22].Scale(glm::vec2(0.83f, 0.11f));
	sprites[23].Scale(glm::vec2(0.1f));
	sprites[24].Scale(glm::vec2(0.1f));
	sprites[2].Position(glm::vec3(0, -100, 0));
	sprites[3].Position(glm::vec3(0, -250, 0));
	sprites[24].Position(glm::vec3(82, -75, 0));
	sprites[22].Position(glm::vec3(0, -75, 0));
	sprites[23].Position(glm::vec3(-83, -75, 0));
	}
	else if (id == 12)
	{
	sprites[11].Scale(glm::vec2(0));
	sprites[16].Scale(glm::vec2(0));
	sprites[17].Scale(glm::vec2(0));
	sprites[18].Scale(glm::vec2(0));
	sprites[19].Scale(glm::vec2(0));
	sprites[20].Scale(glm::vec2(0));
	sprites[21].Scale(glm::vec2(0));
	sprites[25].Scale(glm::vec2(0));
	sprites[26].Scale(glm::vec2(0));
	sprites[27].Scale(glm::vec2(0));
	sprites[28].Scale(glm::vec2(0));
	sprites[29].Scale(glm::vec2(0));
	sprites[30].Scale(glm::vec2(0));
	sprites[31].Scale(glm::vec2(0));
	sprites[32].Scale(glm::vec2(0));
	sprites[33].Scale(glm::vec2(0));
	sprites[34].Scale(glm::vec2(0));
	sprites[35].Scale(glm::vec2(0));
	sprites[36].Scale(glm::vec2(0));
	sprites[37].Scale(glm::vec2(0));
	sprites[38].Scale(glm::vec2(0));
	sprites[39].Scale(glm::vec2(0));
	sprites[40].Scale(glm::vec2(0));
	sprites[41].Scale(glm::vec2(0));
	sprites[42].Scale(glm::vec2(0));
	sprites[43].Scale(glm::vec2(0));
	sprites[44].Scale(glm::vec2(0));
	sprites[45].Scale(glm::vec2(0));
	sprites[46].Scale(glm::vec2(0));
	sprites[47].Scale(glm::vec2(0));

	sprites[1].Scale(glm::vec2(1, 3.5f));
	sprites[1].Position(glm::vec3(100, 0, 0));
	sprites[8].Scale(glm::vec2(1));
	sprites[9].Scale(glm::vec2(1));
	}
}

/**
*分岐でスプライト描画しているUIの変更.
*
*@param	sprites	スプライト描画.
*@param	other	int型変数.
*/
void SpriteRenderer::SprRootChange(std::vector<Sprite>& sprites, int other, int id)
{
	if (id == 0)
	{
		if (other == 0)
		{
			sprites[12].Scale(glm::vec2(0.25f));
		}
		else if (other == 1)
		{
			sprites[13].Scale(glm::vec2(0.3f));
		}
		else if (other == 2)
		{
			sprites[14].Scale(glm::vec2(0.3f));
		}
		else if (other == 3)
		{
			sprites[15].Scale(glm::vec2(0.3f));
		}
	}
	else if (id == 1)
	{
		if (other != 1)
		{
			sprites[26].Scale(glm::vec2(2));
			sprites[27].Scale(glm::vec2(1));
		}
	}else if(id == 2)
	{
		if (other == 0)
		{
			sprites[12].Scale(glm::vec2(0.25f));
		}
		else if (other == 1)
		{
			sprites[13].Scale(glm::vec2(0.3f));
		}
		else if (other == 2)
		{
			sprites[14].Scale(glm::vec2(0.3f));
		}
		else if (other == 3)
		{
			sprites[15].Scale(glm::vec2(0.3f));
		}
	}
}

/*
*ミニマップ表示.
*
*@param	sprites	描画するスプライト.
*@param	a		アクターリスト.
*@param	flag	フラグ.
*@param	pos		プレイヤーの位置.
*/
void SpriteRenderer::MiniMap(std::vector<Sprite>& sprites, ActorList& a, bool flag, glm::vec3 pos)
{
	if (flag)
	{
		//プレイヤーのミニマップアイコン.
		sprites[38].Scale(glm::vec2(1));
		sprites[39].Scale(glm::vec2(0.02f));
		const glm::vec3 startPos(79, 0, 100);
		mapIcon.position = glm::vec3(pos.x - startPos.x, -(pos.z - startPos.z), 0) * 2.8f;
		sprites[39].Position(mapIcon.position + glm::vec3(530, 270, 0));

		//クリスタルのミニマップアイコン.
		sprites[40].Scale(glm::vec2(1));
		sprites[41].Scale(glm::vec2(1));
		sprites[42].Scale(glm::vec2(1));

		const glm::vec3 x = (*(a.begin() + 0))->position;
		const glm::vec3 y = (*(a.begin() + 1))->position;
		const glm::vec3 z = (*(a.begin() + 2))->position;
		const glm::vec3 X = glm::vec3(x.x - startPos.x,
			-(x.z - startPos.z), 0) * 2.8f;
		const glm::vec3 Y = glm::vec3(y.x - startPos.x,
			-(y.z - startPos.z), 0) * 2.8f;
		const glm::vec3 Z = glm::vec3(z.x - startPos.x,
			-(z.z - startPos.z), 0) * 2.8f;
		sprites[40].Position(X + glm::vec3(530, 270, 0));
		sprites[41].Position(Y + glm::vec3(530, 270, 0));
		sprites[42].Position(Z + glm::vec3(530, 270, 0));
	}
	else
	{
		sprites[38].Scale(glm::vec2(0));
		sprites[39].Scale(glm::vec2(0));
		sprites[40].Scale(glm::vec2(0));
		sprites[41].Scale(glm::vec2(0));
		sprites[42].Scale(glm::vec2(0));
	}
}

/*
*ディフェンスポイントのUI.
*
*@param	defLine	ディフェンスポイントの体力.
*@param	sprites	描画するスプライト.
*/
void SpriteRenderer::DefenceUI(float defLine, std::vector<Sprite>& sprites)
{
	sprites[26].Scale(glm::vec2(2));
	sprites[27].Scale(glm::vec2(1));

	//防衛ラインのHPバーの表示.
	//攻撃を受けている時.
	if (defLine >= 0)
	{
		sprites[25].Scale(glm::vec2(13.0f * defLine / 100, 0.8f));
		sprites[25].Position(glm::vec3((520 * defLine / 100) / 2 - 370, 345, 0));

		sprites[28].Scale(glm::vec2(0.4f, 0.3f));
		sprites[28].Position(glm::vec3(-80, 350, 0));
	}
	if (defLine <= 10)
	{
		sprites[25].Texture(Texture::Image2D::Create("Res/Red.tga"));
	}
	else if (defLine <= 40)
	{
		sprites[25].Texture(Texture::Image2D::Create("Res/yellow.tga"));
	}
}

/*
*コンボのUI.
*
*@param	combo		コンボ数.
*@param	comTimer	コンボタイマー.
*@param	comBuf		コンボに応じたバフ.
*@param	sprites		描画するスプライト.
*/
void SpriteRenderer::comboUI(float combo,float comTimer,float comBuf, std::vector<Sprite>& sprites)
{
	if (combo > 0)
	{
		sprites[43].Scale(glm::vec2(1));
		sprites[44].Scale(glm::vec2(0.5f));
	}
	//コンボ発生時に出す.
	if (comTimer >= 2.0f)
	{
		sprites[45].Scale(glm::vec2(0));
		sprites[46].Scale(glm::vec2(0));
	}
	else if (comTimer > 0.0f && combo <= 1)
	{
		sprites[45].Scale(glm::vec2(1.0f));
		sprites[46].Scale(glm::vec2(1.0f));
	}
	if (combo < 10)
	{
		sprites[47].Scale(glm::vec2(0));
	}
	else if (combo >= 10)
	{
		sprites[47].Scale(glm::vec2(1.0f));
	}
	for (int i = 0; i < combo; ++i)
	{
		comBuf = (i / combo) + 1.0f;
	}
}

/*
*プレイヤーのスキルコマンドUI.
*
*@param	pID		プレイヤーのID.
*@param	pAb		プレイヤーのアビリティレベル.
*@param	sprites	描画するスプライト.
*/
void SpriteRenderer::pCommandUI(int pID, int pAb,std::vector<Sprite>& sprites)
{
	//魂のスキルコマンドパネル.
	if (pID == 0)
	{
		sprites[33].Scale(glm::vec2(0.2f));

		sprites[18].Scale(glm::vec2(0));
		sprites[19].Scale(glm::vec2(0));
		sprites[20].Scale(glm::vec2(0));
		sprites[21].Scale(glm::vec2(0));
		sprites[31].Scale(glm::vec2(0));
		sprites[32].Scale(glm::vec2(0));
		sprites[29].Scale(glm::vec2(0));
		sprites[30].Scale(glm::vec2(0));
	}
	//ゴブリンのスキルコマンドパネル.
	else if (pID == 1)
	{
		sprites[31].Scale(glm::vec2(0.2f));
		/*sprites[32].Scale(glm::vec2(0.2f));*/

		sprites[18].Scale(glm::vec2(0));
		sprites[19].Scale(glm::vec2(0));
		sprites[20].Scale(glm::vec2(0));
		sprites[21].Scale(glm::vec2(0));
		sprites[29].Scale(glm::vec2(0));
		sprites[30].Scale(glm::vec2(0));
		sprites[33].Scale(glm::vec2(0));
	}
	//ウィザードのスキルコマンドパネル.
	else if (pID == 2)
	{
		sprites[18].Scale(glm::vec2(0.2f));

		if (pAb >= 4)
		{
			sprites[21].Scale(glm::vec2(0.12f));
		}
		if (pAb >= 3)
		{
			sprites[20].Scale(glm::vec2(0.2f));
		}
		if (pAb >= 2)
		{
			sprites[19].Scale(glm::vec2(0.2f));
		}

		sprites[31].Scale(glm::vec2(0));
		sprites[32].Scale(glm::vec2(0));
		sprites[29].Scale(glm::vec2(0));
		sprites[30].Scale(glm::vec2(0));
		sprites[33].Scale(glm::vec2(0));
	}
	//骸骨のスキルコマンドパネル.
	else if (pID == 3)
	{
		sprites[18].Scale(glm::vec2(0.2f));

		if (pAb >= 4)
		{
			sprites[30].Scale(glm::vec2(0.2f));
		}
		if (pAb >= 3)
		{
			sprites[29].Scale(glm::vec2(0.2f));
		}
		if (pAb >= 2)
		{
			sprites[19].Scale(glm::vec2(0.2f));
		}
		sprites[20].Scale(glm::vec2(0));
		sprites[21].Scale(glm::vec2(0));
		sprites[31].Scale(glm::vec2(0));
		sprites[32].Scale(glm::vec2(0));
		sprites[33].Scale(glm::vec2(0));
	}
}

/*
*スキルのインターバルとUI動作.
*
*@param	sprites		描画するスプライト.
*@param	pAb			プレイヤーのアビリティレベル.
*@param	pID			プレイヤーのID.
*@param	s			下のスキルフラグ.
*@param	e			右のスキルフラグ.
*@param	n			上のスキルフラグ.
*@param	w			左のスキルフラグ.
*@param	eT			右のスキルタイマー.
*@param	nT			南のスキルタイマー.
*@param	wT			左のスキルタイマー.
*@param	deltaTime	前回からの更新からの経過時間（秒）..
*/
void SpriteRenderer::comIntUI(std::vector<Sprite>& sprites ,int pAb, int pID, bool s,bool e, bool n, bool w,
								float eT,float nT, float wT, float deltaTime)
{
	//初期攻撃したかどうか.
	if (s == true)
	{
		//下.
		sprites[36].Scale(glm::vec2(0));
	}
	else if (s == false)
	{
		sprites[36].Scale(glm::vec2(1.55f, 1.55f));
		sprites[36].Position(glm::vec3(500, -310, 0));
	}
	//右.
	if (pAb >= 4)
	{
		if (e == true)
		{
			eT += deltaTime;
			sprites[34].Scale(glm::vec2(1.55f, 1.55f * eT / 4.0f));
			sprites[34].Position(glm::vec3(580, 24.8f * 2.5f * eT / 4.0f / 2 - 261, 0));
		}
		else if (e == false)
		{
			sprites[34].Scale(glm::vec2(1.55f, 1.55f));
			sprites[34].Position(glm::vec3(580, -230, 0));
		}
	}
	//上.
	if (pAb >= 3)
	{
		if (n == true)
		{
			nT += deltaTime;
			sprites[37].Scale(glm::vec2(1.55f, 1.55f * nT / 3.0f));
			sprites[37].Position(glm::vec3(500, 24.8f * 2.5f * nT / 3.0f / 2 - 181, 0));
		}
		else if (n == false)
		{
			sprites[37].Scale(glm::vec2(1.55f, 1.55f));
			sprites[37].Position(glm::vec3(500, -150, 0));
		}
	}
	//左.
	if (pAb >= 2 && pID != 1)
	{
		if (w == true)
		{
			wT += deltaTime;
			sprites[35].Scale(glm::vec2(1.55f, 1.55f * wT / 2.0f));
			sprites[35].Position(glm::vec3(420, 24.8f * 2.5f * wT / 2.0f / 2 - 261, 0));
		}
		else if (w == false)
		{
			sprites[35].Scale(glm::vec2(1.55f, 1.55f));
			sprites[35].Position(glm::vec3(420, -230, 0));
		}
	}

	if (pID == 1)
	{
		sprites[34].Scale(glm::vec2(0));
		sprites[35].Scale(glm::vec2(0));
		sprites[37].Scale(glm::vec2(0));
	}
	else if (pID == 0)
	{
		sprites[34].Scale(glm::vec2(0));
		sprites[35].Scale(glm::vec2(0));
		sprites[37].Scale(glm::vec2(0));
	}

	if (wT >= 2.0f)
	{
		w = false;
		wT = 0.0f;
	}
	if (eT >= 4.0f)
	{
		e = false;
		eT = 0.0f;
	}
	if (nT >= 3.0f)
	{
		n = false;
		nT = 0.0f;
	}
}

/**
*スプライト描画データを消去する.
*/

void SpriteRenderer::Clear() {

	primitives.clear();
}
