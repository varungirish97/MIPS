#include "MIPS.h"
#include "ISA.h"
#include "Assembler.h"

#include <unordered_map>

using namespace std;

int main(){
    std::cout << "Converting MIPS assembly to hex encoded instructions" << std::endl;
    parseInputFile();
    std::vector<std::string> I_MEM_ASM = load_assembly_file("program.txt");

    MIPS_MEMORY mips_mem;
    
    std::ifstream input_file("instr_seq.txt");
    int mem_result = mips_mem.load_I_Mem(input_file);
    Pipeline pipeline(mips_mem.I_MEM, mips_mem.num_instr);
    pipeline.ALL_MEMORIES.write_I_MEM_ASM(I_MEM_ASM);
    
    pipeline.ALL_MEMORIES.write_DM(0, 10);
    pipeline.ALL_MEMORIES.write_DM(1, 15);
    pipeline.ALL_MEMORIES.write_DM(2, 20);
    pipeline.ALL_MEMORIES.write_DM(3, 25);
    pipeline.ALL_MEMORIES.write_DM(4, 30);
    
    pipeline.ALL_MEMORIES.write_RF(0, 0);
    int count = 0;

    // Running the stage functions from WB->IF so that each stage is operating on data from the previous stage
    while(pipeline.ALL_MEMORIES.PC <= pipeline.ALL_MEMORIES.num_instr * 4){
        pipeline.WB_Stage();
        pipeline.MEM_Stage();
        pipeline.EX_Stage();
        pipeline.ID_Stage();
        pipeline.IF_Stage();
        std::cout << "Clock : " << count << std::endl;
        count = count + 1;
    }
    

    return 0;
}