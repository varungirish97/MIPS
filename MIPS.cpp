#include "MIPS.h"
#include "ISA.h"
#include "Assembler.h"

using namespace std;

int main(){
    std::cout << "Converting MIPS assembly to hex encoded instructions" << std::endl;
    parseInputFile();
    MIPS_MEMORY mem;
    std::ifstream infile("instr_seq.txt");
    mem.load_I_Mem(infile);
    return 0;
}