#ifndef __NETWORK_UTILITIES_H__
#define __NETWORK_UTILITIES_H__

#include <algorithm>    // std::reverse
#include <concepts>
#include <cstddef>      // std::byte
#include <cstdlib>      // std::size_t

namespace Network {

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
    requires Numeric<T>
constexpr T swap_endianness(T number) noexcept {
    std::byte *bytes = reinterpret_cast<std::byte*>(&number);
    std::reverse(bytes, bytes + sizeof(T));
    return number;
}

} // namespace Network

#endif // __NETWORK_UTILITIES_H__
