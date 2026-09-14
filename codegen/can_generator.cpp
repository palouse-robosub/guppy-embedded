#include <tinyxml2.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <bitset>
#include <fstream>
#include <filesystem>

using tinyxml2::XMLElement;

/// Returns a trimmed substring
std::string trim(const std::string& string) {
    size_t start = 0;
    size_t end = 0;
    // find left side
    for (; string[start] == ' '; start++) {}
    // find right side
    for (end = string.length(); string[end - 1] == ' '; end--) {}

    return string.substr(start, end - start);
}

bool is_valid_topic_and_identifier(const std::string& string) {
    if (string.length() == 0)
        return false;
    // first character
    if (!std::isalpha(string[0]) && !(string[0] == '_'))
        return false;
    bool last_character_was_underscore = string[0] == '_';
    // remaining characters
    for (size_t i = 1; i < string.length(); i++) {
        if (std::isalnum(string[i])) {
            last_character_was_underscore = false;
        } else if (string[i] == '_' && !last_character_was_underscore) {
            last_character_was_underscore = true;
        } else {
            // ROS2 topic names must not contain any number of repeated
            // underscores
            return false;
        }
    }
    return true;
}

const char* read_attribute(XMLElement* can_id, const char* tag_name,
                           const std::string& closest_id_name) {
    const char* attribute = can_id->Attribute(tag_name);
    if (!attribute) {
        std::cerr << "Missing tag " << tag_name << " near " << closest_id_name
            << std::endl;
        std::abort();
    }
    return attribute;
}

std::string read_ident_attribute(XMLElement* can_id, const char* tag_name,
                                 const std::string& closest_id_name) {
    const char* text = read_attribute(can_id, tag_name, closest_id_name);

    std::string name = trim(text);
    if (!is_valid_topic_and_identifier(name)) {
        std::cerr << "Tag `" << tag_name << "=\"" << name << "\"` near "
            << closest_id_name << " is not a valid C++ identifier/ROS2 topic"
            << std::endl;
        std::abort();
    }
    return name;
}

bool read_bool_attribute(XMLElement* can_id, const char* tag_name,
                         const std::string& closest_id_name) {
    const char* text = read_attribute(can_id, tag_name, closest_id_name);
    if (std::strcmp(text, "true") == 0)
        return true;
    else if (std::strcmp(text, "false") == 0)
        return false;
    else {
        std::cerr << "Tag `" << tag_name << "=\"" << text << "\"` near "
            << closest_id_name << R"( must be either "false" or "true")"
            << std::endl;
        std::abort();
    }
}

int read_int_attribute(XMLElement* can_id, const char* tag_name, int bit_width,
                       const std::string& closest_id_name) {
    const char* text = read_attribute(can_id, tag_name, closest_id_name);
    int integer = -1;
    size_t pos = 0;
    try {
        integer = std::stoi(text, &pos);
    } catch (...) {
        // empty catch here because `integer` will remain out of range
        // and surface that it's messed up below
    }

    if (integer >= 0 && integer < 2 << (bit_width - 1) && pos ==
        std::strlen(text)) {
        return integer;
    } else {
        std::cerr << "Tag `" << tag_name << "=\"" << text << "\"` near "
            << closest_id_name << " must be an integer between 0 and "
            << (2 << (bit_width - 1)) - 1 << std::endl;
        std::abort();
    }
}

constexpr uint16_t
    determine_can_id(bool no_ros_echo, int device_id, int sub_id) {
    return (no_ros_echo << 10) | (device_id << 3) | sub_id;
}

std::string cplusplus_constexpr(const std::string& name, uint16_t id) {
    std::stringstream out{};
    out << "constexpr CANID " << name << " = 0b"
    << std::bitset<1>(id >> 10) << "'"
    << std::bitset<1>(id >> 9) << "'"
    << std::bitset<6>(id >> 3) << "'"
    << std::bitset<3>(id)
    << ";";
    return out.str();
}

// Should be pasted into GuppyCAN.hpp
namespace GuppyCAN {
typedef unsigned short CANID;
constexpr const char* name_from_id(const CANID id) {
    switch (id) {
    case 0b00000000000: return "CurrentState";
    case 0b00001100011: return "BarometerAlt";
    default: return "";
    }
}
}


// End: Should be pasted into GuppyCAN.hpp

struct NameIDPair {
    std::string name;
    GuppyCAN::CANID id;
};

int main() {
    tinyxml2::XMLDocument doc;
    const char* path = "./can-ids.xml";
    char amountText[100];
    doc.LoadFile(path);

    XMLElement* root = doc.RootElement();
    if (!root) {
        std::cerr << "Could not find " << path << " in working directory"
            << " or XML is invalid" << std::endl;
        // Possible improvement: show canonical path it searched here
        //                       remember to handle possible errors on that
        std::abort();
    }

    std::vector<NameIDPair> can_ids{};

    std::string closest_id_name = "start of document";
    XMLElement* can_id = root->FirstChildElement("id");
    while (can_id) {
        std::string name =
            read_ident_attribute(can_id, "name", closest_id_name);

        bool no_ros_echo = read_bool_attribute(
            can_id, "no-ros-echo", closest_id_name
        );
        closest_id_name = name;
        int device_id = read_int_attribute(
            can_id, "device-id", 6, closest_id_name
        );
        int sub_id = read_int_attribute(
            can_id, "sub-id", 3, closest_id_name
        );

        can_ids.push_back(NameIDPair {
            .name = name,
            .id = determine_can_id(no_ros_echo, device_id, sub_id)
        });

        can_id = can_id->NextSiblingElement("id");
    }

    std::filesystem::create_directory("generated");
    std::ofstream header("generated/GuppyCAN.hpp");

    header << R"(/// GENERATED -- Do not edit directly
/// CAN IDs are specified in can-ids.xml
#ifndef GUPPY_CAN_HPP
#define GUPPY_CAN_HPP

typedef unsigned short CANID;

namespace GuppyCAN {
)";
    for (const NameIDPair& pair : can_ids) {
        header << cplusplus_constexpr(pair.name, pair.id) << "\n";
    }
    header << R"(}

#endif // GUPPY_CAN_HPP)";

    std::cout << "Processing finished successfully" << std::endl;
    return 0;
}