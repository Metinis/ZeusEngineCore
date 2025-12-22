#pragma once
#include "ZeusEngineCore/StreamReader.h"

namespace ZEN {
    class FileStreamReader : public StreamReader {
    public:
        FileStreamReader(const std::filesystem::path& path);
        ~FileStreamReader() override;
        bool isStreamGood()  override;
        uint64_t getStreamPosition()  override;
        void setStreamPosition(uint64_t position) override;
        bool readData(char* destination, size_t size)  override;
    private:
        std::filesystem::path m_Path{};
        std::ifstream m_Stream;
    };
}


