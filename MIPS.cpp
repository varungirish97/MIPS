#include "MIPS.h"
#include "ISA.h"
#include "Assembler.h"

#include <unordered_map>

using namespace std;

int main(){
    std::cout << "Converting MIPS assembly to hex encoded instructions" << std::endl;
    parseInputFile();

    MIPS_MEMORY mips_mem;
    
    std::ifstream input_file("instr_seq.txt");
    int mem_result = mips_mem.load_I_Mem(input_file);
    Pipeline pipeline(mips_mem.I_MEM, mips_mem.num_instr);
    pipeline.ALL_MEMORIES.write_DM(0, 10);
    pipeline.ALL_MEMORIES.write_DM(1, 15);
    pipeline.ALL_MEMORIES.write_DM(2, 20);
    pipeline.ALL_MEMORIES.write_DM(3, 25);
    pipeline.ALL_MEMORIES.write_DM(4, 30);
    
    pipeline.ALL_MEMORIES.write_RF(0, 0);
    int count = 0;
    while(count < 10){
        pipeline.IF_Stage();
        pipeline.ID_Stage();
        // Mismatch in the way ALUFunct is being used. So the ALU control signals are not getting generated correctly
        pipeline.EX_Stage();
        pipeline.MEM_Stage();
        pipeline.WB_Stage();
        count = count + 1;
    }
    

    return 0;
}