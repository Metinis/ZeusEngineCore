
#include "ZeusEngineCore/FileStreamWriter.h"

ZEN::FileStreamWriter::FileStreamWriter(const std::filesystem::path &path) : m_Path(path),
m_Stream(path, std::ios::binary | std::ios::trunc) {

}

ZEN::FileStreamWriter::~FileStreamWriter() {
    m_Stream.close();
}

bool ZEN::FileStreamWriter::writeData(const char *data, size_t size) {
    m_Stream.write(data, size);
    return true;
}

bool ZEN::FileStreamWriter::isStreamGood() {
    return m_Stream.good();
}

void ZEN::FileStreamWriter::setStreamPosition(uint64_t position) {
    m_Stream.seekp(position);
}

uint64_t ZEN::FileStreamWriter::getStreamPosition() {
    return m_Stream.tellp();
}
