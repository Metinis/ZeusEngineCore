
#include "ZeusEngineCore/FileStreamReader.h"

ZEN::FileStreamReader::FileStreamReader(const std::filesystem::path& path) :m_Path(path), m_Stream(path, std::ios::binary) {

}

ZEN::FileStreamReader::~FileStreamReader() {
    m_Stream.close();
}

bool ZEN::FileStreamReader::isStreamGood() {
    return m_Stream.good();
}

uint64_t ZEN::FileStreamReader::getStreamPosition() {
    return m_Stream.tellg();
}

void ZEN::FileStreamReader::setStreamPosition(uint64_t position) {
    m_Stream.seekg(position);
}

bool ZEN::FileStreamReader::readData(char *destination, size_t size) {
    if (!m_Stream.read(destination, size))
        return false;
    return true;
}
