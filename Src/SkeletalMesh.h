/**
* @file SleletalMesh.h
*/
#ifndef SKELETAL_MESH_H_INCLUDED
#define SKELETAL_MESH_H_INCLUDED
#include "Mesh.h"

namespace Mesh {

	// スキンデータ.
	struct Skin {
		std::string name;
		std::vector<int> joints;
	};

	// ノード.
	struct Node {
		Node* parent = nullptr;
		int mesh = -1;
		int skin = -1;
		std::vector<Node*> children;
		glm::mat4 matLocal = glm::mat4(1);
		glm::mat4 matGlobal = glm::mat4(1);
		glm::mat4 matInverseBindPose = glm::mat4(1);
	};

	// アニメーションのキーフレーム.
	template<typename T>
	struct KeyFrame {
		float frame;
		T value;
	};

	// アニメーションのタイムライン.
	template<typename T>
	struct Timeline {
		int targetNodeId;
		std::vector<KeyFrame<T>> timeline;
	};

	// アニメーション.
	struct Animation {
		std::vector<Timeline<glm::vec3>> translationList;
		std::vector<Timeline<glm::quat>> rotationList;
		std::vector<Timeline<glm::vec3>> scaleList;
		float totalTime = 0;
		std::string name;
	};

	// シーン.
	struct Scene {
		int rootNode;
		std::vector<const Node*> meshNodes;
	};

	// 拡張ファイル.
	struct ExtendedFile {
		std::string name; // ファイル名.
		std::vector<Mesh> meshes;
		std::vector<Material> materials;

		std::vector<Scene> scenes;
		std::vector<Node> nodes;
		std::vector<Skin> skins;
		std::vector<Animation> animations;
	};
	using ExtendedFilePtr = std::shared_ptr<ExtendedFile>;

	/**
	* 骨格アニメーションをするメッシュ.
	*
	* アニメーション制御機能を含むため、インスタンスはその都度作成せざるを得ない.
	*/
	class SkeletalMesh
	{
	public:
		enum class State {
			stop, ///< 停止中.
			play, ///< 再生中.
			pause, ///< 一時停止中.
		};

		SkeletalMesh() = default;
		SkeletalMesh(const ExtendedFilePtr& f, const Node* n);

		void Update(float deltaTime, const glm::mat4& matModel, const glm::vec4& color);
		void Draw(DrawType drawType) const;
		const std::vector<Animation>& GetAnimationList() const;
		const std::string& GetAnimation() const;
		float GetTotalAnimationTime() const;
		State GetState() const;
		bool Play(const std::string& name, bool loop = true);
		bool Stop();
		bool Pause();
		bool Resume();
		void SetAnimationSpeed(float speed);//animationSpeed.
		float GetAnimationSpeed() const;
		void SetPosition(float);
		float GetPosition() const;
		bool IsFinished() const;
		bool Loop() const;
		void Loop(bool);

	private:
		ExtendedFilePtr file;
		const Node* node = nullptr;
		const Animation* animation = nullptr;

		State state = State::stop;
		float frame = 0;
		float animationSpeed = 1;
		bool loop = true;

		GLintptr uboOffset = 0;
		GLsizeiptr uboSize = 0;
	};
	using SkeletalMeshPtr = std::shared_ptr<SkeletalMesh>;

	/**
	* SkeletalMeshの使い方:
	*
	* main関数への追加:
	* 1. main関数の、OpenGLの初期化が完了したあと、シーンを作成するより前にMesh::SkeletalAnimation::Initialize()を実行.
	* 2. main関数の終了直前にMesh::SkeletalAnimation::Finalize()を実行.
	* 3. SceneStack::Update関数呼び出しの直前にMesh::SkeletalAnimation::ResetUniformData()を実行.
	* 4. SceneStack::Update関数呼び出しの直後にMesh::SkeletalAnimation::UploadUniformData()を実行.
	*
	* Mesh::Bufferクラスへの追加:
	* 1. Mesh::Material構造体にスケルタルメッシュ用シェーダポインタを追加.
	* 2. Mesh::Bufferクラスにスケルタルメッシュ用のシェーダを読み込む処理を追加.
	* 3. Mesh::Buffer::Init関数の最後でBindUniformBlockを実行.
	* 4. Mesh::BufferクラスにLoadSkeletalMesh関数、GetSkeletalMesh関数を追加.
	*
	* ゲーム中:
	*
	*/
	namespace SkeletalAnimation {

		bool Initialize();
		void Finalize();
		bool BindUniformBlock(const Shader::ProgramPtr&);
		void ResetUniformData();
		void UploadUniformData();

	} // namespace SkeletalAnimation

} // namespace Mesh

#endif // SKELETAL_MESH_H_INCLUDED