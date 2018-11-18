/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
//registers 0 is valid
//registers_valid 1 is valid

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 *        implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 16);
  memset(cpu->regs_valid, 1, sizeof(int) * 16);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
      "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
      cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
       cpu->code_memory[i].opcode,
       cpu->code_memory[i].rd,
       cpu->code_memory[i].rs1,
       cpu->code_memory[i].rs2,
       cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 *        implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf(
      "%s,R%d,R%d,#%d,stalled: %d, busy: %d ", stage->opcode, stage->rs1, stage->rs2, stage->imm,stage->stalled, stage->busy);
  }
  
  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }

  if (strcmp(stage->opcode, "LOAD") == 0) {
    printf(
      "%s,R%d,R%d,#%d,stalled: %d, busy: %d ", stage->opcode, stage->rs1, stage->rs2, stage->imm,stage->stalled, stage->busy);
  }

  if (strcmp(stage->opcode, "AND") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  
  if (strcmp(stage->opcode, "OR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "EX-OR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "SUB") == 0) {
    printf("%s,R%d,R%d,R%d,stalled: %d, busy: %d ", stage->opcode, stage->rd, stage->rs1, stage->rs2,stage->stalled, stage->busy);
  }

  if (strcmp(stage->opcode, "ADD") == 0) {
    printf("%s,R%d,R%d,R%d,stalled: %d, busy: %d ", stage->opcode, stage->rd, stage->rs1, stage->rs2,stage->stalled, stage->busy);
  }

  if (strcmp(stage->opcode, "MUL") == 0) {
    printf("%s,R%d,R%d,R%d,stalled: %d, busy: %d ", stage->opcode, stage->rd, stage->rs1, stage->rs2, stage->stalled, stage->busy);
  }
   if (strcmp(stage->opcode, "BZ") == 0) {
    printf("%s, #%d ", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "BNZ") == 0) {
    printf("%s, #%d ", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "JUMP") == 0) {
    printf("%s, R%d, #%d ", stage->opcode, stage->rs1, stage->imm);
  }

}
/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

// when the stage is empty
static void
print_stage_contents(char* name)
{
  printf("%-15s: EMPTY ", name);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */

int
fetch(APEX_CPU* cpu)   
{

  CPU_Stage* stage = &cpu->stage[F];

  if (!stage->busy && !stage->stalled) {  
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;


    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd; 
    
    if(cpu->stage[DRF].stalled == 1)
    {
      if (ENABLE_DEBUG_MESSAGES) {
        print_stage_content("Fetch", stage);
      }
      return 0;
    }

    cpu->pc+=4;
    
/* Copy data from fetch latch to decode latch*/
    cpu->stage[DRF] = cpu->stage[F];
    
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", stage);
    }
  }
  return 0;
}
/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];
//
  if(!cpu->stage[EX].busy==0 && (!strcmp(cpu->stage[EX].opcode, "MUL") == 1)) {
    stage->stalled=1;  
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode", stage);
    }
    return 0;
  }

  if(cpu->regs_valid[stage->rs1] == 1 || cpu->regs_valid[stage->rs2] == 1){//dependency check
    stage->stalled=1;
  }
  else
  {
    stage->stalled=0;
  }
//  
  if (!stage->busy && !stage->stalled) {
    if (strcmp(stage->opcode, "STORE") == 0) {  
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];       
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {  
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];       
    }

    if (strcmp(stage->opcode, "ADD") == 0) {  
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];         
    }

    if (strcmp(stage->opcode, "SUB") == 0) {  
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];       
    }

    if (strcmp(stage->opcode, "AND") == 0) {  
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];       
    }

    if (strcmp(stage->opcode, "OR") == 0) {  
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];       
    }

    if (strcmp(stage->opcode, "XOR") == 0) {  
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];       
    }

    if (strcmp(stage->opcode, "MUL") == 0) {  
      stage->rs1_value=cpu->regs[stage->rs1];
      stage->rs2_value=cpu->regs[stage->rs2];       
    }
