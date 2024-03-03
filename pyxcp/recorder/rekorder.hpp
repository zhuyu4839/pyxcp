
#if !defined(__REKORDER_HPP)
    #define __REKORDER_HPP

    #if !defined(STANDALONE_REKORDER)
        #define STANDALONE_REKORDER 0
    #endif /* STANDALONE_REKORDER */

    #include <array>
    #include <atomic>
    #include <bit>
    #include <bitset>
    #include <cerrno>
    #include <cstdint>
    #include <cstdio>
    #include <cstdlib>
    #include <cstring>
    #include <ctime>
    #include <exception>
    #include <functional>
    #include <optional>
    #include <stdexcept>
    #include <string>
    #include <thread>
    #include <utility>
    #include <variant>
    #include <vector>

    #include "blockmem.hpp"
    #include "event.hpp"
    #include "tsqueue.hpp"

    #if defined(_WIN32)
        #include <Windows.h>
        #include <fcntl.h>
        #include <io.h>
    #endif /* _WIN32 */

    #include "lz4.h"
    #include "mio.hpp"

    #if STANDALONE_REKORDER == 0
        #include <pybind11/numpy.h>
        #include <pybind11/pybind11.h>
        #include <pybind11/stl.h>

namespace py = pybind11;
using namespace pybind11::literals;
    #endif /* STANDALONE_REKORDER */

    #if !defined(__BIGGEST_ALIGNMENT__)
        #define __BIGGEST_ALIGNMENT__ (8)
    #endif

    #define __ALIGNMENT_REQUIREMENT __BIGGEST_ALIGNMENT__
    #define __ALIGN                 alignas(__ALIGNMENT_REQUIREMENT)


constexpr auto kilobytes(std::uint32_t value) -> std::uint32_t {
    return value * 1024;
}

constexpr auto megabytes(std::uint32_t value) -> std::uint32_t {
    return kilobytes(value) * 1024;
}

constexpr std::uint16_t XCP_PAYLOAD_MAX = 0xFFFFUL;

constexpr std::uint16_t XMRAW_HAS_METADATA = 0x0004UL;

    /*
    byte-order is, where applicable little ending (LSB first).
    */
    #pragma pack(push)
    #pragma pack(1)

struct FileHeaderType {
    std::uint16_t hdr_size;
    std::uint16_t version;
    std::uint16_t options;
    std::uint32_t num_containers;
    std::uint32_t record_count;
    std::uint32_t size_compressed;
    std::uint32_t size_uncompressed;
};

using HeaderTuple = std::tuple<std::uint16_t, std::uint16_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, double>;

static_assert(sizeof(FileHeaderType) == 22);

struct ContainerHeaderType {
    std::uint32_t record_count;
    std::uint32_t size_compressed;
    std::uint32_t size_uncompressed;
};

using blob_t      = unsigned char;
using blob_string = std::basic_string<blob_t>;

    #if STANDALONE_REKORDER == 1
using payload_t = std::shared_ptr<blob_t[]>;
    #else
using payload_t = py::array_t<blob_t>;
    #endif /* STANDALONE_REKORDER */

struct frame_header_t {
    std::uint8_t  category{ 0 };
    std::uint16_t counter{ 0 };
    double   timestamp{ 0.0 };
    std::uint16_t length{ 0 };
};

    #pragma pack(pop)

using FrameTuple       = std::tuple<std::uint8_t, std::uint16_t, double, std::uint16_t, payload_t>;
using FrameVector      = std::vector<FrameTuple>;
using FrameTupleWriter = std::tuple<std::uint8_t, std::uint16_t, double, std::uint16_t, char*>;

enum class FrameCategory : std::uint8_t {
    META,
    CMD,
    RES,
    ERR,
    EV,
    SERV,
    DAQ,
    STIM,
};

namespace detail {
const std::string FILE_EXTENSION(".xmraw");
const std::string MAGIC{ "ASAMINT::XCP_RAW" };
constexpr auto    MAGIC_SIZE       = 16;
constexpr auto    VERSION          = 0x0100;
constexpr auto    FILE_HEADER_SIZE = sizeof(FileHeaderType);
constexpr auto    CONTAINER_SIZE   = sizeof(ContainerHeaderType);
}  // namespace detail

constexpr auto file_header_size() -> std::uint32_t {
    return (detail::FILE_HEADER_SIZE + detail::MAGIC_SIZE);
}

using rounding_func_t = std::function<std::uint32_t(std::uint32_t)>;

inline rounding_func_t create_rounding_func(std::uint32_t multiple) {
    return [multiple](std::uint32_t value) {
        return (value + (multiple - 1)) & ~(multiple - 1);
    };
}

const auto round_to_alignment = create_rounding_func(__ALIGNMENT_REQUIREMENT);

inline void _fcopy(char* dest, char const * src, std::uint32_t n) noexcept {
    for (std::uint32_t i = 0; i < n; ++i) {
        dest[i] = src[i];
    }
}

    #if STANDALONE_REKORDER == 1
inline blob_t* get_payload_ptr(const payload_t& payload) noexcept {
    return payload.get();
}

inline payload_t create_payload(std::uint32_t size, blob_t const * data) noexcept {
    auto pl = std::make_shared<blob_t[]>(size);
    _fcopy(reinterpret_cast<char*>(pl.get()), reinterpret_cast<char const *>(data), size);
    return pl;
}
    #else
inline payload_t create_payload(std::uint32_t size, blob_t const * data) {
    return py::array_t<blob_t>(size, data);
}

inline blob_t* get_payload_ptr(const payload_t& payload) noexcept {
    py::buffer_info buf = payload.request();

    return static_cast<blob_t*>(buf.ptr);
}
    #endif /* STANDALONE_REKORDER */

inline void hexdump(blob_t const * buf, std::uint16_t sz) {
    for (std::uint16_t idx = 0; idx < sz; ++idx) {
        printf("%02X ", buf[idx]);
    }
    printf("\n\r");
}

    #include "reader.hpp"
    #include "unfolder.hpp"
    #include "writer.hpp"

#endif  // __REKORDER_HPP
