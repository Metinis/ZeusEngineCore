#include "Utils.h"
#include <fstream>
#include <sstream>
std::string readFile(const std::filesystem::path& filePath) {
    const std::ifstream file(filePath);
    if(!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
