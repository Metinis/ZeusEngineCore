#include "DescriptorBuffer.h"
#include "APIBackend.h"
#include <glad/glad.h>

using namespace ZEN::OGLAPI;
DescriptorBuffer::DescriptorBuffer(const BufferCreateInfo& bufferCreateInfo) {
    if(bufferCreateInfo.type == eDescriptorBufferType::UBO){
        m_Binding = 0;
    }
    else{
        m_Binding = 1;
    }
    glGenBuffers(1, &m_BufferHandle);
    glBindBuffer(GL_UNIFORM_BUFFER, m_BufferHandle);
    m_Size = 1;
    glBufferData(GL_UNIFORM_BUFFER, m_Size, nullptr, GL_DYNAMIC_DRAW); // allocate memory
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void DescriptorBuffer::Write(std::span<const std::byte> bytes) {

    glBindBuffer(GL_UNIFORM_BUFFER, m_BufferHandle);
    if(m_Size < bytes.size()){
        m_Size = bytes.size();
        glBufferData(GL_UNIFORM_BUFFER, m_Size, nullptr, GL_DYNAMIC_DRAW);
    }
    glBufferSubData(GL_UNIFORM_BUFFER, 0, bytes.size(), bytes.data());
}

void DescriptorBuffer::Bind() {
    glBindBuffer(GL_UNIFORM_BUFFER, m_BufferHandle);
    glBindBufferBase(GL_UNIFORM_BUFFER, m_Binding, m_BufferHandle);
}
