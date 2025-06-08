#include "Utils.h"
#include <fstream>
#include <sstream>
namespace fs = std::filesystem;

std::string readFile(const std::filesystem::path& filePath) {
    fs::path absolutePath = fs::absolute(filePath).lexically_normal();

    std::ifstream file(absolutePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + absolutePath.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
