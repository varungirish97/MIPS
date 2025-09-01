#ifndef MIPS_H
#define MIPS_H

#include "ISA.h"
#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>

// Currently supporting only a few instructions. Will optimize with early branch and hazard detection and data forwarding.
class MIPS_MEMORY {
    public:
        uint32_t I_MEM[256]     = {0};              // Instruction Memory
        uint32_t D_MEM[256]     = {0};              // Data Memory
        uint32_t REG_FILE[32]   = {0};              // Register file
        uint32_t PC             = 0x00000000;       // Program Counter starts off from 0. Indexes the I_MEM
        int      num_instr      = 0;

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

        void generate_control_signals(uint8_t opcode, uint8_t funct);
        uint8_t generate_ALU_control(uint8_t opcode);
        void IF_Stage() {
            if (ALL_MEMORIES.PC < ALL_MEMORIES.num_instr * 4) {
                uint32_t instr      = ALL_MEMORIES.I_MEM[ALL_MEMORIES.PC >> 2];           // Each instruction is 32 bits or 4 bytes, so we need to index IMEM with PC/4
                IF_ID.Instruction   = instr;
                IF_ID.PC_plus_4     = ALL_MEMORIES.PC + 4;
                ALL_MEMORIES.PC    += 4;
            } else {
                IF_ID.Instruction   = 0;
            }
        }
        void ID_Stage() {
            uint32_t instr      = IF_ID.Instruction;
            uint8_t opcode      = (instr >> 26) & 0x3F;
            uint8_t rs          = (instr >> 21) & 0x3F;
            uint8_t rt          = (instr >> 16) & 0x3F;
            uint8_t rd          = (instr >> 11) & 0x3F;
            uint8_t shamt       = (instr >> 6) & 0x1F;
            uint8_t funct       = instr & 0x3F;
            uint16_t imm        = instr & 0xFFFF;                       // For I type instructions
            uint32_t addr       = instr & 0x3FFFFFF;                    // For J type instructions

            ID_EX.PC_plus_4 =   IF_ID.PC_plus_4;
            ID_EX.Rs        =   rs;
            ID_EX.Rt        =   rt;
            ID_EX.Rd        =   rd;

            // Read RF and get Rs_val and Rt_val;
            ID_EX.Rs_Val    =   ALL_MEMORIES.REG_FILE[rs];
            ID_EX.Rt_Val    =   ALL_MEMORIES.REG_FILE[rt];
            ID_EX.Immediate =   static_cast<int32_t> (imm);

            ID_EX.RegDst    =   false;
            ID_EX.ALUSrc    =   false;
            ID_EX.MemRd     =   false;
            ID_EX.MemWr     =   false;
            ID_EX.RegWr     =   false;
            ID_EX.MemToReg  =   false;

            generate_control_signals(opcode, funct);
        }

        void EX_Stage() {
            uint8_t ALU_ctrl    = generate_ALU_control(ID_EX.ALUFunct);        // One hot encoded ALU control signals
            uint32_t result     = 0x0;
            uint32_t ALU_A      =   ID_EX.Rs_Val;
            uint32_t ALU_B      =   (ID_EX.ALUSrc) ? ID_EX.Immediate : ID_EX.Rt_Val;

            switch(ALU_ctrl) {
                case 0x01:      // ADD
                    result = ALU_A + ALU_B;
                case 0x02:      // SUB
                    result = ALU_A - ALU_B;
                case 0x03:      // AND
                    result = ALU_A & ALU_B;
                case 0x04:      // OR
                    result = ALU_A | ALU_B;
                case 0x05:      // SLT
                    result = (ALU_A < ALU_B) ? 0x1 : 0x0;
            }

            EX_MEM.PC_plus_4    =   ID_EX.PC_plus_4;
            EX_MEM.Rd           =   (ID_EX.RegDst) ? ID_EX.Rd : ID_EX.Rt;
            EX_MEM.ALURes       =   result;
            EX_MEM.RegWr        =   ID_EX.RegWr;
            EX_MEM.MemRd        =   ID_EX.MemRd;
            EX_MEM.MemWr        =   ID_EX.MemWr;
            EX_MEM.MemToReg     =   ID_EX.MemToReg;
        }

        void MEM_Stage();
        void WB_Stage();
        void clock() {
            IF_Stage();
            ID_Stage();
            EX_Stage();
            MEM_Stage();
            WB_Stage();
        }
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
    int32_t     Immediate   =   0;

    bool        RegDst      =   false;                                        //Decides the destination register. For R-Type it is Rd and for I-Type it is Rt
    bool        ALUSrc      =   false;                                        //Decides the 2nd input to the ALU. R-type have the 2nd SRC to be from RegFile, but for I-Type we get it from regValue+sign_extended_imm
    bool        MemRd       =   false;                                        //LW instruction
    bool        MemWr       =   false;                                        //SW instruction
    bool        RegWr       =   false;                                        //For any R-type or LW as well
    bool        MemToReg    =   false;                                        //Decides whether to write the register file from the output of the ALU or Memory
    bool        Branch      =   false;    
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
    num_instr = index;
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

void Pipeline::generate_control_signals(uint8_t opcode, uint8_t funct) {

    bool r_type     =   false;
    bool i_type     =   false;
    bool j_type     =   false;

    if(findValinMap(R_Funct, opcode)) {
        r_type  =   true;
    }
    else if(findValinMap(I_Opcode_Map, opcode)) {
        i_type  =   true;
    }
    else if(findValinMap(J_Opcode_Map, opcode)) {
        j_type  =   true;
    }
    if(r_type) {
        ID_EX.RegDst    =   true;
        ID_EX.RegWr     =   true;
        ID_EX.ALUFunct  =   funct;
    }
    // Load Word instruction
    else if(i_type & opcode == 0x23) {
        ID_EX.ALUSrc    =   true;
        ID_EX.MemRd     =   true;
        ID_EX.RegWr     =   true;
        ID_EX.MemToReg  =   true;
        ID_EX.ALUFunct  =   opcode;
    }
    // Store Word instruction
    else if(i_type & opcode == 0x2B) {
        ID_EX.ALUSrc    =   true;
        ID_EX.MemWr     =   true;
        ID_EX.ALUFunct  =   opcode;
    }
    else if(i_type) {
        ID_EX.ALUSrc    =   true;
        ID_EX.RegWr     =   true;
        ID_EX.ALUFunct  =   opcode;
    }
    else if(i_type & (opcode == 0x04 || opcode == 0x05)) {
        ID_EX.Branch    =   true;
        ID_EX.ALUFunct  =   opcode;
    }

}

uint8_t Pipeline::generate_ALU_control(uint8_t opcode) {
    switch(opcode) {
        case 0x20:      // ADD
            return 0x01;
        case 0x08:      // ADDI
            return 0x01;
        case 0x22:      // SUB
            return 0x02;
        case 0x24:      // AND
            return 0x03;
        case 0x0C:     // ANDI
            return 0x03;
        case 0x25:      // OR
            return 0x04;
        case 0x0D:      // ORI
            return 0x04;
        case 0x2A:      // SLT
            return 0x05;
        case 0x0A:      // SLTI
            return 0x05;
    }
}

#endif