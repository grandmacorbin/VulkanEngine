
#include "entity.h"
#include "component.h"
#include "transformComponent.h"

struct Mesh
{

};

struct Material
{

};

class MeshComponent : public Component {
private:
    //TODO: make the Mesh and Material Structs
    Mesh* mesh = nullptr;
    Material* material = nullptr;

public:
    MeshComponent(Mesh* m, Material* mat) : mesh(m), material(mat) {}

    void SetMesh(Mesh* m) { mesh = m; }
    void SetMaterial(Material* mat) { material = mat; }

    Mesh* GetMesh() const { return mesh; }
    Material* GetMaterial() const { return material; }

    void Render() override;
};