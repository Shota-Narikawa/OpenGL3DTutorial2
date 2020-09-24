/**
*@file.cpp
*/

#define _CRT_SECURE_NO_WARNINGS
#include "Font.h"
#include <memory>
#include <iostream>
#include <stdio.h>

/**
*フォント描画オブジェクトを初期化する.
*
*@param maxCharacter	最大描画文字数.
*
*@retval true			初期化成功.
*@retval false			初期化失敗.
*/

bool FontRenderer::Init(size_t maxCharacter) {

	return spriteRenderer.Init(maxCharacter, "Res/Sprite.vert", "Res/Sprite.frag");
}

/**
*フォントファイルを読み込む.
*
*@param filename	ファイル名.
*
*@retval true	読み込み成功.
*@retval false	読み込み失敗.
*/

bool FontRenderer::LoadFromFile(const char* filename) {

	//ファイルを開く.
	std::unique_ptr<FILE, decltype(&fclose)>fp(fopen(filename, "r"), &fclose);

	if (!fp) {
		std::cerr << "[エラー]" << __func__ << ":" << filename << "を開けません.\n";
		return false;
	}

	//info行を読み込む.
	int line = 1;		//読み込む行番号(エラー表示番号).
	int spacing[2];		//一行目の読み込みチェック表.
	int ret = fscanf(fp.get(),
		"info face=\"%*[^\"]\" size=%*d bold=%*d italic=%*d charset=%*s unicode=%*d"
		" stretchH=%*d smooth=%*d aa=%*d padding=%*d,%*d,%*d,%*d spacing=%d,%d%*[^\n]",
		&spacing[0], &spacing[1]);

	if (ret < 2) {

		std::cerr << "[エラー]" << __func__ << ":" << filename << "の読み込みに失敗(" <<
			line << "行目).\n";
		return false;
	}
	++line;

	//common行を読み込む.
	float scaleH;
	ret = fscanf(fp.get(),
		" common lineHeight=%f base=%f scaleW=%*d scaleH=%f pages=%*d packed=%*d%*[^\n]",
		&lineHeight, &base, &scaleH);

	if (ret < 3) {
		std::cerr << "[エラー]" << __func__ << ":" << filename << "の読み込みに失敗(" <<
			line << "行目).\n";
		return false;
	}
	++line;

	//page行を読み込む.
	std::vector<std::string> texNameList;
	texNameList.reserve(16);

	for (;;) {

		int id;
		char tex[256];
		ret = fscanf(fp.get(), " page id=%d file=\"%255[^\"]\"", &id, tex);

		if (ret < 2) {

			break;
		}
		tex[sizeof(tex) / sizeof(tex[0]) - 1] = '\0';	//0終端を保証する.
		if (texNameList.size() <= static_cast<size_t>(id)) {

			texNameList.resize(id + 1);
		}
		texNameList[id] = std::string("Res/") + tex;
		++line;
	}
	if (texNameList.empty()) {

		std::cerr << "[エラー]" << __func__ << ":" << filename << "の読み込みに失敗(" <<
			line << "行目).\n";
		return false;
	}

	//chars行を読み込む.
	int charCount;	//char行の数.
	ret = fscanf(fp.get(), " chars count=%d", &charCount);

	if (ret < 1) {

		std::cerr << "[エラー]" << __func__ << ":" << filename << "の読み込みに失敗(" <<
			line << "行目).\n";
		return false;
	}
	++line;

	//char行を読み込む.
	characterInfoList.clear();
	characterInfoList.resize(65536);	//16bitで表せる範囲を確保.

	for (int i = 0; i < charCount; ++i) {

		CharacterInfo info;
		ret = fscanf(fp.get(),
			" char id=%d x=%f y=%f width=%f height=%f xoffset=%f yoffset=%f xadvance=%f"
			" page=%d chnl=%*d",
			&info.id, &info.uv.x, &info.uv.y, &info.size.x, &info.size.y,
			&info.offset.x, &info.offset.y, &info.xadvance, &info.page);

		if (ret < 9) {

			std::cerr << "[エラー]" << __func__ << ":" << filename << "の読み込みに失敗(" <<
				line << "行目).\n";
			return false;
		}

		//フォントファイルは左上が原点なので、OpenGLの座標系（左下が原点）に変換.
		info.uv.y = scaleH - info.uv.y - info.size.y;

		if (info.id < characterInfoList.size()) {

			characterInfoList[info.id] = info;
		}
		++line;
	}

	//テクスチャを読み込む.
	textures.clear();
	textures.reserve(texNameList.size());

	for (const std::string& e : texNameList) {

		Texture::Image2DPtr tex = Texture::Image2D::Create(e.c_str());

		if (!tex) {

			return false;
		}
		textures.push_back(tex);
	}

	return true;
}

