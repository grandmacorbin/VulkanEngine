

#include "Mesh.h"


bool Mesh::doLoad() 
{
    std::string filePath = "models/" + GetId() + ".gltf";

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    if(!LoadMeshData){
        return false;
    }

    CreateVertexBuffer(vertices);
    CreateIndexBuffer(indices);

    vertexCount = static_cast<uint32_t>(vertices.size());
    indexCount = static_cast<uint32_t>(indices.size());

    return Resource::Load();
}

bool Mesh::doUnload() 
{
    if(IsLoaded())
    {   
        Resource::Unload();
    }
}

bool Mesh::LoadMeshData(const std::string& filePath, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices){

}
    
void Mesh::CreateVertexBuffer(const std::vector<Vertex>& vertices){

    vk::BufferCreateInfo bufferInfo;
    bufferInfo.size = static_cast<uint32_t>(vertices.size());
    bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    vertexBuffer = vk::raii::Buffer(GetDevice(), bufferInfo);

    vk::MemoryRequirements memRequirements = vertexBuffer.getMemoryRequirements();


}
    
void Mesh::CreateIndexBuffer(const std::vector<uint32_t>& indices){
    vk::BufferCreateInfo bufferInfo;
    bufferInfo.size = static_cast<uint32_t>(indices.size());
    bufferInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer;
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    indexBuffer = vk::raii::Buffer(GetDevice(), bufferInfo);

    vk::MemoryRequirements memRequirements = indexBuffer.getMemoryRequirements();
    vk::MemoryAllocateInfo memoryAllocateInfo;
    memoryAllocateInfo.allocationSize = memRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    vertexBufferMemory = vk::raii::DeviceMemory(GetDevice(), memoryAllocateInfo);
    vertexBuffer.bindMemory(*vertexBufferMemory, 0);
}

uint32_t Mesh::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties){

}
    
vk::raii::Device Mesh::GetDevice(){

}