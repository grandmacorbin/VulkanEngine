
#include "texture.h"


bool Texture::doLoad() 
{
    std::string filePath = "textures/" + GetId() + ".ktx";

    unsigned char* data = LoadImageData(filePath, &width, &height, &channels);
    if(!data)
    {
        return false;
    }

    CreateVulkanImage(data, width, height, channels);

    FreeImageData(data);

    return Resource::Load();
}

bool Texture::doUnload() 
{
    if(IsLoaded())
    {
        vk::Device device = GetDevice();

        device.destroySampler(sampler);
        device.destroyImageView(imageView);
        device.destroyImage(image);
        device.freeMemory(memory);

        Resource::Unload();
    }
}

unsigned char* Texture::LoadImageData(const std::string& filepath, int* width, int* height, int* channles)
{
    
}
    
void Texture::FreeImageData(unsigned char* data)
{

}

void Texture::CreateVulkanImage(unsigned char* data, int width, int height, int channels)
{

}