/**
* 文字色を設定する.
*
* @param c文字色.
*/
void FontRenderer::Color(const glm::vec4& c)
{
	color = c;
}

/**
*文字色を取得する.
*
* @return文字色.
*/
const glm::vec4& FontRenderer::Color() const
{
	return color;
}

/**
*文字列の追加を開始する.
*/

void FontRenderer::BeginUpdate() {

	spriteRenderer.BeginUpdate();
}

/**
*文字列を追加する.
*
*@param position	表示開始座標（Y座標はフォントのベースライン）.
*@param str			追加するUF-16文字列.
*
*@retval true		追加成功.
*@retval false		追加失敗.
*/

bool FontRenderer::AddString(const glm::vec2& position, const wchar_t* str) {

	glm::vec2 pos = position;

	for (const wchar_t* itr = str; *itr; ++itr) {

		const CharacterInfo& info = characterInfoList[*itr];

		if (info.id >= 0 && info.size.x && info.size.y) {

			//スプライトの座標は画像の中心を指定するが、フォントは左上を指定する.
			//そこで、その差を打ち消すための補正値を計算する.

			const float baseX = info.size.x * 0.5f + info.offset.x;
			const float baseY = base - info.size.y * 0.5f - info.offset.y;
			const glm::vec3 spritePos = glm::vec3(pos + glm::vec2(baseX, baseY), 0);

			Sprite sprite(textures[info.page]);
			sprite.Position(spritePos);
			sprite.Rectangle({ info.uv, info.size });
			sprite.Color(color);
			sprite.Scale(scale);

			if (!spriteRenderer.AddVertices(sprite)) {

				return false;
			}
		}
		pos.x += info.xadvance;	//次の表示位置へ移動.
	}
	return true;
}

/**
*文字列の追加を終了する.
*/
void FontRenderer::EndUpdate() {

	spriteRenderer.EndUpdate();
}

/**
*フォントを描画する.
*
*@param screenSize	画面サイズ.
*/

void FontRenderer::Draw(const glm::vec2& screenSize) const {

	spriteRenderer.Draw(screenSize);
}

/**
*行の高さを取得する.
*
*@return	行の高さ（ピクセル数）.
*/

float FontRenderer::LineHeight() const {

	return lineHeight;
}

/**
*文字の横幅を取得する.
*
*@param		c	横幅を調べる文字.
*
*@return		文字cの横幅.
*/
float FontRenderer::XAdvance(wchar_t c) const
{
	if (c < 0 || c >= characterInfoList.size()) {
		return 0;
	}
	return characterInfoList[c].xadvance;
}

