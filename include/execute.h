#ifndef EXECUTE_H
#define EXECUTE_H
#include "types.h"

void execute_instruction(struct CPU_State* cpu, opcode op);

// --- Phase 3: MOV / ADD / SUB ---
void handle_mov_imm(struct CPU_State* cpu, byte reg_index, byte value);
void handle_mov_reg(struct CPU_State* cpu, byte dest_reg, byte src_reg);
void handle_add_imm(struct CPU_State* cpu, byte immediate);
void handle_add_reg(struct CPU_State* cpu, byte reg_index);
void handle_sub_imm(struct CPU_State* cpu, byte immediate);
void handle_sub_reg(struct CPU_State* cpu, byte reg_index);

// --- Phase 4: Jumps / CALL / RET ---
void handle_jmp(struct CPU_State* cpu, address target);
void handle_jz(struct CPU_State* cpu, address target);
void handle_jnz(struct CPU_State* cpu, address target);
void handle_jc(struct CPU_State* cpu, address target);
void handle_jnc(struct CPU_State* cpu, address target);
void handle_js(struct CPU_State* cpu, address target);
void handle_jns(struct CPU_State* cpu, address target);
void handle_call(struct CPU_State* cpu, address target);
void handle_ret(struct CPU_State* cpu);

#endif // EXECUTE_H
