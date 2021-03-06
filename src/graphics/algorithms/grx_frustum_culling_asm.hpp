#pragma once

#include <cstdint>

extern "C" {
    void x86_64_sse_frustum_culling(void* results, void* aabbs, void* frustum, std::size_t count, std::size_t bit_pos);
    void x86_64_avx_frustum_culling(void* results, void* aabbs, void* frustum, std::size_t count, std::size_t bit_pos);
}
