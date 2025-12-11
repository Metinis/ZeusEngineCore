#pragma once

namespace ZEN {
    class StreamReader {
    public:
        virtual ~StreamReader() = default;
        virtual bool isStreamGood() = 0;
        virtual uint64_t getStreamPosition() = 0;
        virtual void setStreamPosition(uint64_t position) = 0;
        virtual bool readData(char* destination, size_t size) = 0;

        template<typename T>
        void readRaw(T& type) {
            readData((char*)&type, sizeof(T));
        }

        template<typename T>
        void readObject(T& obj) {
            T::deserialize(this, obj);
        }

        template<typename T>
        void readVector(std::vector<T>& vec, bool readSize = true) {
            if(readSize) {
                uint32_t count;
                readRaw(count);
                vec.resize(count);
            }
            for(auto& e : vec) {
                if constexpr (std::is_trivial<T>()) {
                    readRaw(e);
                }
                else {
                    readObject(e);
                }
            }

        }
    };
};