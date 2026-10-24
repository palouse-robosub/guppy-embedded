#ifndef GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP
#define GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP

#include <cstdint>
#include <string_view>

namespace firmware_version
{

inline constexpr std::uint32_t report_interval_ms = 1000;

/**
 * Parse v.XX.XX.XX, with exactly two decimal digits per component.
 * Pack the version as 0x00MMmmpp (major, minor, patch).
 * The RP2350 CAN transmitter sends this integer little-endian:
 * patch, minor, major, 0. Invalid formats return 0xFFFFFFFFu; callers must skip
 * version transmission when parsing fails.
 * Git describe suffixes are rejected so development builds are not silently
 * reported as the exact tagged release.
 */
constexpr std::uint32_t parse(std::string_view text)
{
    if (text.size() != 10 || text[0] != 'v' || text[1] != '.'
        || text[4] != '.' || text[7] != '.')
    {
        return 0xFFFFFFFFu;
    }

    std::uint32_t packed = 0;
    for (std::size_t position = 2; position < text.size(); position += 3)
    {
        const char tens = text[position];
        const char units = text[position + 1];
        if (tens < '0' || tens > '9' || units < '0' || units > '9')
        {
            return 0xFFFFFFFFu;
        }

        const auto component = static_cast<std::uint32_t>(
            (tens - '0') * 10 + (units - '0'));
        packed = (packed << 8) | component;
    }

    return packed;
}

} // namespace firmware_version

#endif // GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP
