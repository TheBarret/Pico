#ifndef DECODER_H
#define DECODER_H

#include "types.h"

// Opcode Definitions (PICO rev 0.3)
#define OP_NOP          0x00
#define OP_HALT         0x01
#define OP_RET          0x02
#define OP_IRET         0x03
#define OP_SHL          0x04
#define OP_SHR          0x05
#define OP_ROL          0x06

#define OP_ADD_IMM      0x78
#define OP_SUB_IMM      0x88
#define OP_CMP_IMM      0xB0

#define OP_JMP          0xD0
#define OP_JZ           0xD1
#define OP_JNZ          0xD2
#define OP_JC           0xD3
#define OP_JNC          0xD4
#define OP_JS           0xD5
#define OP_JNS          0xD6
#define OP_CALL         0xD7
#define OP_INT          0xD8

#define OP_INVALID      0xFF

// Core Decode Function
opcode decode_instruction(struct CPU_State* cpu);

// Instruction Category Classifiers
int is_valid_opcode(opcode op);
int is_alu_op(opcode op);
int is_branch_op(opcode op);
int is_memory_op(opcode op);
int is_stack_op(opcode op);
int is_immediate_op(opcode op);

#endif // DECODER_H