/*
*ゲーム内に表示する情報系のフォントデータをIDで管理する.
*
*@param	fontRenderer	表示するフォント.
*@param	id				表示する場面のID.
*/
void GameSceneFont(FontRenderer& fontRenderer ,int id)
{
	if(id == 0)
	{
		fontRenderer.AddString(glm::vec2(90, 200), L"ソウル");
		fontRenderer.AddString(glm::vec2(-130, -90), L"本作の主人公。");
		fontRenderer.AddString(glm::vec2(-130, -130), L"敵軍に実体を奪われ、実体のない魂の存在。");
		fontRenderer.AddString(glm::vec2(-130, -170), L"彼は再び体を取り戻すべく奮闘する。");
		fontRenderer.AddString(glm::vec2(-130, -210), L"しかし移動とジャンプしかできない。");
	}
	else if (id == 1)
	{
		fontRenderer.AddString(glm::vec2(90, 200), L"ゴブリン");
		fontRenderer.AddString(glm::vec2(-130, -90), L"ゴブリン族の一人。");
		fontRenderer.AddString(glm::vec2(-130, -130), L"自分の存在をゴブリンと認めたくないと");
		fontRenderer.AddString(glm::vec2(-130, -170), L"同族のゴブリンを絶滅させることを誓う。");
		fontRenderer.AddString(glm::vec2(-130, -210), L"彼の夢は人間の女と結婚することである。");
	}
	else if (id == 2)
	{
		fontRenderer.AddString(glm::vec2(90, 200), L"ウィザード");
		fontRenderer.AddString(glm::vec2(-130, -90), L"人間の魔道師。");
		fontRenderer.AddString(glm::vec2(-130, -130), L"魔法の名家で魔道師として育てられた。");
		fontRenderer.AddString(glm::vec2(-130, -170), L"しかし彼は落ちこぼれを絵にかいたような");
		fontRenderer.AddString(glm::vec2(-130, -210), L"人間だった。微力な攻撃魔法と");
		fontRenderer.AddString(glm::vec2(-130, -250), L"主に杖を使った近接戦を得意とする。");
	}
	else if (id == 3)
	{
		fontRenderer.AddString(glm::vec2(90, 200), L"スケルトン");
		fontRenderer.AddString(glm::vec2(-130, -90), L"黄泉の国から復活した骸骨騎士。");
		fontRenderer.AddString(glm::vec2(-130, -130), L"世界一とも名高い元エリート騎士。");
		fontRenderer.AddString(glm::vec2(-130, -170), L"それと同じく無類の酒と女好きの");
		fontRenderer.AddString(glm::vec2(-130, -210), L"遊び人だった。彼の死因は泥酔し、王の");
		fontRenderer.AddString(glm::vec2(-130, -250), L"娘へのセクハラ行為により死刑。");
	}
	else if (id == 4)
	{
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
		fontRenderer.AddString(glm::vec2(-500, -200), L"対応コマンドの");
		fontRenderer.AddString(glm::vec2(-500, -250), L"確認ができます");
	}
	else if (id == 5)
	{
		fontRenderer.AddString(glm::vec2(0, 210), L"キーボードコマンド表");
		fontRenderer.AddString(glm::vec2(-100, 150), L"WSAD:移動");
		fontRenderer.AddString(glm::vec2(-100, 100), L"M：マップ表示");
		fontRenderer.AddString(glm::vec2(-100, 50), L"U：スキル１");
		fontRenderer.AddString(glm::vec2(-100, 0), L"H：スキル２");
		fontRenderer.AddString(glm::vec2(-100, -50), L"K：スキル３");
		fontRenderer.AddString(glm::vec2(-100, -100), L"J：スキル４");
		fontRenderer.AddString(glm::vec2(170, 150), L"A：カメラ左回転");
		fontRenderer.AddString(glm::vec2(170, 100), L"D：カメラ右回転");
		fontRenderer.AddString(glm::vec2(170, 50), L"SPACE:メニュー画面");
		fontRenderer.AddString(glm::vec2(170, 0), L"ENTER:決定キー");
		fontRenderer.AddString(glm::vec2(-100, -200), L"Yボタン　または　Uボタンで");
		fontRenderer.AddString(glm::vec2(-100, -250), L"ゲームパッド操作説明に切り替えます");
	}
	else if (id == 6)
	{
		fontRenderer.AddString(glm::vec2(0, 210), L"ゲームパッドコマンド表");
		fontRenderer.AddString(glm::vec2(-100, 150), L"十字キー:移動");
		fontRenderer.AddString(glm::vec2(-100, 100), L"R：マップ表示");
		fontRenderer.AddString(glm::vec2(-100, 50), L"Y：スキル１");
		fontRenderer.AddString(glm::vec2(-100, 0), L"X：スキル２");
		fontRenderer.AddString(glm::vec2(-100, -50), L"B：スキル３");
		fontRenderer.AddString(glm::vec2(-100, -100), L"A：スキル４");
		fontRenderer.AddString(glm::vec2(170, 150), L"L2：カメラ左回転");
		fontRenderer.AddString(glm::vec2(170, 100), L"R2：カメラ右回転");
		fontRenderer.AddString(glm::vec2(170, 50), L"BACK:メニュー画面");
		fontRenderer.AddString(glm::vec2(170, 0), L"START:決定キー");
		fontRenderer.AddString(glm::vec2(-100, -200), L"Yボタン　または　Uボタンで");
		fontRenderer.AddString(glm::vec2(-100, -250), L"キーボード操作説明に切り替えます");
	}
	else if (id == 7)
	{
		fontRenderer.AddString(glm::vec2(-510, -190), L"スキルの確認が");
		fontRenderer.AddString(glm::vec2(-510, -230), L"できます。");
		fontRenderer.AddString(glm::vec2(-125, -100), L"説明");
	}
	else if (id == 8)
	{
		fontRenderer.AddString(glm::vec2(-110, 200), L"ソウル");
		fontRenderer.AddString(glm::vec2(-125, 150), L"何もできない");
		fontRenderer.AddString(glm::vec2(-125, 100), L"何もできない");
		fontRenderer.AddString(glm::vec2(-125, 50), L"何もできない");
		fontRenderer.AddString(glm::vec2(-125, 0), L"何もできない");

		fontRenderer.AddString(glm::vec2(-125, -150), L"今のままでは何もできない");
		fontRenderer.AddString(glm::vec2(-125, -200), L"他のキャラクターに憑依しよう");
	}
	else if (id == 9)
	{
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
		fontRenderer.AddString(glm::vec2(-460, 200), L"プレイヤー");
		fontRenderer.AddString(glm::vec2(-450, 150), L"コマンド");
		fontRenderer.AddString(glm::vec2(-440, 100), L"スキル");
		fontRenderer.AddString(glm::vec2(-460, 0), L"ゲーム終了");
		fontRenderer.AddString(glm::vec2(-440, -50), L"閉じる");
	}
}

