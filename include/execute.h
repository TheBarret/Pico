#ifndef EXECUTE_H
#define EXECUTE_H
#include "types.h"

void execute_instruction(struct CPU_State* cpu, opcode op);

void handle_and_reg(struct CPU_State* cpu, byte reg_index);
void handle_or_reg(struct CPU_State* cpu, byte reg_index);
void handle_xor_reg(struct CPU_State* cpu, byte reg_index);

void handle_shl(struct CPU_State* cpu);
void handle_shr(struct CPU_State* cpu);
void handle_rol(struct CPU_State* cpu);

void handle_mov_imm(struct CPU_State* cpu, byte reg_index, byte value);
void handle_mov_reg(struct CPU_State* cpu, byte dest_reg, byte src_reg);

void handle_add_imm(struct CPU_State* cpu, byte immediate);
void handle_add_reg(struct CPU_State* cpu, byte reg_index);
void handle_sub_imm(struct CPU_State* cpu, byte immediate);
void handle_sub_reg(struct CPU_State* cpu, byte reg_index);

void handle_cmp_reg(struct CPU_State* cpu, byte reg_index);
void handle_cmp_imm(struct CPU_State* cpu, byte immediate);
void handle_test_reg(struct CPU_State* cpu, byte reg_index);

void handle_inc_reg(struct CPU_State* cpu, byte reg_index);
void handle_dec_reg(struct CPU_State* cpu, byte reg_index);

void handle_jmp(struct CPU_State* cpu, address target);
void handle_jz(struct CPU_State* cpu, address target);
void handle_jnz(struct CPU_State* cpu, address target);
void handle_jc(struct CPU_State* cpu, address target);
void handle_jnc(struct CPU_State* cpu, address target);
void handle_js(struct CPU_State* cpu, address target);
void handle_jns(struct CPU_State* cpu, address target);

void handle_call(struct CPU_State* cpu, address target);
void handle_ret(struct CPU_State* cpu);

void handle_mov_mem_to_reg(struct CPU_State* cpu, byte reg_index, address addr);
void handle_mov_reg_to_mem(struct CPU_State* cpu, address addr, byte reg_index);

void handle_int(struct CPU_State* cpu, byte vector);
void handle_iret(struct CPU_State* cpu);

#endif // EXECUTE_H
