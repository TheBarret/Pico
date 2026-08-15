#include <stdio.h>
#include <assert.h>

#include "../include/state.h"
#include "../include/memory.h"
#include "../include/types.h"
#include "../include/decoder.h"

/* Layer 1 Test Suite */

void test_memory_read_write(void) {
    struct CPU_State* cpu = cpu_create();

    /* Test byte write/read */
    mem_write(cpu, 0x0000, 0xAB);
    assert(mem_read(cpu, 0x0000) == 0xAB);

    /* Test byte at max address */
    mem_write(cpu, 0xFFFF, 0xCD);
    assert(mem_read(cpu, 0xFFFF) == 0xCD);

    /* Test byte at random address */
    mem_write(cpu, 0x1234, 0xEF);
    assert(mem_read(cpu, 0x1234) == 0xEF);

    cpu_destroy(cpu);
    printf("test_memory_read_write: PASSED\n");
}

void test_memory_word_operations(void) {
    struct CPU_State* cpu = cpu_create();
    pair test_word;

    /* Test word write/read (little-endian) */
    test_word.low = 0x78;
    test_word.high = 0x56;
    mem_write_word(cpu, 0x1000, test_word);

    pair read_word = mem_read_word(cpu, 0x1000);
    assert(read_word.low == 0x78);
    assert(read_word.high == 0x56);

    /* Verify memory layout */
    assert(mem_read(cpu, 0x1000) == 0x78);
    assert(mem_read(cpu, 0x1001) == 0x56);

    cpu_destroy(cpu);
    printf("test_memory_word_operations: PASSED\n");
}

void test_stack_operations(void) {
    struct CPU_State* cpu = cpu_create();
    cpu_reset(cpu);

    /* Test byte push/pop */
    push(cpu, 0xAA);
    assert(cpu->sp == 0xFE);
    assert(mem_read(cpu, STACK_PAGE + 0xFF) == 0xAA);

    byte popped = pop(cpu);
    assert(popped == 0xAA);
    assert(cpu->sp == 0xFF);

    /* Test multiple pushes */
    push(cpu, 0x01);
    push(cpu, 0x02);
    push(cpu, 0x03);
    assert(cpu->sp == 0xFC);
    assert(mem_read(cpu, STACK_PAGE + 0xFF) == 0x01);
    assert(mem_read(cpu, STACK_PAGE + 0xFE) == 0x02);
    assert(mem_read(cpu, STACK_PAGE + 0xFD) == 0x03);

    /* Test multiple pops (LIFO order) */
    popped = pop(cpu);
    assert(popped == 0x03);
    popped = pop(cpu);
    assert(popped == 0x02);
    popped = pop(cpu);
    assert(popped == 0x01);
    assert(cpu->sp == 0xFF);

    cpu_destroy(cpu);
    printf("test_stack_operations: PASSED\n");
}

void test_stack_word_operations(void) {
    struct CPU_State* cpu = cpu_create();
    cpu_reset(cpu);

    pair test_word;
    test_word.low = 0x12;
    test_word.high = 0x34;

    /* Test word push (high byte first, then low byte) */
    push_word(cpu, test_word);
    assert(cpu->sp == 0xFD);
    assert(mem_read(cpu, STACK_PAGE + 0xFF) == 0x34);
    assert(mem_read(cpu, STACK_PAGE + 0xFE) == 0x12);

    /* Test word pop (low byte first, then high byte) */
    pair popped_word = pop_word(cpu);
    assert(popped_word.low == 0x12);
    assert(popped_word.high == 0x34);
    assert(cpu->sp == 0xFF);

    cpu_destroy(cpu);
    printf("test_stack_word_operations: PASSED\n");
}

void test_memory_reset(void) {
    struct CPU_State* cpu = cpu_create();

    /* Write test data */
    mem_write(cpu, 0x0000, 0xFF);
    mem_write(cpu, 0x1000, 0xAA);
    mem_write(cpu, 0xFFFF, 0x55);

    /* Reset memory */
    mem_reset(cpu);

    /* Verify all zero */
    assert(mem_read(cpu, 0x0000) == 0x00);
    assert(mem_read(cpu, 0x1000) == 0x00);
    assert(mem_read(cpu, 0xFFFF) == 0x00);

    cpu_destroy(cpu);
    printf("test_memory_reset: PASSED\n");
}

