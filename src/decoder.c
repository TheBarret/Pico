#include "decoder.h"
#include "memory.h"

opcode decode_instruction(struct CPU_State* cpu) {
    if (!cpu || cpu->halted) {
        return OP_INVALID;
    }

    opcode op = mem_read(cpu, cpu->pc);

    // Explicit rejection of invalid/unassigned opcodes before PC advance
    if (!is_valid_opcode(op)) {
        return OP_INVALID;
    }

    cpu->pc++;
    return op;
}

int is_valid_opcode(opcode op) {
    // Single Control / Shift Operations
    if (op <= 0x06) return 1;

    // MOV reg, imm
    if (op >= 0x10 && op <= 0x17) return 1;

    // MOV reg, reg (64 slots)
    if (op >= 0x20 && op <= 0x5F) return 1;

    // MOV reg, [addr] and MOV [addr], reg
    if (op >= 0x60 && op <= 0x6F) return 1;

    // ADD acc, reg and ADD acc, imm
    if (op >= 0x70 && op <= 0x78) return 1;

    // SUB acc, reg and SUB acc, imm
    if (op >= 0x80 && op <= 0x88) return 1;

    // AND / OR acc, reg
    if (op >= 0x90 && op <= 0x9F) return 1;

    // XOR / CMP acc, reg
    if (op >= 0xA0 && op <= 0xAF) return 1;

    // CMP acc, imm
    if (op == OP_CMP_IMM) return 1;

    // TEST acc, reg
    if (op >= 0xB8 && op <= 0xBF) return 1;

    // INC / DEC reg
    if (op >= 0xC0 && op <= 0xCF) return 1;

    // Jumps, Call, Interrupt
    if (op >= 0xD0 && op <= 0xD8) return 1;

    // All unassigned gaps (0x07-0x0F, 0x18-0x1F, 0x79-0x7F, 0x89-0x8F, 0xB1-0xB7, 0xD9-0xFF)
    return 0;
}

int is_alu_op(opcode op) {
    if (op >= 0x04 && op <= 0x06) return 1; // SHL, SHR, ROL
    if (op >= 0x70 && op <= 0x78) return 1; // ADD
    if (op >= 0x80 && op <= 0x88) return 1; // SUB
    if (op >= 0x90 && op <= 0xAF) return 1; // AND, OR, XOR, CMP (reg)
    if (op == OP_CMP_IMM) return 1;         // CMP (imm)
    if (op >= 0xB8 && op <= 0xCF) return 1; // TEST, INC, DEC
    return 0;
}

int is_branch_op(opcode op) {
    return (op >= OP_JMP && op <= OP_JNS);
}

int is_memory_op(opcode op) {
    return (op >= 0x60 && op <= 0x6F);
}

int is_stack_op(opcode op) {
    return (op == OP_RET || op == OP_IRET || op == OP_CALL || op == OP_INT);
}

int is_immediate_op(opcode op) {
    if (op >= 0x10 && op <= 0x17) return 1; // MOV reg, imm
    if (op == OP_ADD_IMM) return 1;          // ADD acc, imm
    if (op == OP_SUB_IMM) return 1;          // SUB acc, imm
    if (op == OP_CMP_IMM) return 1;          // CMP acc, imm
    return 0;
}