/*
//HALT 
      if(strcmp(stage->opcode, "HALT") == 0) {
        cpu->stage[F].stalled = 1;
        cpu->stage[F].pc = 0;
        strcpy(cpu->stage[F].opcode, "");
        cpu->halt_exec = 1;
      }

// JUMP
      if(strcmp(stage->opcode, "JUMP") == 0) {
        if(cpu->regs_valid[stage->rs1] == 0){
          stage->stalled = 1;
        }
        else {
          stage->rs1_value = cpu->regs[stage->rs1];
        }
      }

//BZ & BNZ
    if(strcmp(stage->opcode, "BZ") == 0 ||
       strcmp(stage->opcode, "BNZ") == 0) {
    if((cpu->stage[WB].bubble == 0) ||
         (cpu->stage[MEM].bubble == 0)) {
        stage->stalled = 1;
      }
    }
*/
    cpu->stage[EX] = cpu->stage[DRF];
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode", stage);
    }
  }
  else
  {
    cpu->stage[EX] = cpu->stage[DRF];
    
    
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode", stage);
    }
  }
  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
//0 is free or valid 
//flow dependency  is d,s1,s2 dest is source
// add r3,r1,r2
//sub r4,r3,r3
int
execute(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX];
// 
  if(stage->busy && strcmp(stage->opcode, "MUL") == 0){
    cpu->stage[EX].busy=stage[EX].busy-1;
    cpu->stage[F].stalled=0;
    cpu->stage[DRF].stalled=0;
    goto end;
  }
//
  if (!stage->busy && !stage->stalled) {

    if(strcmp(stage->opcode, "MUL") == 0){
       cpu->stage[F].stalled=1;
       cpu->stage[DRF].stalled=1;
       cpu->stage[EX].busy=1;
    }  

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      
      stage->mem_address=(stage->rs2_value)+(stage->imm);
    }

    /* Load */
    if (strcmp(stage->opcode, "LOAD") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->mem_address=(stage->rs1_value)+(stage->imm);
     }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer=(stage->imm);
    }

    /* ADD */
    if (strcmp(stage->opcode, "ADD") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer=(stage->rs1_value)+(stage->rs2_value); 
    }

      /* Sub */
    if (strcmp(stage->opcode, "SUB") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer=(stage->rs1_value)-(stage->rs2_value);
    }

    /* Or */
    if (strcmp(stage->opcode, "OR") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer=(stage->rs1_value)|(stage->rs2_value);
    }

    /* Xor */
    if (strcmp(stage->opcode, "XOR") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer=(stage->rs1_value)^(stage->rs2_value);
    }

    if (strcmp(stage->opcode, "AND") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer=(stage->rs1_value)&(stage->rs2_value);  
    }

    if (strcmp(stage->opcode, "MUL") == 0) {
      cpu->regs_valid[stage->rd] = 1;
      stage->buffer=(stage->rs1_value)*(stage->rs2_value);

    }
    /*
    if(strcmp(stage->opcode, "BZ") == 0) {
        stage->mem_address = stage->pc + stage->imm;
        if(cpu->zero_flag == 1) {
          int num_instr_skipped = 0;
          if(stage->mem_address > 4000 + (4*(cpu->code_memory_size))) {
            num_instr_skipped = (((4000 + (4*(cpu->code_memory_size))) -
                                stage->pc) / 4) - 1;
            cpu->code_memory_size -= num_instr_skipped;
          }
          else if(stage->imm > 0){
            num_instr_skipped = ((stage->mem_address - stage->pc) / 4) - 1;
            cpu->code_memory_size -= num_instr_skipped;
          }
          else {
            num_instr_skipped = ((stage->pc - stage->mem_address) / 4) + 1;
            cpu->code_memory_size += num_instr_skipped;
          }

          cpu->pc = stage->mem_address;
          cpu->pc_set = 1;
        }
      }

      if(strcmp(stage->opcode, "BNZ") == 0) {
        stage->mem_address = stage->pc + stage->imm;
        if(cpu->zero_flag == 0) {
          int num_instr_skipped = 0;
          if(stage->mem_address > 4000 + (4*(cpu->code_memory_size))) {
            num_instr_skipped = (((4000 + (4*(cpu->code_memory_size))) -
                                stage->pc) / 4) - 1;
            cpu->code_memory_size -= num_instr_skipped;
          }
          else if(stage->imm > 0){
            num_instr_skipped = ((stage->mem_address - stage->pc) / 4) - 1;
            cpu->code_memory_size -= num_instr_skipped;
          }
          else {
            num_instr_skipped = ((stage->pc - stage->mem_address) / 4) + 1;
            cpu->code_memory_size += num_instr_skipped;
          }
          cpu->pc = stage->mem_address;
          cpu->pc_set = 1;
          transform_to_bubble(&cpu->stage[DRF]);
        }
      }

      if(strcmp(stage->opcode, "JUMP") == 0) {
        int new_pc = stage->imm + stage->rs1_value;
        int num_instr_diff = ((new_pc - cpu->pc) / 4);
        if(num_instr_diff > 0) {
          cpu->code_memory_size -= (num_instr_diff - 1);
        }
        else if(num_instr_diff < 0) {
          cpu->code_memory_size -= (num_instr_diff + 1);
        }
        cpu->pc = new_pc;
        cpu->pc_set = 1;
      }
    }

    if(strcmp(stage->opcode, "HALT") == 0 &&
       stage->bubble == 0) {
      cpu->stage[DRF].pc = 0;
      strcpy(cpu->stage[DRF].opcode, "");
      cpu->stage[DRF].stalled = 1;
      cpu->stage[F].stalled = 1;
      cpu->stage[F].pc = 0;
    }*/
    end:
    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM] = cpu->stage[EX];
    
    if (ENABLE_DEBUG_MESSAGES) {

      print_stage_content("Execute", stage);
    }
  }
