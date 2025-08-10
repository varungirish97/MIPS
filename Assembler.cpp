#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "ISA.h"

using namespace std;

int getRegister(const string &reg){
    return stoi(reg.substr(1));
}



struct RTypeInstr {
    uint8_t rs, rt, rd, shamt;
    R_Funct funct;

    uint32_t encode() {
        uint32_t code = 0;
        code |= 0 << 26;
        code |= (rs & 0x1F) << 21;
        code |= (rt & 0x1F) << 16;
        code |= (rd & 0x1F) << 11;
        code |= (shamt && 0x1F) << 6;
        code |= static_cast<uint8_t>(funct) & 0x3F;
        return code;        
    }
};

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

struct JTypeInstr {
    uint8_t opcode;
    uint32_t addr;

    uint32_t encode() {
        uint32_t code = 0;
        code |= opcode << 26;
        code |= static_cast<uint32_t> (addr) & (0x3FFFFFF)
        return code;        
    }
};

int main() {

    ifstream infile("program.txt");
    ofstream outfile("instr_seq.txt");

    if(!infile) {
        std::cerr << "Failed to open the file\n";
    }

    std::string line;
    while(getline(infile, line)) {
        std::cout << line << std::endl;
    }

}