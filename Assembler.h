#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <unordered_map>
#include <string>
#include "ISA.h"

int getRegister(const std::string &reg);
void parseInputFile();

// This hashmap gives a mapping between the $register name to a register number. Easy to manage ig
const std::unordered_map<std::string, int8_t> register_map = {
    {"$0", 0},  {"$1", 1},  {"$2", 2},  {"$3", 3},
    {"$4", 4},  {"$5", 5},  {"$6", 6},  {"$7", 7},
    {"$8", 8},  {"$9", 9},  {"$10", 10},{"$11", 11},
    {"$12", 12},{"$13", 13},{"$14", 14},{"$15", 15},
    {"$16", 16},{"$17", 17},{"$18", 18},{"$19", 19},
    {"$20", 20},{"$21", 21},{"$22", 22},{"$23", 23},
    {"$24", 24},{"$25", 25},{"$26", 26},{"$27", 27},
    {"$28", 28},{"$29", 29},{"$30", 30},{"$31", 31}
};

// R-Type instruction structure and encodes the instruction according to the format described in the excel
struct RTypeInstr {
    uint8_t rs, rt, rd, shamt;
    uint16_t funct;

    uint32_t encode() {
        uint32_t code = 0;
        code |= 0 << 26;
        code |= (rs & 0x1F) << 21;
        code |= (rt & 0x1F) << 16;
        code |= (rd & 0x1F) << 11;
        code |= (shamt & 0x1F) << 6;
        code |= static_cast<uint8_t>(funct) & 0x3F;
        return code;        
    }
};

// I-Type instruction structure and encodes the instruction according to the format described in the excel
struct ITypeInstr {
    uint8_t opcode, rs, rt;
    uint16_t imm;

    uint32_t encode() {
        uint32_t code = 0;
        code |= opcode << 26;
        code |= (rs & 0x1F) << 21;
        code |= (rt & 0x1F) << 16;
        code |= static_cast<uint16_t>(imm) & 0xFFFF;
        return code;        
    }
};

// J-Type instruction structure and encodes the instruction according to the format described in the excel
struct JTypeInstr {
    uint8_t opcode;
    uint32_t addr;

    uint32_t encode() {
        uint32_t code = 0;
        code |= opcode << 26;
        code |= static_cast<uint32_t> (addr) & (0x3FFFFFF);
        return code;        
    }
};

#endif