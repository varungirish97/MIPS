#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include "ISA.h"
using namespace std;

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
const std::unordered_map<std::string, int> I_Opcode_Map = {
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
const std::unordered_map<std::string, int> J_Opcode_Map = {
    {"J",   0x02},
    {"JAL", 0x03},
    {"INVALID", 0xFF}
};


// R-type funct opcodes map
bool findValinMap(const std::unordered_map<std::string, int> map, uint8_t opcode) {
    bool found = false;
    for(const auto& element : map) {
        if(element.second == opcode) {
            found = true;
            break;
        }
    }
    return found;
}

// This hashmap gives a mapping between the $register name to a register number. Easy to manage ig
const unordered_map<std::string, int> register_map = {
    {"$0", 0},  {"$1", 1},  {"$2", 2},  {"$3", 3},
    {"$4", 4},  {"$5", 5},  {"$6", 6},  {"$7", 7},
    {"$8", 8},  {"$9", 9},  {"$10", 10},{"$11", 11},
    {"$12", 12},{"$13", 13},{"$14", 14},{"$15", 15},
    {"$16", 16},{"$17", 17},{"$18", 18},{"$19", 19},
    {"$20", 20},{"$21", 21},{"$22", 22},{"$23", 23},
    {"$24", 24},{"$25", 25},{"$26", 26},{"$27", 27},
    {"$28", 28},{"$29", 29},{"$30", 30},{"$31", 31}
};

int getRegister(const string &reg){
    return stoi(reg.substr(1));
}

void parseInputFile() {

    ifstream infile("program.txt");
    ofstream outfile("instr_seq.txt");
    if(!infile) {
        std::cerr << "Failed to open the file\n";
    }
    if(!outfile) {
            std::cerr << "Error opening file for write\n";
    }
        
    std::string line;
    while(getline(infile, line)) {
        line.erase(std::remove(line.begin(), line.end(), ','), line.end());

        std::istringstream iss(line);                       // This reads the file as string stream and separates it by whitepsaces.
        std::string instr;
        uint32_t encoded_instr;
        iss >> instr;                                       // First word of the stream. Assumed to be the Instr name.
        

        // Checks if it is RType and encodes the instruction
        if(R_Funct.find(instr) != R_Funct.end()) {
            auto i = R_Funct.find(instr);
            std::string rs, rt, rd;
            iss >> rd >> rs >> rt;
            RTypeInstr r_instr;
            r_instr.rs      = register_map.at(rs);
            r_instr.rd      = register_map.at(rd);
            r_instr.rt      = register_map.at(rt);
            r_instr.shamt   = 0;
            r_instr.funct   = i->second;
            
            encoded_instr   = r_instr.encode();
        }

        else if(I_Opcode_Map.find(instr) != I_Opcode_Map.end()) {
            auto i = I_Opcode_Map.find(instr);
            std::string rs, rt, imm;
            iss >> rt >> rs >> imm;
            ITypeInstr i_instr;
            i_instr.rs      = register_map.at(rs);
            i_instr.rt      = register_map.at(rt);
            i_instr.imm     = std::stoi(imm);
            i_instr.opcode  = i->second;

            encoded_instr   = i_instr.encode();
        }

        else if(J_Opcode_Map.find(instr) != J_Opcode_Map.end()) {
            auto i = J_Opcode_Map.find(instr);
            std::string address;
            iss >> address;
            JTypeInstr j_instr;
            j_instr.addr    = std::stoi(address, nullptr, 16);
            j_instr.opcode  = i->second;

            encoded_instr   = j_instr.encode();
        }
        outfile << std::hex << encoded_instr << std::endl;

    }

}


#endif