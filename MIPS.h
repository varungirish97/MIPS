#ifndef MIPS_H
#define MIPS_H

#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>

class MIPS_MEMORY {
    public:
        uint32_t I_MEM[256]     = {0};              // Instruction Memory
        uint32_t D_MEM[256]     = {0};              // Data Memory
        uint32_t REG_FILE[32]   = {0};              // Register file
        uint32_t PC             = 0x00000000;       // Program Counter starts off from 0. Indexes the I_MEM

        int load_I_Mem(std::ifstream &input_file);
        int read_RF(uint32_t reg);
        int read_IM(uint32_t program_counter);
        int read_DM(uint32_t line_addr);

        int write_RF(uint32_t reg, uint32_t value);
        int write_DM(uint32_t line_addr, uint32_t value);
        //No write_IM because the plan is to load the instruction memory at the beginning of the program
};

class Pipeline {
    public:
        IF_ID           IF_ID;
        ID_EX           ID_EX;
        EX_MEM          EX_MEM;
        MEM_WB          MEM_WB;
        MIPS_MEMORY     ALL_MEMORIES;

        void IF_Stage();
        void ID_Stage();
        void EX_Stage();
        void MEM_Stage();
        void WB_Stage();
};

struct IF_ID {
    uint32_t PC_plus_4 = 0x00000000;
    uint32_t Instruction = 0x00000000;
};

struct ID_EX {
    uint32_t    PC_plus_4   =   0x00000000;
    uint8_t     Rs          =   0;
    uint8_t     Rt          =   0;
    uint8_t     Rd          =   0;
    uint32_t    Rs_Val      =   0;
    uint32_t    Rt_Val      =   0;
    uint32_t    ALUFunct    =   0;
    uint32_t    immediate   =   0;

    bool        RegDst     =    false;                                        //Decides the destination register. For R-Type it is Rd and for I-Type it is Rt
    bool        ALUSrc     =    false;                                        //Decides the 2nd input to the ALU. R-type have the 2nd SRC to be from RegFile, but for I-Type we get it from regValue+sign_extended_imm
    bool        MemRd      =    false;                                        //LW instruction
    bool        MemWr      =    false;                                        //SW instruction
    bool        RegWr      =    false;                                        //For any R-type or LW as well
    bool        MemToReg   =    false;                                        //Decides whether to write the register file from the output of the ALU or Memory
};

struct EX_MEM {
    uint32_t    PC_plus_4   =   0x00000000;
    uint8_t     Rd          =   0;                                            //Needed for R-Type as this is the destination.
    uint8_t     Rt          =   0;                                            //Needed for I-Type as this is the destination.
    uint32_t    ALURes      =   0;

    bool        MemRd       =   false;
    bool        MemWr       =   false;
    bool        RegWr       =   false;
    bool        MemToReg    =   false;
};

struct MEM_WB {
    uint32_t    ALURes      =   0;
    uint32_t    MemData     =   0;
    uint32_t    Rd          =   0;

    bool        RegWr       =   false;
    bool        MemToReg    =   false;
};

int MIPS_MEMORY::load_I_Mem(std::ifstream &input_file) {
    if(!input_file){
        std::cerr << "Error opening the file" << std::endl;
        return 1;
    }
    int index = 0;
    std::string line;
    while(std::getline(input_file, line)) {
        uint32_t instr;
        std::stringstream ss;
        ss << std::hex << line;
        ss >> instr;
        I_MEM[index++] = instr;
    }
    input_file.close();
    
    return 0;
}

int MIPS_MEMORY::read_RF(uint32_t reg) {
    if(reg > 31){
        std::cerr << "Register value exceeds the number of architectural registers" << std::endl;
        return 1;
    }

    return REG_FILE[reg];
}

int MIPS_MEMORY::read_IM(uint32_t program_counter) {
    if(program_counter > 255){
        std::cerr << "PC exceeds the size of IMEM" << std::endl;
        return 1;
    }

    return I_MEM[program_counter>>2];
}

int MIPS_MEMORY::read_DM(uint32_t line_addr) {
    if(line_addr > 255){
        std::cerr << "Line address exceeds the size of DMEM" << std::endl;
        return 1;
    }

    return D_MEM[line_addr];
}

int MIPS_MEMORY::write_RF(uint32_t reg, uint32_t value) {
    if(reg > 31){
        std::cerr << "Register value exceeds the number of architectural registers" << std::endl;
        return 1;
    }

    REG_FILE[reg] = value;
}

int MIPS_MEMORY::write_DM(uint32_t line_addr, uint32_t value) {
    if(line_addr > 255){
        std::cerr << "Line address exceeds the size of DMEM" << std::endl;
        return 1;
    }

    D_MEM[line_addr] = value;
}

#endif