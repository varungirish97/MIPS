#include <iostream>
#include <cstdint>
#include "ISA.h"

using namespace std;
/*
enum class Opcode {
    R_TYPE = 0,
    J_TYPE = 2,
    I_TYPE = 1,
    INVALID = 3
};

enum class I_Opcode {
    ADDI = 8,
    ANDI = 12,
    BEQ = 4,
    BGEZ = 1,
    BGTZ = 7,
    BLEZ = 6,
    BLTZ = 0,
    BNE = 5,
    LW = 35,
    SW = 43,
    ORI = 13,
    XORI = 14
};

enum class Funct {
    ADD = 32,
    SLT = 42,
    SUB = 34,
    SRA = 35,
    AND = 36,
    OR = 37,
    NOR = 39,
    SLL = 0,
    SRL = 2,
    XOR = 38,
    MULT = 24,
    DIV = 16,
};
*/

uint32_t createRTypeInstr(uint8_t opcode, uint8_t rs, uint8_t rt, uint8_t rd, uint8_t shamt, uint8_t func){
    return ( ( opcode << 26 ) | ( rs << 21 ) | ( rt << 16 ) | ( rd << 11 ) | ( shamt << 6 ) | ( func ) );
}

uint32_t createITypeInstr(uint8_t opcode, uint8_t rs, uint8_t rt, uint16_t immediate){
    return ( ( opcode << 26 ) | ( rs << 21 ) | ( rt << 16 ) | ( immediate ) );
}

uint32_t createJTypeInstr(uint8_t opcode, uint32_t address){
    return ( ( opcode << 26 ) | ( address ) );
}


int main(){

uint8_t opcode = static_cast<uint8_t>(Opcode::R_TYPE);
uint8_t rd = 1;
uint8_t rs = 2;
uint8_t rt = 3;

Funct function = Funct::ADD;
uint8_t shamt = 0;
uint16_t immediate = 0;
uint32_t address = 0;

if(opcode == static_cast<uint8_t>(Opcode::R_TYPE)){
    uint32_t RTypeInstr = createRTypeInstr( 0, rs, rt, rd, shamt, static_cast<uint8_t>(function) );
    cout << "R-Type instruction is : 0x" << hex << RTypeInstr << endl;
}

}