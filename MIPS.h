#ifndef MIPS_H
#define MIPS_H

#include "ISA.h"
#include "Assembler.h"
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
        std::vector<std::string> I_MEM_ASM;
        int      num_instr      = 0;

        MIPS_MEMORY() = default;
        void write_I_MEM_ASM(std::vector<std::string> INSTR_ASM);
        int load_I_Mem(std::ifstream &input_file);
        int read_RF(uint32_t reg);
        int read_IM(uint32_t program_counter);
        int read_DM(uint32_t line_addr);

        int write_RF(uint32_t reg, uint32_t value);
        int write_DM(uint32_t line_addr, uint32_t value);
        //No write_IM because the plan is to load the instruction memory at the beginning of the program
};

struct IF_ID {
    uint32_t PC_plus_4      = 0x00000000;
    uint32_t Instruction    = 0x00000000;
    bool     Valid          = false;
    std::string instr_str   = "NOP";
};
struct ID_EX {
    uint32_t    PC_plus_4   =   0x00000000;
    uint8_t     Rs          =   0;
    uint8_t     Rt          =   0;
    uint8_t     Rd          =   0;
    uint32_t    Rs_Val      =   0;
    uint32_t    Rt_Val      =   0;
    uint32_t    shamt       =   0;
    uint32_t    ALUFunct    =   0;
    int32_t     Immediate   =   0;
    bool        Valid       =   false;
    std::string instr_str   =   "NOP";

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
    uint8_t     WriteReg    =   0;                                            //Needed for R-Type as this is the destination.                                          //Needed for I-Type as this is the destination.
    uint32_t    ALURes      =   0;
    uint32_t    Rt_Val      =   0;
    bool        Valid       =   false;
    std::string instr_str   =   "NOP";

    bool        MemRd       =   false;
    bool        MemWr       =   false;
    bool        RegWr       =   false;
    bool        MemToReg    =   false;
};

struct MEM_WB {
    uint32_t    MemData     =   0;
    uint32_t    ALURes      =   0;
    uint32_t    Mem_ALU     =   0;
    uint32_t    WriteReg    =   0;
    bool        Valid       =   false;
    std::string instr_str   =   "NOP";

    bool        RegWr       =   false;
    bool        MemToReg    =   false;
};

class Pipeline {
    public:
        IF_ID           IF_ID;
        ID_EX           ID_EX;
        EX_MEM          EX_MEM;
        MEM_WB          MEM_WB;
        MIPS_MEMORY     ALL_MEMORIES;
        bool            stall;

        Pipeline(uint32_t instr_mem[256], int num_instr){
            for(int i = 0 ; i < 256 ; i++){
                ALL_MEMORIES.I_MEM[i] = instr_mem[i];
            }
            ALL_MEMORIES.num_instr = num_instr;
            stall = false;
        }
        
        void generate_control_signals(uint8_t opcode, uint8_t funct);
        uint8_t generate_ALU_control(uint8_t opcode);
        bool isPipelineEmpty();
        bool hazard_detection_unit(uint8_t Rs, uint8_t Rt);
        uint16_t forwarding_unit();
        
        void IF_Stage() {
            if (ALL_MEMORIES.PC < ALL_MEMORIES.num_instr * 4 && !this->stall) {
                uint32_t instr      = ALL_MEMORIES.I_MEM[ALL_MEMORIES.PC >> 2];           // Each instruction is 32 bits or 4 bytes, so we need to index IMEM with PC/4
                IF_ID.Instruction   = instr;
                IF_ID.PC_plus_4     = ALL_MEMORIES.PC + 4;
                IF_ID.instr_str     = ALL_MEMORIES.I_MEM_ASM[ALL_MEMORIES.PC >> 2];
                ALL_MEMORIES.PC    += 4;
                IF_ID.Valid         = true;
            } else {
                IF_ID.Instruction   = 0;
                IF_ID.Valid         = false;
            }
        }
        
