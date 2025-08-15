#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include "Assembler.h"

using namespace std;

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
            std::cout << rd << rs << rt << std::endl;
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

int main() {
    parseInputFile();
    return 1;
}