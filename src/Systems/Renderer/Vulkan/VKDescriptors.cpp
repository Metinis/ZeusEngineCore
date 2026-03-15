#include "VKDescriptors.h"
#include <vulkan/vk_enum_string_helper.h>
#include "ZeusEngineCore/engine/rendering/VKUtils.h"

using namespace ZEN;
void DescriptorLayoutBuilder::addBinding(uint32_t binding, VkDescriptorType type) {
    VkDescriptorSetLayoutBinding newBinding{};
    newBinding.binding = binding;
    newBinding.descriptorCount = 1;
    newBinding.descriptorType = type;

    bindings.push_back(newBinding);
}

void DescriptorLayoutBuilder::clear() {
    bindings.clear();
}

VkDescriptorSetLayout DescriptorLayoutBuilder::build(VkDevice device, VkShaderStageFlags shaderStages, void *pNext,
    VkDescriptorSetLayoutCreateFlags flags) {

    for (auto& b : bindings) {
        b.stageFlags |= shaderStages;
    }

    VkDescriptorSetLayoutCreateInfo info { .sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

    info.pNext = pNext;
    info.pBindings = bindings.data();
    info.bindingCount = (uint32_t)bindings.size();
    info.flags = flags;

    VkDescriptorSetLayout set;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &info, nullptr, &set));
    return set;
}

void DescriptorAllocatorGrowable::init(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
    //allocate the first descriptor pool
    ratios.clear();
    for (auto ratio : poolRatios) {
        ratios.push_back(ratio);
    }
    VkDescriptorPool newPool = createPool(device, maxSets, poolRatios);
    setsPerPool = maxSets * 1.5;
    readyPools.push_back(newPool);
}

void DescriptorAllocatorGrowable::clearPools(VkDevice device) {
    for (auto p : readyPools) {
        vkResetDescriptorPool(device, p ,0);
    }
    for (auto p : fullPools) {
        vkResetDescriptorPool(device, p, 0);
        readyPools.push_back(p);
    }
    fullPools.clear();
}

void DescriptorAllocatorGrowable::destroyPools(VkDevice device) {
    for (auto p : readyPools) {
        vkDestroyDescriptorPool(device, p, 0);
    }
    readyPools.clear();
    for (auto p : fullPools) {
        vkDestroyDescriptorPool(device, p, 0);
    }
    fullPools.clear();
}

VkDescriptorSet DescriptorAllocatorGrowable::allocate(VkDevice device, VkDescriptorSetLayout layout, void *pNext) {
    VkDescriptorPool poolToUse = getPool(device);
    VkDescriptorSetAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = pNext;
    allocInfo.descriptorPool = poolToUse;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet set;
    VkResult result = vkAllocateDescriptorSets(device, &allocInfo, &set);

    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        //this is when pool is full, we get a new one
        fullPools.push_back(poolToUse);
        poolToUse = getPool(device);
        allocInfo.descriptorPool = poolToUse;
        VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &set));
    }
    readyPools.push_back(poolToUse);
    return set;
}

VkDescriptorPool DescriptorAllocatorGrowable::getPool(VkDevice device) {
    //when we create a new pool, mimic vector resizing
    //create a new pool when we hit a limit
    VkDescriptorPool newPool;
    if (!readyPools.empty()) {
        newPool = readyPools.back();
        readyPools.pop_back();
    } else {
        newPool = createPool(device, setsPerPool, ratios);
        setsPerPool *= 1.5;
        if (setsPerPool > 4092) {
            setsPerPool = 4092;
        }
    }
    return newPool;
}

VkDescriptorPool DescriptorAllocatorGrowable::createPool(VkDevice device, uint32_t setCount,
    std::span<PoolSizeRatio> poolRatios) {
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (PoolSizeRatio ratio : poolRatios) {
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = ratio.type,
            .descriptorCount = (uint32_t)(ratio.ratio * setCount),
        });
    }
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = 0;
    poolInfo.maxSets = setCount;
    poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();

    VkDescriptorPool newPool;
    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &newPool));
    return newPool;
}

void DescriptorWriter::writeImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout,
    VkDescriptorType type) {
    VkDescriptorImageInfo& info = imageInfos.emplace_back();
    info.sampler = sampler;
    info.imageView = image;
    info.imageLayout = layout;

    VkWriteDescriptorSet write = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.dstBinding = binding;
    write.dstSet = VK_NULL_HANDLE;
    write.descriptorType = type;
    write.descriptorCount = 1;
    write.pImageInfo = &info;
    writes.push_back(write);
}

void DescriptorWriter::writeBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type) {
    VkDescriptorBufferInfo& info = bufferInfos.emplace_back();
    info.buffer = buffer;
    info.offset = offset;
    info.range = size;

    VkWriteDescriptorSet write = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    write.dstBinding = binding;
    write.dstSet = VK_NULL_HANDLE;
    write.descriptorType = type;
    write.descriptorCount = 1;
    write.pBufferInfo = &info;
    writes.push_back(write);

}

void DescriptorWriter::clear() {
    imageInfos.clear();
    bufferInfos.clear();
    writes.clear();
}

void DescriptorWriter::updateSet(VkDevice device, VkDescriptorSet set) {
    for (VkWriteDescriptorSet& write : writes) {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(device,
        (uint32_t)(writes.size()),
        writes.data(),
        0,
        nullptr);
}

void DescriptorAllocator::initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios) {
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (PoolSizeRatio ratio : poolRatios) {
        poolSizes.push_back(VkDescriptorPoolSize{
            .type = ratio.type,
            .descriptorCount = (uint32_t)(ratio.ratio * maxSets),
        });
    }
    VkDescriptorPoolCreateInfo poolInfo {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    poolInfo.flags = 0;
    poolInfo.maxSets = maxSets;
    poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();

    vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
}

void DescriptorAllocator::clearDescriptors(VkDevice device) {
    vkResetDescriptorPool(device, pool, 0);
}

void DescriptorAllocator::destroyPool(VkDevice device) {
    vkDestroyDescriptorPool(device, pool, nullptr);
}

VkDescriptorSet DescriptorAllocator::allocate(VkDevice device, VkDescriptorSetLayout layout) {
    VkDescriptorSetAllocateInfo info {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    info.pNext = nullptr;
    info.descriptorPool = pool;
    info.descriptorSetCount = 1;
    info.pSetLayouts = &layout;

    VkDescriptorSet set;
    VK_CHECK(vkAllocateDescriptorSets(device, &info, &set));
    return set;
}
