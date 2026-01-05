#pragma once
#include "StreamWriter.h"

namespace ZEN {
    class FileStreamWriter : public StreamWriter {
    public:
        FileStreamWriter(const std::filesystem::path& path);
        ~FileStreamWriter() override;;
        bool writeData(const char* data, size_t size) override;
        bool isStreamGood() override;;
        void setStreamPosition(uint64_t position) override;;
        uint64_t getStreamPosition() override;;
    private:
        std::filesystem::path m_Path{};
        std::ofstream m_Stream{};
    };
}
