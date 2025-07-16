#include "ZeusEngineCore/Utils.h"
#include <fstream>
#include <sstream>
namespace fs = std::filesystem;
using namespace ZEN;

std::string ZEN::ReadFile(const std::filesystem::path& filePath) {
    fs::path absolutePath = fs::absolute(filePath).lexically_normal();

    std::ifstream file(absolutePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + absolutePath.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::uint32_t> ZEN::ToSpirV(const std::filesystem::path& filePath) {
    if (filePath.empty()) {
        throw std::runtime_error("SPIR-V file path is empty.");
    }

    fs::path absolutePath = fs::absolute(filePath).lexically_normal();

    std::ifstream file(absolutePath, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + absolutePath.string());
    }

    // Seek to the end to get size
    file.seekg(0, std::ios::end);
    auto const size = file.tellg();
    if (size < 0) {
        throw std::runtime_error("Failed to get file size: " + absolutePath.string());
    }
    auto const usize = static_cast<std::uint64_t>(size);

    if (usize % sizeof(std::uint32_t) != 0) {
        throw std::runtime_error{ std::format("Invalid SPIR-V size: {}", usize) };
    }

    // Seek back to beginning before reading
    file.seekg(0, std::ios::beg);

    std::vector<std::uint32_t> spirvData(usize / sizeof(std::uint32_t));
    file.read(reinterpret_cast<char*>(spirvData.data()), size);
    if (!file) {
        throw std::runtime_error("Failed to read SPIR-V file: " + absolutePath.string());
    }

    return spirvData;
}

void ZEN::RequireSuccess(vk::Result const result, char const* errorMsg)
{
    if (result != vk::Result::eSuccess) { throw std::runtime_error{ errorMsg }; }
}
