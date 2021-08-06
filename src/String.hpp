#pragma once

#include "Defines.hpp"

struct String {
    u8* Data;
    u64 Length;

    String() : Data(nullptr), Length(0) {}
    String(const char* cstring) : Data((u8*)cstring), Length(std::strlen(cstring)) {}
    String(u8* data, u64 length) : Data(data), Length(length) {}

    u8& operator[](u64 index) {
        return this->Data[index];
    }

    const u8& operator[](u64 index) const {
        return this->Data[index];
    }
};
