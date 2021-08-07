#pragma once

#include "Defines.hpp"

#include <new>
#include <utility>

template<typename T>
struct Array {
    T* Data      = nullptr;
    u64 Length   = 0;
    u64 Capacity = 0;

    T& operator[](u64 index) {
        return this->Data[index];
    }

    const T& operator[](u64 index) const {
        return this->Data[index];
    }
};

template<typename T>
Array<T> Array_Create() {
    return {};
}

template<typename T>
void Array_Destroy(Array<T>& array) {
    for (u64 i = 0; i < array.Length; i++) {
        array[i].~T();
    }
    ::operator delete(array.Data, array.Capacity * sizeof(T));
    array = {};
}

template<typename T>
void Array_Grow(Array<T>& array, u64 newCapacity) {
    if (array.Capacity >= newCapacity) {
        return;
    }

    T* newData = (T*)::operator new(newCapacity * sizeof(T));

    for (u64 i = 0; i < array.Length; i++) {
        new (&newData[i]) T(std::move(array[i]));
    }

    ::operator delete(array.Data, array.Capacity * sizeof(T));
    array.Data     = newData;
    array.Capacity = newCapacity;
}

template<typename T>
T& Array_Add(Array<T>& array, const T& value) {
    if (array.Length >= array.Capacity) {
        Array_Grow(array, array.Capacity == 0 ? 1 : array.Capacity * 2);
    }

    new (&array[array.Length]) T(value);
    return array[array.Length++];
}
