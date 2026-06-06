

#include "RenderGraph.h"


 void RenderGraph::AddResource(const std::string& name, vk::Format format, vk::Extent2D extent,
                    vk::ImageUsageFlags usage, vk::ImageLayout initialLayout, 
                    vk::ImageLayout finalLayout) {
                        Resource resource;
                        resource.name = name;
                        resource.format = format;
                        resource.extent = extent;
                        resource.usage = usage;
                        resource.initialLayout = initialLayout;
                        resource.finalLayout = finalLayout;

                        resources.emplace(name, std::move(resource));
                    }


void RenderGraph::AddPass(const std::string& name,
                const std::vector<std::string>& inputs,
                const std::vector<std::string>& outputs,
                std::function<void(vk::raii::CommandBuffer&)> executeFunc) {
                    Pass pass;
                    pass.name = name;
                    pass.inputs = inputs;
                    pass.outputs = outputs;
                    pass.executeFunc = executeFunc;

                    passes.push_back(std::move(pass));
                }


void RenderGraph::Compile() {
    //build bidrectional dependency relationships between passes
    // Dependencies are the passes that must run before a pass
    std::vector<std::vector<size_t>> dependencies(passes.size());
    // dependents are the passes that need this pass to run
    std::vector<std::vector<size_t>> dependents(passes.size());

    // track which pass produces each resources
    std::unordered_map<std::string, size_t> resourceWriters;

    // analyze eac hpass to determine data flow relationships
    for(size_t i = 0; i < passes.size(); ++i) {
        const auto& pass = passes[i];

        for(const auto& input : pass.inputs) {
            auto it = resourceWriters.find(input);
            if(it != resourceWriters.end()) {
                dependencies[i].push_back(it->second);
                dependents[it->second].push_back(i);
            }
        }

        for(const auto& output : pass.outputs) {
            resourceWriters[output] = i;
        }
    }

    // topological Sort for optimal execution order
    // use depth-First search to compute valid execution sequence while detecting cycles
    std::vector<bool> visited(passes.size(), false);
    std::vector<bool> inStack(passes.size(), false);

    std::function<void(size_t)> visit = [&](size_t node) {
        if(inStack[node]) {
            throw std::runtime_error("Cycle detected in rendergraph");
        }

        if(visited[node]) {
            return;
        }

        inStack[node] = true;

        for(auto dependent : dependents[node]) {
            visit(dependent);
        }

        inStack[node] = false;
        visited[node] = true;
        executionOrder.push_back(node);
    };

    // visit all disconnected components
    for(size_t i = 0; i < passes.size(); i++) {
        if(!visited[i]) {
            visit(i);
        }
    }

    std::reverse(executionOrder.begin(), executionOrder.end());

    for(size_t i = 0; i < passes.size(); ++i) {
        for(auto dep : dependencies[i]) {
            semaphores.emplace_back(device.createSemaphore({}));
            semaphoreSignalWaitPairs.emplace_back(dep, i);
        }
    }

    for(auto& [name, resource] : resources) {
        vk::ImageCreateInfo imageInfo;
        imageInfo.setImageType(vk::ImageType::e2D)
            .setFormat(resource.format)
            .setExtent({resource.extent.width, resource.extent.height, 1})
            .setMipLevels(1)
            .setArrayLayers(1)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(vk::ImageTiling::eOptimal)
            .setUsage(resource.usage)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setInitialLayout(vk::ImageLayout::eUndefined);

        resource.image = device.createImage(imageInfo);

        vk::MemoryRequirements memRequirements = resource.image.getMemoryRequirements();

        vk::MemoryAllocateInfo allocInfo;
        allocInfo.setAllocationSize(memRequirements.size)
            .setMemoryTypeIndex(FindMemoryType(memRequirements.memoryTypeBits,
                 vk::MemoryPropertyFlagBits::eDeviceLocal));

        resource.memory = device.allocateMemory(allocInfo);
        resource.image.bindMemory(*resource.memory, 0);

        vk::ImageViewCreateInfo viewInfo;
        viewInfo.setImage(*resource.image)
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(resource.format)
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    }

}

void RenderGraph::Execute(vk::raii::CommandBuffer& commandBuffer, vk::Queue queue) {
    std::vector<vk::CommandBuffer> cmdBuffers;
    std::vector<vk::Semaphore> waitSemaphores;
    std::vector<vk::PipelineStageFlags> waitStages;
    std::vector<vk::Semaphore> signalSemaphores;

    for(auto passIdx : executionOrder) {
        const auto& pass = passes[passIdx];

        waitSemaphores.clear();
        waitStages.clear();

        for(size_t i = 0; i < semaphoreSignalWaitPairs.size(); ++i) {
            if(semaphoreSignalWaitPairs[i].second == passIdx) {
                waitSemaphores.push_back(*semaphores[i]);
                waitStages.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
            }
        }

        commandBuffer.begin({});

        for(const auto& input : pass.inputs) {
            auto& resource = resources[input];

            vk::ImageMemoryBarrier barrier;
            barrier.setOldLayout(resource.initialLayout)
                    .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                    .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                    .setImage(*resource.image)
                    .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
                    .setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead);
            
            commandBuffer.pipelineBarrier(
                    vk::PipelineStageFlagBits::eAllCommands,                           
                    vk::PipelineStageFlagBits::eFragmentShader,                        
                    vk::DependencyFlagBits::eByRegion,                                 
                    {}, {}, barrier                               
                );
        }

        for (const auto& output : pass.outputs) {
            auto& resource = resources[output];

            vk::ImageMemoryBarrier barrier;
            barrier.setOldLayout(resource.initialLayout)                           
                    .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)         
                    .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                    .setImage(*resource.image)
                    .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
                    .setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)             
                    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);   

                // Insert barrier for safe transition to writable state
            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eAllCommands,
                vk::PipelineStageFlagBits::eColorAttachmentOutput,                 
                vk::DependencyFlagBits::eByRegion,
                {}, {}, barrier
            );
        }
        
        pass.executeFunc(commandBuffer);
        
        for(const auto& output : pass.outputs) {
            auto& resource = resources[output];

            vk::ImageMemoryBarrier barrier;
            barrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
                    .setNewLayout(resource.finalLayout)
                    .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored)
                    .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
                    .setImage(*resource.image)
                    .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
                    .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
            
            commandBuffer.pipelineBarrier(
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,                          
                    vk::PipelineStageFlagBits::eAllCommands,                        
                    vk::DependencyFlagBits::eByRegion,                                
                    {}, {}, barrier                               
                );
        }

        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
            .setPWaitSemaphores(waitSemaphores.data())
            .setPWaitDstStageMask(waitStages.data())
            .setCommandBufferCount(1)
            .setPCommandBuffers(&*commandBuffer)
            .setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
            .setPSignalSemaphores(signalSemaphores.data());

        queue.submit(1, &submitInfo, nullptr);
    }
}

uint32_t RenderGraph::FindMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    return 0;
}