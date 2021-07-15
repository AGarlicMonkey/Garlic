namespace garlic::clove::CacheUtils {
    template<typename HashType, typename T>
    void hashCombine(HashType &currentHash, T const &value) {
        std::hash<T> hash{};
        currentHash ^= hash(value) + 0x9e3779b9 + (currentHash << 6) + (currentHash >> 2);
    }
}