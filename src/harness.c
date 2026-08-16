#include "../include/harness.h"
#include "../include/alu.h"
#include "../include/state.h"
#include "../include/memory.h"
#include "../include/host.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Layer 4: Test Harness Implementation */

struct CPU_State* create_expected_state(void) {
    return cpu_create();
}

void calculate_mathematical_result(struct CPU_State* expected, opcode op, byte operand1, byte operand2) {
    byte flags = 0;
    byte result = alu_execute(operand1, operand2, op, &flags);

    expected->acc = result;
    expected->flags = flags;
}

int compare_cpu_states(struct CPU_State* actual, struct CPU_State* expected) {
    int mismatch_count = 0;

    if (actual->pc != expected->pc) {
        report_pc_mismatch(actual->pc, expected->pc);
        mismatch_count++;
    }

    if (actual->sp != expected->sp) {
        printf("SP mismatch: actual 0x%02X, expected 0x%02X\n", actual->sp, expected->sp);
        mismatch_count++;
    }

    if (actual->acc != expected->acc) {
        report_register_mismatch(-1, actual->acc, expected->acc);
        mismatch_count++;
    }

    if (actual->flags != expected->flags) {
        if ((actual->flags & FLAG_Z) != (expected->flags & FLAG_Z)) {
            report_flag_mismatch('Z', actual->flags & FLAG_Z, expected->flags & FLAG_Z);
        }
        if ((actual->flags & FLAG_C) != (expected->flags & FLAG_C)) {
            report_flag_mismatch('C', actual->flags & FLAG_C, expected->flags & FLAG_C);
        }
        if ((actual->flags & FLAG_S) != (expected->flags & FLAG_S)) {
            report_flag_mismatch('S', actual->flags & FLAG_S, expected->flags & FLAG_S);
        }
        if ((actual->flags & FLAG_O) != (expected->flags & FLAG_O)) {
            report_flag_mismatch('O', actual->flags & FLAG_O, expected->flags & FLAG_O);
        }
        mismatch_count++;
    }

    for (int i = 0; i < REGISTER_COUNT; i++) {
        if (actual->regs[i] != expected->regs[i]) {
            report_register_mismatch(i, actual->regs[i], expected->regs[i]);
            mismatch_count++;
        }
    }

    return mismatch_count;
}

void run_test_sequence(byte* program, size_t program_size, unsigned int steps) {
    struct CPU_State* cpu = cpu_create();
    struct CPU_State* expected = cpu_create();

    if (!cpu || !expected) {
        if (cpu) cpu_destroy(cpu);
        if (expected) cpu_destroy(expected);
        return;
    }

    for (size_t i = 0; i < program_size && i < RAM_SIZE; i++) {
        mem_write(cpu, (address)i, program[i]);
    }

    cpu->pc = 0x0000;

    for (unsigned int i = 0; i < steps && !cpu->halted; i++) {
        cpu_step(cpu);
    }

    cpu_destroy(cpu);
    cpu_destroy(expected);
}

void report_register_mismatch(int reg_index, byte actual, byte expected) {
    if (reg_index == -1) {
        printf("ACC mismatch: actual 0x%02X, expected 0x%02X\n", actual, expected);
    } else {
        printf("R%d mismatch: actual 0x%02X, expected 0x%02X\n", reg_index, actual, expected);
    }
}

void report_flag_mismatch(char flag_name, unsigned int actual, unsigned int expected) {
    printf("Flag %c mismatch: actual %d, expected %d\n", flag_name, actual ? 1 : 0, expected ? 1 : 0);
}

void report_pc_mismatch(address actual, address expected) {
    printf("PC mismatch: actual 0x%04X, expected 0x%04X\n", actual, expected);
}

void report_memory_mismatch(address addr, byte actual, byte expected) {
    printf("Memory 0x%04X mismatch: actual 0x%02X, expected 0x%02X\n", addr, actual, expected);
}