        void ID_Stage() {
            uint32_t instr      = IF_ID.Instruction;
            uint8_t opcode      = (instr >> 26) & 0x3F;
            uint8_t rs          = (instr >> 21) & 0x1F;
            uint8_t rt          = (instr >> 16) & 0x1F;
            uint8_t rd          = (instr >> 11) & 0x1F;
            uint8_t shamt       = (instr >> 6) & 0x1F;
            uint8_t funct       = instr & 0x3F;
            uint16_t imm        = instr & 0xFFFF;                       // For I type instructions
            uint32_t addr       = instr & 0x3FFFFFF;                    // For J type instructions
            uint8_t shift_op    = (funct == 0x0 || funct == 0x02);      // SLL or SRL
            this->stall         = hazard_detection_unit(rs, rt);

            if( !stall ){
                ID_EX.PC_plus_4 =   IF_ID.PC_plus_4;
                ID_EX.Rs        =   rs;
                ID_EX.Rt        =   rt;
                ID_EX.Rd        =   rd;
                ID_EX.instr_str =   IF_ID.instr_str;
                std::cout << rs << std::endl;
                std::cout << rt << std::endl;
                std::cout << rd << std::endl;

                // Read RF and get Rs_val and Rt_val;
                ID_EX.Rs_Val    =   ALL_MEMORIES.REG_FILE[rs];
                ID_EX.Rt_Val    =   shift_op ? shamt : ALL_MEMORIES.REG_FILE[rt];
                ID_EX.shamt     =   shamt;
                ID_EX.Immediate =   static_cast<int32_t> (imm);

                ID_EX.Valid     =   IF_ID.Valid;
                ID_EX.RegDst    =   false;
                ID_EX.ALUSrc    =   false;
                ID_EX.MemRd     =   false;
                ID_EX.MemWr     =   false;
                ID_EX.RegWr     =   false;
                ID_EX.MemToReg  =   false;

                generate_control_signals(opcode, funct);
            }
            else {          // If there is a stall then insert a NOP or a bubble, de-assert the control signals 
                ID_EX.instr_str =   "NOP";
                ID_EX.MemWr     =   false;
                ID_EX.RegWr     =   false;
            }


        }

        void EX_Stage() {
            uint16_t ALUSrc     =   forwarding_unit();
            uint8_t ALUSelA     =   ALUSrc & 0xFF;
            uint8_t ALUSelB     =   (ALUSrc >> 8) & 0xFF;

            uint8_t ALU_ctrl    =   generate_ALU_control(ID_EX.ALUFunct);        // One hot encoded ALU control signals
            uint32_t result     =   0x0;

            uint32_t ALU_A      =   (ALUSelA == 2) ? EX_MEM.ALURes : (ALUSelA == 1) ? MEM_WB.Mem_ALU : ID_EX.Rs_Val;
            //uint32_t ALU_A      =   ID_EX.Rs_Val;
            uint32_t ALU_B      =   (ALUSelB == 2) ? EX_MEM.ALURes : (ALUSelA == 1) ? MEM_WB.Mem_ALU : (ID_EX.ALUSrc) ? ID_EX.Immediate : ID_EX.Rt_Val;
            //uint32_t ALU_B      =   (ID_EX.ALUSrc) ? ID_EX.Immediate : ID_EX.Rt_Val;

            switch(ALU_ctrl) {
                case 0x01:      // ADD
                    result = ALU_A + ALU_B;
                    break;
                case 0x02:      // SUB
                    result = ALU_A - ALU_B;
                    break;
                case 0x03:      // AND
                    result = ALU_A & ALU_B;
                    break;
                case 0x04:      // OR
                    result = ALU_A | ALU_B;
                    break;
                case 0x05:      // SLT
                    result = (ALU_A < ALU_B) ? 0x1 : 0x0;
                    break;
                case 0x06:      // SLL
                    result = (ALU_A << ALU_B);
                    break;
                case 0x07:      // SRL
                    result = (ALU_A >> ALU_B);
                    break;
            }

            EX_MEM.Valid        =   ID_EX.Valid;
            EX_MEM.PC_plus_4    =   ID_EX.PC_plus_4;
            EX_MEM.WriteReg     =   (ID_EX.RegDst) ? ID_EX.Rd : ID_EX.Rt;
            EX_MEM.Rt_Val       =   ID_EX.Rt_Val;
            EX_MEM.ALURes       =   result;
            EX_MEM.RegWr        =   ID_EX.RegWr;
            EX_MEM.MemRd        =   ID_EX.MemRd;
            EX_MEM.MemWr        =   ID_EX.MemWr;
            EX_MEM.MemToReg     =   ID_EX.MemToReg;
            EX_MEM.instr_str    =   ID_EX.instr_str;
        }

