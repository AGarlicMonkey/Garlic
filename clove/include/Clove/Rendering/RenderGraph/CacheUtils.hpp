#pragma once

namespace garlic::clove::CacheUtils {
    template<typename HashType, typename T>
    void hashCombine(HashType &currentHash, T const &value);
}

#include "CacheUtils.inl"