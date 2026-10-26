#ifndef GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP
#define GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace firmware_version
{

    /*
    Parse v.X.Y.Z, where each component can be 0 to 255 (for example v.1.25.134).
    Pack the version as 0x00MMmmpp (major, minor, patch).
    Return 0xFFFFFFFFu on error (if the version format is wrong).
    */
    constexpr std::uint32_t parse(std::string_view text)
    {
        // check for minimum length and starts with v
        if (text.size() < 7 || text[0] != 'v' || text[1] != '.' || text.back() == '.')
        {
            return 0xFFFFFFFFu;
        }

        // parse each component
        std::uint32_t packed = 0;
        int components = 0;
        for (std::size_t position = 2; position < text.size(); ++position)
        {
            const std::size_t start = position;
            std::uint32_t component = 0;
            
            // parse digits until the next . or end
            while (position < text.size() && text[position] != '.')
            {
                // check if the character is digit
                const char digit = text[position];
                if (digit < '0' || digit > '9')
                {
                    return 0xFFFFFFFFu;
                }
                
                // add the digit to the component
                component = component * 10 + (digit - '0');
                if (component > 0xFF)
                {
                    return 0xFFFFFFFFu;
                }

                ++position;
            }

            if (position == start) // empty component, e.g. "v.1..2"
            {
                return 0xFFFFFFFFu;
            }

            packed = (packed << 8) | component;
            ++components;
        }

        // check that we have exactly 3 components (major, minor, patch)
        if (components != 3)
        {
            return 0xFFFFFFFFu;
        }

        return packed;
    }

} // namespace firmware_version

#endif // GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP