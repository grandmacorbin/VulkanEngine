#include "meshComponent.h"



void MeshComponent::Render() {
    if(!mesh || !material) return;

    auto transform = GetOwner()->GetComponent<TransformComponent>();
    if(!transform) return;

    //render mesh with material and transform
    material->Bind();
    material->SetUniform("modelMatrix", transform->GetTransformMatrix());
    mesh->Render();
};