//  
  else{
    cpu->stage[MEM] = cpu->stage[EX];
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_contents("Execute");
    }
//    
  }
  return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
int
memory(APEX_CPU* cpu)
{


  CPU_Stage* stage = &cpu->stage[MEM];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      cpu->data_memory[stage->mem_address]=(stage->rs1_value);
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
      stage->buffer=cpu->data_memory[stage->mem_address];
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {

    }
    /*
    if(strcmp(stage->opcode, "HALT") == 0 &&
       stage->bubble == 0) {
      cpu->stage[EX].pc = 0;
      strcpy(cpu->stage[EX].opcode, "");
      cpu->stage[DRF].pc = 0;
      strcpy(cpu->stage[DRF].opcode, "");
      cpu->stage[EX].stalled = 1;
      cpu->stage[DRF].stalled = 1;
      cpu->stage[F].stalled = 1;
      cpu->stage[F].pc = 0;
*/
    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory", stage);
    }

  }
//  
  else{
    cpu->stage[WB] = cpu->stage[MEM];
    if (ENABLE_DEBUG_MESSAGES) {
     print_stage_contents("Memory");
    }
 //   
  }
  
  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
int
writeback(APEX_CPU* cpu)
{

  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled) {

    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 0;
      
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      }

    if (strcmp(stage->opcode, "ADD") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
       }

    if (strcmp(stage->opcode, "SUB") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
       }

    if (strcmp(stage->opcode, "OR") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      }

    if (strcmp(stage->opcode, "AND") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      }

    if (strcmp(stage->opcode, "XOR") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      }

    if (strcmp(stage->opcode, "MUL") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      }
      /*
        if(strcmp(stage->opcode, "HALT") == 0) {
        cpu->ins_completed = cpu->code_memory_size - 1;
        cpu->stage[EX].pc = 0;
        strcpy(cpu->stage[EX].opcode, "");
        cpu->stage[DRF].pc = 0;
        strcpy(cpu->stage[DRF].opcode, "");
        cpu->stage[EX].stalled = 1;
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
        cpu->stage[F].pc = 0;
        cpu->stage[MEM].pc = 0;
        strcpy(cpu->stage[MEM].opcode, "");
        cpu->stage[MEM].stalled = 1;
      }*/
//
    cpu->regs_valid[stage->rd] = 0;
//
    cpu->ins_completed++; 
  
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Writeback", stage);
    }


  }
//  
  else{
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_contents("Writeback");
    }
  }
//  
  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 *         implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu)
{
  while (1) {

    /* All the instructions committed, so exit */
    if (cpu->ins_completed == cpu->code_memory_size) {
      printf("(apex) >> Simulation Complete");
      break;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }

    writeback(cpu);
    memory(cpu);
    execute(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;
  }
   /* Register Value */
  for(int x=0; x<16; x++){
  	printf("Register Value: %d = %d\n", x, cpu->regs[x]);	
  }
  return 0;
}