        void MEM_Stage() {
            MEM_WB.MemData = 0;
            // Handling the LW instruction
            if(EX_MEM.MemRd) {
                MEM_WB.MemData   =   ALL_MEMORIES.D_MEM[EX_MEM.ALURes / 4];
            }
            // Handling the SW instruction
            if(EX_MEM.MemWr) {
                ALL_MEMORIES.D_MEM[EX_MEM.ALURes / 4]   =   EX_MEM.Rt_Val;
            }

            MEM_WB.Valid    =   EX_MEM.Valid;
            MEM_WB.ALURes   =   EX_MEM.ALURes;
            MEM_WB.WriteReg =   EX_MEM.WriteReg;
            MEM_WB.RegWr    =   EX_MEM.RegWr;
            MEM_WB.MemToReg =   EX_MEM.MemToReg;
            MEM_WB.instr_str=   EX_MEM.instr_str;
            MEM_WB.Mem_ALU  =   (EX_MEM.MemToReg) ? MEM_WB.MemData : EX_MEM.ALURes;
        }

        void WB_Stage() {
            // Selects between the output of the memory and the ALU
            uint32_t reg_data = (MEM_WB.MemToReg) ? MEM_WB.MemData : MEM_WB.ALURes; 
            
            // Updates the Register file
            if (MEM_WB.RegWr) {
                ALL_MEMORIES.REG_FILE[MEM_WB.WriteReg]    =   reg_data;
            }
        }
        
        void clock() {
            IF_Stage();
            ID_Stage();
            EX_Stage();
            MEM_Stage();
            WB_Stage();
        }
};

void MIPS_MEMORY::write_I_MEM_ASM(std::vector<std::string> INSTR_ASM) {
    I_MEM_ASM = INSTR_ASM;
}

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
    else{
        REG_FILE[reg] = value;
        return 0;
    }
   
}

int MIPS_MEMORY::write_DM(uint32_t line_addr, uint32_t value) {
    if(line_addr > 255){
        std::cerr << "Line address exceeds the size of DMEM" << std::endl;
        return 1;
    }
    else{
        D_MEM[line_addr] = value;
        return 0;
    }

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
    else if(i_type & (opcode == 0x23)) {
        ID_EX.ALUSrc    =   true;
        ID_EX.MemRd     =   true;
        ID_EX.RegWr     =   true;
        ID_EX.MemToReg  =   true;
        ID_EX.ALUFunct  =   opcode;
    }
    // Store Word instruction
    else if(i_type & (opcode == 0x2B)) {
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
        case 0x23:      // LW
            return 0x01;
        case 0x2B:      // SW
            return 0x01;
        case 0x04:      // BEQ
            return 0x02;
        case 0x05:      // BNE
            return 0x02;
        case 0x00:      // SLL
            return 0x06;
        case 0x02:      // SRL
            return 0x07;
        default:
            return 0x0;
    }
}

bool Pipeline::isPipelineEmpty() {
    bool PipelineEmpty = !(IF_ID.Valid || ID_EX.Valid || EX_MEM.Valid || MEM_WB.Valid);
    return PipelineEmpty;
}

bool Pipeline::hazard_detection_unit(uint8_t Rs, uint8_t Rt) {
    if(this->ID_EX.RegWr && !this->ID_EX.RegDst && (this->ID_EX.Rt == Rs || this->ID_EX.Rt == Rt) ) {           // LW instruction in EX and the destination of LW matches source of ID stage instruction
        return true;
    }
    else
        return false;
}

uint16_t Pipeline::forwarding_unit() {
    uint8_t ALUSelA = 0;            // Rs (1st operand)
    uint8_t ALUSelB = 0;            // Rt (2nd operand)
    uint16_t ALUSrc = 0;
    // R-type in MEM stage and a dependent R-type in EX, output data can be forwarded to the source in EX
    if(EX_MEM.RegWr && !EX_MEM.MemRd && EX_MEM.WriteReg == ID_EX.Rs){
        ALUSelA     =   2;
    }
    // R-type / LW in WB stage and a dependent R-type in EX, output data can be forwarded to the source in EX
    else if(MEM_WB.RegWr && MEM_WB.WriteReg == ID_EX.Rs) {
        ALUSelA     =   1;
    }
    if(EX_MEM.RegWr && !EX_MEM.MemRd && EX_MEM.WriteReg == ID_EX.Rt){
        ALUSelB     =   2;
    }
    else if(MEM_WB.RegWr && MEM_WB.WriteReg == ID_EX.Rt) {
        ALUSelB     =   1;
    }

    ALUSrc  =   (ALUSelB << 8 | ALUSelA);
    return ALUSrc;
}
#endif