#pragma once

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cassert>

#define ASSERT(x) assert(x)

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using f32 = float;
using f64 = double;

#define Alloc(size) std::malloc(size)

#define Dealloc(ptr) std::free(size)

#define Error(message, ...)                           \
    do {                                              \
        std::fflush(stdout);                          \
        std::fprintf(stderr, message, ##__VA_ARGS__); \
        std::fprintf(stderr, "\n");                   \
        std::fflush(stderr);                          \
        std::exit(-1);                                \
    } while (0)

#define Print(message, ...)                           \
    do {                                              \
        std::fflush(stderr);                          \
        std::fprintf(stdout, message, ##__VA_ARGS__); \
        std::fflush(stdout);                          \
    } while (0)

#define PrintError(message, ...)                      \
    do {                                              \
        std::fflush(stdout);                          \
        std::fprintf(stderr, message, ##__VA_ARGS__); \
        std::fflush(stderr);                          \
    } while (0)
