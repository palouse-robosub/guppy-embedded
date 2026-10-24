#ifndef GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP
#define GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP

#include <string_view>

namespace firmware_version
{

/**
 * Parse v.XX.XX.XX, with exactly two decimal digits per component.
 * Pack the version as 0x00MMmmpp (major, minor, patch).
 * Return 0xFFFFFFFFu on error (if version format is wrong).
 */
constexpr std::uint32_t parse(std::string_view text)
{
    if (text.size() != 10 || text[0] != 'v' || text[1] != '.'
        || text[4] != '.' || text[7] != '.')
    {
        return 0xFFFFFFFFu;
    }

    std::uint32_t packed = 0;
    for (int position = 2; position < text.size(); position += 3)
    {
        const char tens = text[position];
        const char units = text[position + 1];
        if (tens < '0' || tens > '9' || units < '0' || units > '9')
        {
            return 0xFFFFFFFFu;
        }

        const int component = std::stoi(std::string(text.substr(position, 2)));
        packed = (packed << 8) | component;
    }

    return packed;
}

} // namespace firmware_version

#endif // GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP
