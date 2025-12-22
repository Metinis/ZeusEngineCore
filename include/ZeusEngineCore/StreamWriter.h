#pragma once

namespace ZEN {
    class StreamWriter {
    public:
        virtual ~StreamWriter() = default;
        virtual bool writeData(const char* data, size_t size) = 0;
        virtual bool isStreamGood() = 0;
        virtual void setStreamPosition(uint64_t position) = 0;
        virtual uint64_t getStreamPosition() = 0;

        template<typename T>
        void writeRaw(const T& type) {
            //todo check this
            writeData((char*)& type, sizeof(T));
        }
        template<typename T>
        void writeObject(const T& obj) {
            T::serialize(this, obj);
        }
        template<typename T>
        void writeVector(const std::vector<T>& vec, bool writeSize = true) {
            if(writeSize) {
                writeRaw<uint32_t>((uint32_t)vec.size());
            }
            for(const auto& e : vec) {
                if constexpr (std::is_trivial<T>()) {
                    writeRaw<T>(e);
                }
                else {
                    writeObject<T>(e);
                }
            }
        }
    };
};