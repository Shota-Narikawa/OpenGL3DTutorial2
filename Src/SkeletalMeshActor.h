/**
* @file SkeletalMeshActor.h
*/
#ifndef SKELETALMESHACTOR_H_INCLUDED
#define SKELETALMESHACTOR_H_INCLUDED
#include "Actor.h"
#include "SkeletalMesh.h"

/**
* スケルタルメッシュ用アクター.
*/
class SkeletalMeshActor : public Actor
{
public:
  SkeletalMeshActor(const Mesh::SkeletalMeshPtr& m, const std::string& name, int hp, const glm::vec3& pos,
    const glm::vec3& rot = glm::vec3(0), const glm::vec3& scale = glm::vec3(1));
  virtual ~SkeletalMeshActor() = default;

  virtual void UpdateDrawData(float) override;
  virtual void Draw(Mesh::DrawType drawType) override;

  const Mesh::SkeletalMeshPtr& GetMesh() const { return mesh; }
  void SetMesh(Mesh::SkeletalMeshPtr p, const int meshType) { mesh = p; sMesh = nullptr; }
  void SetStaticMesh(Mesh::FilePtr p, const int meshType) { sMesh = p; mesh = nullptr; }
 

  void SetPointLightList(const std::vector<int>& v);
  void SetSpotLightList(const std::vector<int>& v);

private:
  Mesh::SkeletalMeshPtr mesh;
  Mesh::FilePtr sMesh;

  int pointLightCount = 0;
  int pointLightIndex[8] = {};
  int spotLightCount = 0;
  int spotLightIndex[8] = {};
};
using SkeletalMeshActorPtr = std::shared_ptr<SkeletalMeshActor>;

#endif // SKELETALMESHACTOR_H_INCLUDED