#ifndef GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP
#define GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace firmware_version
{

    /** @brief Parse a firmware version string
     *  @param text The version string to parse, expected in the format "v.X.Y.Z" where X, Y, and Z are decimal numbers 0 to 255
     *  @return The packed version number, or an error with upper bytes 0xFF:
     *  0xFF000001u: invalid length, prefix, or trailing separator
     *  0xFF000002u: non-digit character in a component
     *  0xFF000003u: component exceeds 255
     *  0xFF000004u: empty component
     *  0xFF000005u: component count is not three
     */
    constexpr uint32_t parse(std::string_view text)
    {
        // check for minimum length and starts with v
        if (text.size() < 7 || text[0] != 'v' || text[1] != '.' || text.back() == '.')
        {
            return 0xFF000001u;
        }

        // parse each component
        uint32_t packed = 0;
        int components = 0;
        for (size_t position = 2; position < text.size(); ++position)
        {
            const size_t start = position;
            uint32_t component = 0;
            
            // parse digits until the next . or end
            while (position < text.size() && text[position] != '.')
            {
                // check if the character is digit
                const char digit = text[position];
                if (digit < '0' || digit > '9')
                {
                    return 0xFF000002u;
                }
                
                // add the digit to the component
                component = component * 10 + (digit - '0');
                if (component > 0xFF)
                {
                    return 0xFF000003u;
                }

                ++position;
            }

            if (position == start) // empty component, e.g. "v.1..2"
            {
                return 0xFF000004u;
            }

            packed = (packed << 8) | component;
            ++components;
        }

        // check that we have exactly 3 components (major, minor, patch)
        if (components != 3)
        {
            return 0xFF000005u;
        }

        return packed;
    }

} // namespace firmware_version

#endif // GUPPY_EMBEDDED_FIRMWARE_VERSION_HPP