/*
*分岐を必要としているフォント表示の管理.
*
*@param	fontRenderer	表示するフォント.
*@param	skCount			スキルカウント.
*@param	playerAb		プレイヤーのアビリティレベル.
*@param	id				表示する場面のID.
*/
void PlayerFont(FontRenderer& fontRenderer, int skCount, int playerAb, int id)
{
	if (id == 0)
	{
		fontRenderer.AddString(glm::vec2(-105, 200), L"ゴブリン");
		if (skCount == 1)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"強靭な爪で相手を攻撃");
			fontRenderer.AddString(glm::vec2(-125, -200), L"その爪は岩でも砕いてしまう");
		}
		else if (skCount == 2)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"早歩きができるようになる");
			fontRenderer.AddString(glm::vec2(-125, -200), L"未実装");
		}
		else if (skCount == 3)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"ダッシュができるようになる");
			fontRenderer.AddString(glm::vec2(-125, -200), L"未実装");
		}
		else if (skCount == 4)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"高速移動ができるようになる");
			fontRenderer.AddString(glm::vec2(-125, -200), L"未実装");
		}
		fontRenderer.AddString(glm::vec2(-95, 150), L"通常攻撃");
		//使えないスキルは半透明.
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
		fontRenderer.AddString(glm::vec2(-85, 100), L"早歩き");
		fontRenderer.AddString(glm::vec2(-95, 50), L"ダッシュ");
		fontRenderer.AddString(glm::vec2(-95, 0), L"高速移動");

		if (playerAb >= 4)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-95, 0), L"高速移動");
		}
		if (playerAb >= 3)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-95, 50), L"ダッシュ");
		}
		if (playerAb >= 2)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-85, 100), L"早歩き");
		}
	}
	else if (id == 1)
	{
		fontRenderer.AddString(glm::vec2(-110, 200), L"ウィザード");
		//スキルの説明.
		if (skCount == 1)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"杖で突く近距離攻撃");
			fontRenderer.AddString(glm::vec2(-125, -200), L"杖は父からもらったものである");
		}
		else if (skCount == 2)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"杖を振り下ろす近距離攻撃");
			fontRenderer.AddString(glm::vec2(-125, -200), L"父からは杖で人を殴るなと教えられた");
		}
		else if (skCount == 3)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"前方に炎魔法を飛ばす中距離攻撃");
			fontRenderer.AddString(glm::vec2(-125, -200), L"スピードは遅いが範囲は広い");
		}
		else if (skCount == 4)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"空から氷魔法を落とす遠距離攻撃");
			fontRenderer.AddString(glm::vec2(-125, -200), L"直線上につららを落とし敵を一掃する");
		}

		fontRenderer.AddString(glm::vec2(-95, 150), L"通常攻撃");
		//使えないスキルは半透明.
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
		fontRenderer.AddString(glm::vec2(-100, 100), L"かぶと割り");
		fontRenderer.AddString(glm::vec2(-95, 50), L"ファイア");
		fontRenderer.AddString(glm::vec2(-95, 0), L"ブリザド");

		if (playerAb >= 4)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-95, 0), L"ブリザド");
		}
		if (playerAb >= 3)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-95, 50), L"ファイア");
		}
		if (playerAb >= 2)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-100, 100), L"かぶと割り");
		}
	}
	else if (id == 2)
	{
		fontRenderer.AddString(glm::vec2(-110, 200), L"スケルトン");
		//スキルの説明.
		if (skCount == 1)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"剣を振り下ろす近距離攻撃");
			fontRenderer.AddString(glm::vec2(-125, -200), L"型は自己流である");
		}
		else if (skCount == 2)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"縦横の二段階攻撃");
			fontRenderer.AddString(glm::vec2(-125, -200), L"二段階による攻撃で敵を翻弄する");
		}
		else if (skCount == 3)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"勢いよく剣を振り下ろす中距離攻撃");
			fontRenderer.AddString(glm::vec2(-125, -200), L"離れた敵を剣風で薙ぎ払う");
		}
		else if (skCount == 4)
		{
			fontRenderer.AddString(glm::vec2(-125, -150), L"洗練された究極の中距離攻撃");
			fontRenderer.AddString(glm::vec2(-125, -200), L"全方位の強力な一撃で敵を一掃する");
		}

		fontRenderer.AddString(glm::vec2(-95, 150), L"通常攻撃");
		//使えないスキルは半透明.
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f));
		fontRenderer.AddString(glm::vec2(-95, 100), L"二段斬り");
		fontRenderer.AddString(glm::vec2(-80, 50), L"爆裂斬");
		fontRenderer.AddString(glm::vec2(-125, 0), L"全方位燕返し");

		if (playerAb >= 4)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-125, 0), L"全方位燕返し");
		}
		if (playerAb >= 3)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-80, 50), L"爆裂斬");
		}
		if (playerAb >= 2)
		{
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
			fontRenderer.AddString(glm::vec2(-95, 100), L"二段斬り");
		}
	}
}

