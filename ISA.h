#ifndef ISA_H
#define ISA_H

#include <unordered_map>
/*
extern const std::unordered_map<std::string, int> R_Funct;
extern const std::unordered_map<std::string, int> I_Opcode_Map;
const std::unordered_map<std::string, int> J_Opcode_Map;
*/
// Top-level class for the different instruction types
enum class InstFormat {
    R_TYPE,
    I_TYPE,
    J_TYPE,
    INVALID
};



#endif