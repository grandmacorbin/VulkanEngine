

#include "vulkan_raii.hpp"
#include "resourceManager.h"

class Shader : public Resource {
private:
    vk::ShaderModule shaderModule;
    vk::ShaderStageFlagBits stage;

public:
    Shader(const std::string& id, vk::ShaderStageFlagBits shaderStage)
        : Resource(id), stage(shaderStage) {}

    ~Shader() override{
        Unload();
    }

protected:
    bool doLoad() override;
    bool doUnload() override;

public:
    vk::ShaderModule GetShaderModule() const { return shaderModule; }
    vk::ShaderStageFlagBits GetStage() const { return stage; }

private:
    bool readFile(const std::string& filePath, std::vector<char>& buffer);
    void createShaderModule(const std::vector<char>& code);
};