/*
*ステージチェンジ、ロード画面の分岐用のフォント表示の管理.
*
*@param	fontRenderer	表示するフォント.
*@param	stage			いずれかのステージチェンジ.
*@param	flag			イベントのフラグ..
*@param	id				表示する場面のID.
*/
void StChangeFont(FontRenderer& fontRenderer, std::vector<Sprite>& sprites, int stage, bool flag, int id)
{
	if (id == 0)
	{
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 10));
		fontRenderer.AddString(glm::vec2(280, -280), L"NowLoading...");
		//ロード画面中にランダムでゲームに役立つヒントが表示される.
		if (stage == 1)
		{
			sprites[4].Scale(glm::vec2(0.4f));
			fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 10));
			fontRenderer.AddString(glm::vec2(-300, -100), L"火がついているのは一度クリアしたステージ。");
			fontRenderer.AddString(glm::vec2(-300, -150), L"だが、何回でも挑戦可能。");
		}
		else if (stage == 2)
		{
			sprites[5].Scale(glm::vec2(0.4f));
			fontRenderer.AddString(glm::vec2(-300, -100), L"主人公の肉体の情報が保管されたクリスタル。");
			fontRenderer.AddString(glm::vec2(-300, -150), L"ステージごとにランダムで配置される。");
			fontRenderer.AddString(glm::vec2(-300, -200), L"ステージが始まれば真っ先に場所を把握しよう。");
		}
		else if (stage == 3)
		{
			sprites[6].Scale(glm::vec2(0.4f));
			fontRenderer.AddString(glm::vec2(-300, -100), L"個性豊かなキャラクターたちを使用できる。");
			fontRenderer.AddString(glm::vec2(-300, -150), L"スキルやプロフィールはメニュー画面で確認可能。");
		}
		else if (stage == 4)
		{
			sprites[7].Scale(glm::vec2(0.4f));
			fontRenderer.AddString(glm::vec2(-300, -100), L"敵は大きさにより性質が異なる。");
			fontRenderer.AddString(glm::vec2(-300, -150), L"攻撃力が高いもの、移動が速いものなど。");
			fontRenderer.AddString(glm::vec2(-300, -200), L"倒す順番には優先順位をつけよう。");
		}
	}
	else if (id == 1)
	{
		fontRenderer.Color(glm::vec4(1.0f, 1.0f, 1.0f, 1));
		fontRenderer.AddString(glm::vec2(-30, -85), L"はい");
		fontRenderer.AddString(glm::vec2(-45, -125), L"いいえ");
		fontRenderer.AddString(glm::vec2(-200, -280), L"挑みますか？");

		if (stage == 1)
		{
			fontRenderer.AddString(glm::vec2(-270, -240), L"天の声：Aの試練が待っている.");
			fontRenderer.AddString(glm::vec2(20, -280), L"難易度★☆☆☆☆");
			flag = false;
		}
		else if (stage == 2)
		{
			fontRenderer.AddString(glm::vec2(-270, -240), L"天の声：Bの試練が待っている.");
			fontRenderer.AddString(glm::vec2(20, -280), L"難易度★★☆☆☆");
			flag = false;
		}
		else if (stage == 3)
		{
			fontRenderer.AddString(glm::vec2(-270, -240), L"天の声：Cの試練が待っている.");
			fontRenderer.AddString(glm::vec2(20, -280), L"難易度★★★☆☆");
			flag = false;
		}
		else if (stage == 4)
		{
			fontRenderer.AddString(glm::vec2(-270, -240), L"天の声：Dの試練が待っている.");
			fontRenderer.AddString(glm::vec2(20, -280), L"難易度★★★★★");
			flag = false;
		}
	}
}

/*
*数字フォントの表示.
*
*@param	name			値の情報.
*@param	fontRenderer	表示するフォント.
*@param	id				表示する場面のID.
*/
void NumFont(FontRenderer& fontRenderer, int name, int numTime, int id)
{
	if (id == 0)
	{
		//ステージNo表示.
		wchar_t str[] = L"STAGE.  ";
		int n = name;
		for (int i = 0; i < numTime; ++i)
		{
			str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
			n /= 10;
			if (n <= 0)
			{
				break;
			}
		}
		fontRenderer.AddString(glm::vec2(280, 200), str);
	}
	else if(id == 1)
	{
		//レベル表示.
		wchar_t str[] = L"Lv.   ";
		int n = name;
		for (int i = 0; i < numTime; ++i)
		{
			str[(sizeof(str) / sizeof(str[0]) - 2) - i] = L'0' + n % 10;
			n /= 10;
			if (n <= 0)
			{
				break;
			}
		}
		fontRenderer.AddString(glm::vec2(100, 150), str);
	}
}