void test_state_management(void) {
    struct CPU_State* cpu = cpu_create();

    /* Verify initial state */
    assert(cpu->pc == 0x0000);
    assert(cpu->sp == 0xFF);
    assert(cpu->flags == 0x00);
    assert(cpu->halted == 0);
    assert(cpu->interrupt_enabled == 0);
    assert(cpu->cycles == 0);
    assert(cpu->acc == 0x00);

    for (int i = 0; i < REGISTER_COUNT; i++) {
        assert(cpu->regs[i] == 0x00);
    }

    /* Modify state */
    cpu->pc = 0x1234;
    cpu->sp = 0x80;
    cpu->flags = FLAG_Z | FLAG_C;
    cpu->halted = 1;
    cpu->interrupt_enabled = 1;
    cpu->cycles = 100;
    cpu->acc = 0xAA;
    cpu->regs[0] = 0x01;
    cpu->regs[3] = 0x02;
    cpu->regs[7] = 0x03;

    /* Reset state */
    cpu_reset(cpu);

    /* Verify reset */
    assert(cpu->pc == 0x0000);
    assert(cpu->sp == 0xFF);
    assert(cpu->flags == 0x00);
    assert(cpu->halted == 0);
    assert(cpu->interrupt_enabled == 0);
    assert(cpu->cycles == 0);
    assert(cpu->acc == 0x00);

    for (int i = 0; i < REGISTER_COUNT; i++) {
        assert(cpu->regs[i] == 0x00);
    }

    cpu_destroy(cpu);
    printf("test_state_management: PASSED\n");
}

void test_hard_reset(void) {
    struct CPU_State* cpu = cpu_create();

    /* Write memory and modify state */
    mem_write(cpu, 0x1000, 0xFF);
    cpu->pc = 0x5678;
    cpu->flags = FLAG_S | FLAG_O;

    /* Hard reset */
    cpu_hard_reset(cpu);

    /* Verify memory and state reset */
    assert(mem_read(cpu, 0x1000) == 0x00);
    assert(cpu->pc == 0x0000);
    assert(cpu->sp == 0xFF);
    assert(cpu->flags == 0x00);
    assert(cpu->halted == 0);
    assert(cpu->interrupt_enabled == 0);
    assert(cpu->cycles == 0);

    cpu_destroy(cpu);
    printf("test_hard_reset: PASSED\n");
}

void test_multiple_cpu_instances(void) {
    struct CPU_State* cpu1 = cpu_create();
    struct CPU_State* cpu2 = cpu_create();

    /* Setup independent states */
    cpu1->pc = 0x1000;
    cpu1->acc = 0xAA;
    mem_write(cpu1, 0x0000, 0x01);

    cpu2->pc = 0x2000;
    cpu2->acc = 0xBB;
    mem_write(cpu2, 0x0000, 0x02);

    /* Verify independence */
    assert(cpu1->pc == 0x1000);
    assert(cpu2->pc == 0x2000);
    assert(cpu1->acc == 0xAA);
    assert(cpu2->acc == 0xBB);
    assert(mem_read(cpu1, 0x0000) == 0x01);
    assert(mem_read(cpu2, 0x0000) == 0x02);

    /* Verify cross-operation */
    mem_write(cpu1, 0x0001, 0x03);
    mem_write(cpu2, 0x0001, 0x04);
    assert(mem_read(cpu1, 0x0001) == 0x03);
    assert(mem_read(cpu2, 0x0001) == 0x04);

    cpu_destroy(cpu1);
    cpu_destroy(cpu2);
    printf("test_multiple_cpu_instances: PASSED\n");
}

/*
    int main(void) {
        test_memory_read_write();
        test_memory_word_operations();
        test_stack_operations();
        test_stack_word_operations();
        test_memory_reset();
        test_state_management();
        test_hard_reset();
        test_multiple_cpu_instances();
        return 0;
    }
*/

//####################



// Test Helper: Load single opcode into memory and verify decoder response
static void test_single_opcode(struct CPU_State* cpu, address pc_start, opcode op, int expected_valid) {
    cpu->pc = pc_start;
    mem_write(cpu, pc_start, op);

    opcode decoded = decode_instruction(cpu);

    if (expected_valid) {
        assert(decoded == op);
        assert(cpu->pc == (address)(pc_start + 1)); // Valid opcodes advance PC
    } else {
        assert(decoded == OP_INVALID);
        assert(cpu->pc == pc_start);                 // Invalid opcodes preserve PC
    }
}

// 1. Full 256-Opcode Exhaustive Map Scan
static void test_all_opcode_bounds(struct CPU_State* cpu) {
    printf("[1/4] Scanning full 0x00-0xFF opcode boundary spectrum...\n");

    for (int op = 0x00; op <= 0xFF; op++) {
        opcode code = (opcode)op;
        int expected = is_valid_opcode(code);

        // Spot-check known valid bounds
        if ((code <= 0x06) ||
            (code >= 0x10 && code <= 0x17) ||
            (code >= 0x20 && code <= 0x5F) ||
            (code >= 0x60 && code <= 0x6F) ||
            (code >= 0x70 && code <= 0x78) ||
            (code >= 0x80 && code <= 0x88) ||
            (code >= 0x90 && code <= 0xAF) ||
            (code == OP_CMP_IMM) ||
            (code >= 0xB8 && code <= 0xCF) ||
            (code >= 0xD0 && code <= 0xD8)) {
            assert(expected == 1);
        }

        // Spot-check explicit reserved/gap ranges
        if ((code >= 0x07 && code <= 0x0F) ||
            (code >= 0x18 && code <= 0x1F) ||
            (code >= 0x79 && code <= 0x7F) ||
            (code >= 0x89 && code <= 0x8F) ||
            (code >= 0xB1 && code <= 0xB7) ||
            (code >= 0xD9 && code <= 0xFF)) {
            assert(expected == 0);
        }

        test_single_opcode(cpu, 0x0200, code, expected);
    }
}

// 2. Immediate Operands Scope Constraint Validation
static void test_immediate_scope_constraints(void) {
    printf("[2/4] Testing explicit immediate-operand restrictions...\n");

    // Valid Immediates
    assert(is_immediate_op(0x10)); // MOV R0, imm
    assert(is_immediate_op(0x17)); // MOV R7, imm
    assert(is_immediate_op(OP_ADD_IMM)); // 0x78
    assert(is_immediate_op(OP_SUB_IMM)); // 0x88
    assert(is_immediate_op(OP_CMP_IMM)); // 0xB0

    // Banned Bitwise Immediates (Register-only enforcement)
    assert(!is_immediate_op(0x90)); // AND acc, R0
    assert(!is_immediate_op(0x98)); // OR acc, R0
    assert(!is_immediate_op(0xA0)); // XOR acc, R0
    assert(!is_immediate_op(0xB8)); // TEST acc, R0
}

// 3. Category Classifier Predicates
static void test_category_classifiers(void) {
    printf("[3/4] Verifying category detection helpers...\n");

    // ALU
    assert(is_alu_op(OP_SHL));
    assert(is_alu_op(0x70)); // ADD reg
    assert(is_alu_op(0xA5)); // XOR reg
    assert(is_alu_op(0xC3)); // INC reg
    assert(!is_alu_op(OP_JMP));

    // Branching
    assert(is_branch_op(OP_JMP));
    assert(is_branch_op(OP_JZ));
    assert(is_branch_op(OP_JNS));
    assert(!is_branch_op(OP_CALL)); // CALL is stack/control

    // Memory
    assert(is_memory_op(0x60)); // MOV reg, [addr]
    assert(is_memory_op(0x6F)); // MOV [addr], reg
    assert(!is_memory_op(0x20)); // MOV reg, reg

    // Stack Operations
    assert(is_stack_op(OP_RET));
    assert(is_stack_op(OP_IRET));
    assert(is_stack_op(OP_CALL));
    assert(is_stack_op(OP_INT));
    assert(!is_stack_op(OP_JMP));
}

// 4. Edge Cases: CPU Halt Guard
static void test_halted_cpu_guard(struct CPU_State* cpu) {
    printf("[4/4] Testing halted CPU execution prevention...\n");

    cpu->pc = 0x0000;
    cpu->halted = 1;
    mem_write(cpu, 0x0000, OP_NOP);

    opcode decoded = decode_instruction(cpu);
    assert(decoded == OP_INVALID);
    assert(cpu->pc == 0x0000); // PC must not advance when halted
}

int main(void) {

    printf("Group 1:\n");

    test_memory_read_write();
    test_memory_word_operations();
    test_stack_operations();
    test_stack_word_operations();
    test_memory_reset();
    test_state_management();
    test_hard_reset();
    test_multiple_cpu_instances();

    printf("Group 2:\n");

    struct CPU_State* cpu = cpu_create();
    assert(cpu != NULL);

    test_all_opcode_bounds(cpu);
    test_immediate_scope_constraints();
    test_category_classifiers();
    test_halted_cpu_guard(cpu);

    cpu_destroy(cpu);
    return 0;
}
