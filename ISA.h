#include <unordered_map>

// Top-level class for the different instruction types
enum class InstFormat {
    R_TYPE,
    I_TYPE,
    J_TYPE,
    INVALID
};

// R-type funct opcodes map
const std::unordered_map<std::string, int> R_Funct = {
    {"SUB", 0x22},
    {"ADD", 0x20},
    {"AND", 0x24},
    {"OR", 0x25},
    {"SLT", 0x2A},
    {"SLL", 0x00},
    {"SRL", 0x02},
    {"JR", 0x08},
    {"INVALID", 0xFF}
};

// I-type opcodes map
const std::unordered_map<std::string, uint8_t> I_Opcode_Map = {
    {"ADDI", 0x08},
    {"ANDI", 0x0C},
    {"ORI",  0x0D},
    {"SLTI", 0x0A},
    {"LW",   0x23},
    {"SW",   0x2B},
    {"BEQ",  0x04},
    {"BNE",  0x05},
    {"INVALID", 0xFF}
};

// J-type opcodess map
const std::unordered_map<std::string, uint8_t> J_Opcode_Map = {
    {"J",   0x02},
    {"JAL", 0x03},
    {"INVALID", 0xFF}
};