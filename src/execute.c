#include "../include/execute.h"
#include "../include/types.h"
#include "../include/decoder.h"
#include "../include/alu.h"
#include "../include/memory.h"


// SHL/SHR/ROL, memory MOV, AND/OR/XOR/CMP/TEST, INC/DEC, IRET, INT, reserved/illegal opcodes,
// belongs to Phase 5 / Phase 7 and is intentionally not handled in this section.

//  local helpers: address <-> pair conversion
// (pair is {byte low; byte high;} per Layer 0; address is uint16_t)
static pair addr_to_pair(address a) {
    pair p;
    p.low  = (byte)(a & 0xFF);
    p.high = (byte)((a >> 8) & 0xFF);
    return p;
}

static address pair_to_addr(pair p) {
    return (address)((uint16_t)p.low | ((uint16_t)p.high << 8));
}

// Fetch a 16-bit little-endian address operand at cpu->pc and advance pc by 2.
static address fetch_addr_operand(struct CPU_State* cpu) {
    pair word = mem_read_word(cpu, cpu->pc);
    cpu->pc = (address)(cpu->pc + 2);
    return pair_to_addr(word);
}

// Fetch an 8-bit immediate operand at cpu->pc and advance pc by 1.
static byte fetch_imm_operand(struct CPU_State* cpu) {
    byte value = mem_read(cpu, cpu->pc);
    cpu->pc = (address)(cpu->pc + 1);
    return value;
}


// Phase 3: MOV / ADD / SUB


void handle_mov_imm(struct CPU_State* cpu, byte reg_index, byte value) {
    cpu->regs[reg_index & 0x07] = value;
}

void handle_mov_reg(struct CPU_State* cpu, byte dest_reg, byte src_reg) {
    cpu->regs[dest_reg & 0x07] = cpu->regs[src_reg & 0x07];
}

void handle_add_imm(struct CPU_State* cpu, byte immediate) {
    cpu->acc = alu_execute(cpu->acc, immediate, OP_ADD_IMM, &cpu->flags);
}

void handle_add_reg(struct CPU_State* cpu, byte reg_index) {
    // Representative opcode 0x70 selects the ADD-reg branch inside alu_execute;
    // the specific reg_index bits don't matter to the ALU, only the range does.
    cpu->acc = alu_execute(cpu->acc, cpu->regs[reg_index & 0x07], 0x70, &cpu->flags);
}

void handle_sub_imm(struct CPU_State* cpu, byte immediate) {
    cpu->acc = alu_execute(cpu->acc, immediate, OP_SUB_IMM, &cpu->flags);
}

void handle_sub_reg(struct CPU_State* cpu, byte reg_index) {
    cpu->acc = alu_execute(cpu->acc, cpu->regs[reg_index & 0x07], 0x80, &cpu->flags);
}


// Phase 4: Jumps / CALL / RET


void handle_jmp(struct CPU_State* cpu, address target) {
    cpu->pc = target;
}

void handle_jz(struct CPU_State* cpu, address target) {
    if (cpu->flags & FLAG_Z) cpu->pc = target;
}

void handle_jnz(struct CPU_State* cpu, address target) {
    if (!(cpu->flags & FLAG_Z)) cpu->pc = target;
}

void handle_jc(struct CPU_State* cpu, address target) {
    if (cpu->flags & FLAG_C) cpu->pc = target;
}

void handle_jnc(struct CPU_State* cpu, address target) {
    if (!(cpu->flags & FLAG_C)) cpu->pc = target;
}

void handle_js(struct CPU_State* cpu, address target) {
    if (cpu->flags & FLAG_S) cpu->pc = target;
}

void handle_jns(struct CPU_State* cpu, address target) {
    if (!(cpu->flags & FLAG_S)) cpu->pc = target;
}

void handle_call(struct CPU_State* cpu, address target) {
    // cpu->pc already points at the instruction *after* CALL's operand
    // (execute_instruction fetches the address operand before calling this),
    // so this is the correct return address to push.
    push_word(cpu, addr_to_pair(cpu->pc));
    cpu->pc = target;
}

void handle_ret(struct CPU_State* cpu) {
    pair ret_addr = pop_word(cpu);
    cpu->pc = pair_to_addr(ret_addr);
}


// Dispatcher


void execute_instruction(struct CPU_State* cpu, opcode op) {
    //  Layer-0/control instructions covered so far
    if (op == OP_NOP) {
        return;
    }
    if (op == OP_HALT) {
        cpu->halted = 1;
        return;
    }
    if (op == OP_RET) {
        handle_ret(cpu);
        return;
    }

    //  MOV reg, imm : 0x10-0x17
    if (op >= 0x10 && op <= 0x17) {
        byte reg_index = op & 0x07;
        byte value = fetch_imm_operand(cpu);
        handle_mov_imm(cpu, reg_index, value);
        return;
    }

    //  MOV reg, reg : 0x20-0x5F  (opcode = 0x20 + (bbb<<3 | aaa))
    if (op >= 0x20 && op <= 0x5F) {
        byte field = (byte)(op - 0x20);
        byte dest_reg = (field >> 3) & 0x07;
        byte src_reg  = field & 0x07;
        handle_mov_reg(cpu, dest_reg, src_reg);
        return;
    }

    //  ADD acc, reg : 0x70-0x77
    if (op >= 0x70 && op <= 0x77) {
        byte reg_index = op & 0x07;
        handle_add_reg(cpu, reg_index);
        return;
    }

    //  ADD acc, imm : 0x78
    if (op == OP_ADD_IMM) {
        byte value = fetch_imm_operand(cpu);
        handle_add_imm(cpu, value);
        return;
    }

    //  SUB acc, reg : 0x80-0x87
    if (op >= 0x80 && op <= 0x87) {
        byte reg_index = op & 0x07;
        handle_sub_reg(cpu, reg_index);
        return;
    }

    //  SUB acc, imm : 0x88
    if (op == OP_SUB_IMM) {
        byte value = fetch_imm_operand(cpu);
        handle_sub_imm(cpu, value);
        return;
    }

    //  Jumps / CALL : 0xD0-0xD7, all take a 16-bit address operand
    if (op == OP_JMP) {
        address target = fetch_addr_operand(cpu);
        handle_jmp(cpu, target);
        return;
    }
    if (op == OP_JZ) {
        address target = fetch_addr_operand(cpu);
        handle_jz(cpu, target);
        return;
    }
    if (op == OP_JNZ) {
        address target = fetch_addr_operand(cpu);
        handle_jnz(cpu, target);
        return;
    }
    if (op == OP_JC) {
        address target = fetch_addr_operand(cpu);
        handle_jc(cpu, target);
        return;
    }
    if (op == OP_JNC) {
        address target = fetch_addr_operand(cpu);
        handle_jnc(cpu, target);
        return;
    }
    if (op == OP_JS) {
        address target = fetch_addr_operand(cpu);
        handle_js(cpu, target);
        return;
    }
    if (op == OP_JNS) {
        address target = fetch_addr_operand(cpu);
        handle_jns(cpu, target);
        return;
    }
    if (op == OP_CALL) {
        address target = fetch_addr_operand(cpu);
        handle_call(cpu, target);
        return;
    }

}
