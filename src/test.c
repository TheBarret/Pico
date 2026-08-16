#include <stdio.h>
#include <assert.h>

#include "../include/state.h"
#include "../include/memory.h"
#include "../include/types.h"
#include "../include/decoder.h"
#include "../include/alu.h"
#include "../include/execute.h"

// Unit testers


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
    printf("[1/8] Testing memory read write...\n");
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
    printf("[2/8] Testing memory word operations...\n");
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
    printf("[3/8] Testing stack operations...\n");
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
    printf("[4/8] Testing stack word operations...\n");
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
    printf("[5/8] Testing memory reset...\n");
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
    printf("[6/8] Testing state management...\n");
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
    printf("[7/8] Testing hard reset...\n");
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
    printf("[8/8] Testing multiple cpu instances...\n");
}

// Helper macros to check flags
//#define CHECK_FLAG(flags, bit) (((flags) & (1 << (bit))) != 0)
#define CHECK_FLAG(flags, mask) (((flags) & (mask)) != 0)

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


static void test_add_flags(void) {
    printf("[1/4] Testing ADD flag boundary conditions...\n");
    byte flags = 0;
    byte res;

    // 1. Zero Flag Test: 0 + 0 = 0
    flags = 0;
    res = alu_execute(0x00, 0x00, 0x70, &flags);
    assert(res == 0x00);
    assert(CHECK_FLAG(flags, FLAG_Z));
    assert(!CHECK_FLAG(flags, FLAG_C));
    assert(!CHECK_FLAG(flags, FLAG_S));
    assert(!CHECK_FLAG(flags, FLAG_O));

    // 2. Carry Flag Test: 0xFF + 0x01 = 0x00 (Overflows byte, sets Carry + Zero)
    flags = 0;
    res = alu_execute(0xFF, 0x01, 0x70, &flags);
    assert(res == 0x00);
    assert(CHECK_FLAG(flags, FLAG_Z));
    assert(CHECK_FLAG(flags, FLAG_C));
    assert(!CHECK_FLAG(flags, FLAG_S));
    assert(!CHECK_FLAG(flags, FLAG_O));

    // 3. Sign Flag Test: 0x40 + 0x40 = 0x80 (-128 in signed 8-bit)
    flags = 0;
    res = alu_execute(0x40, 0x40, 0x70, &flags);
    assert(res == 0x80);
    assert(!CHECK_FLAG(flags, FLAG_Z));
    assert(!CHECK_FLAG(flags, FLAG_C));
    assert(CHECK_FLAG(flags, FLAG_S));
    assert(CHECK_FLAG(flags, FLAG_O)); // Positive + Positive = Negative -> Overflow!

    // 4. Signed Overflow (Negative + Negative = Positive): 0x80 (-128) + 0x80 (-128) = 0x00
    flags = 0;
    res = alu_execute(0x80, 0x80, 0x70, &flags);
    assert(res == 0x00);
    assert(CHECK_FLAG(flags, FLAG_Z));
    assert(CHECK_FLAG(flags, FLAG_C));
    assert(!CHECK_FLAG(flags, FLAG_S));
    assert(CHECK_FLAG(flags, FLAG_O)); // Negative + Negative = Positive -> Overflow!
}

static void test_sub_flags(void) {
    printf("[2/4] Testing SUB / CMP flag boundary conditions...\n");
    byte flags = 0;
    byte res;

    // 1. Equal values: 0x50 - 0x50 = 0x00
    flags = 0;
    res = alu_execute(0x50, 0x50, 0x80, &flags);
    assert(res == 0x00);
    assert(CHECK_FLAG(flags, FLAG_Z));
    assert(!CHECK_FLAG(flags, FLAG_C));
    assert(!CHECK_FLAG(flags, FLAG_S));
    assert(!CHECK_FLAG(flags, FLAG_O));

    // 2. Borrow / Carry Flag: 0x00 - 0x01 = 0xFF (-1)
    flags = 0;
    res = alu_execute(0x00, 0x01, 0x80, &flags);
    assert(res == 0xFF);
    assert(!CHECK_FLAG(flags, FLAG_Z));
    assert(CHECK_FLAG(flags, FLAG_C)); // Borrow required
    assert(CHECK_FLAG(flags, FLAG_S)); // Result has bit 7 set
    assert(!CHECK_FLAG(flags, FLAG_O));

    // 3. Signed Overflow: Positive - Negative = Negative (0x7F [127] - 0xFF [-1] = 0x80 [-128])
    flags = 0;
    res = alu_execute(0x7F, 0xFF, 0x80, &flags);
    assert(res == 0x80);
    assert(!CHECK_FLAG(flags, FLAG_Z));
    assert(CHECK_FLAG(flags, FLAG_C)); // 0x7F < 0xFF unsigned
    assert(CHECK_FLAG(flags, FLAG_S));
    assert(CHECK_FLAG(flags, FLAG_O)); // Pos - Neg resulted in Neg -> Overflow!

    // 4. CMP immediate non-destructive flag calculation
    flags = 0;
    res = alu_execute(0x10, 0x20, OP_CMP_IMM, &flags);
    assert(res == 0xF0); // 0x10 - 0x20 = 0xF0 (-16)
    assert(CHECK_FLAG(flags, FLAG_C)); // Borrow required
    assert(CHECK_FLAG(flags, FLAG_S)); // Negative result
}

static void test_inc_dec_flags(void) {
    printf("[3/4] Testing INC / DEC boundary conditions...\n");
    byte flags = 0;
    byte res;

    // INC roll-over (0xFF -> 0x00)
    flags = 0;
    res = alu_execute(0xFF, 0x00, 0xC0, &flags); // INC R0
    assert(res == 0x00);
    assert(CHECK_FLAG(flags, FLAG_Z));
    assert(CHECK_FLAG(flags, FLAG_C));

    // DEC roll-under (0x00 -> 0xFF)
    flags = 0;
    res = alu_execute(0x00, 0x00, 0xC8, &flags); // DEC R0
    assert(res == 0xFF);
    assert(!CHECK_FLAG(flags, FLAG_Z));
    assert(CHECK_FLAG(flags, FLAG_C));
    assert(CHECK_FLAG(flags, FLAG_S));
}

static void test_shift_flags(void) {
    printf("[4/4] Testing SHL, SHR, ROL flag updates...\n");
    byte flags = 0;
    byte res;

    // SHL 0x80 -> 0x00 with Carry
    flags = 0;
    res = alu_execute(0x80, 0, OP_SHL, &flags);
    assert(res == 0x00);
    assert(CHECK_FLAG(flags, FLAG_Z));
    assert(CHECK_FLAG(flags, FLAG_C));

    // SHR 0x01 -> 0x00 with Carry
    flags = 0;
    res = alu_execute(0x01, 0, OP_SHR, &flags);
    assert(res == 0x00);
    assert(CHECK_FLAG(flags, FLAG_Z));
    assert(CHECK_FLAG(flags, FLAG_C));

    // ROL 0x01 with Carry set -> 0x03
    //flags = (1 << FLAG_C); (wrong)
    flags = FLAG_C;
    res = alu_execute(0x01, 0, OP_ROL, &flags);
    assert(res == 0x03);
    assert(!CHECK_FLAG(flags, FLAG_C)); // High bit of 0x01 was 0
}

// Group 4 (execute): Phase 3 + Phase 4 instruction execution tests

// Helper: write one opcode + operand bytes at addr, return addr past them.
static address load_bytes(struct CPU_State* cpu, address addr, const byte* bytes, int count) {
    for (int i = 0; i < count; i++) {
        mem_write(cpu, (address)(addr + i), bytes[i]);
    }
    return (address)(addr + count);
}

// Helper: run exactly one fetch-decode-execute cycle from cpu->pc.
static void step_once(struct CPU_State* cpu) {
    opcode op = decode_instruction(cpu);
    execute_instruction(cpu, op);
}

static void test_mov_execute(void) {
    printf("[1/4] Testing MOV imm / MOV reg,reg execution...\n");
    struct CPU_State* cpu = cpu_create();
    assert(cpu != NULL);
    cpu_reset(cpu);

    // MOV R0, 0x42   (0x10 | reg=0)
    // MOV R1, R0     (0x20 + (1<<3 | 0) = 0x28)
    byte prog[] = { 0x10, 0x42, 0x28 };
    cpu->pc = 0x0200;
    load_bytes(cpu, 0x0200, prog, 3);

    step_once(cpu); // MOV R0, 0x42
    assert(cpu->regs[0] == 0x42);
    assert(cpu->pc == 0x0202);

    step_once(cpu); // MOV R1, R0
    assert(cpu->regs[1] == 0x42);
    assert(cpu->pc == 0x0203);

    cpu_destroy(cpu);
}

static void test_add_sub_execute(void) {
    printf("[2/4] Testing ADD/SUB imm and reg execution...\n");
    struct CPU_State* cpu = cpu_create();
    assert(cpu != NULL);
    cpu_reset(cpu);

    // MOV R0, 0x05        (0x10, 0x05)
    // ADD acc, 0x10        (OP_ADD_IMM=0x78, 0x10)   -> acc = 0x10
    // ADD acc, R0           (0x70 | reg=0)             -> acc = 0x15
    // SUB acc, 0x05         (OP_SUB_IMM=0x88, 0x05)   -> acc = 0x10
    // SUB acc, R0           (0x80 | reg=0)             -> acc = 0x0B
    byte prog[] = { 0x10, 0x05, 0x78, 0x10, 0x70, 0x88, 0x05, 0x80 };
    cpu->pc = 0x0200;
    load_bytes(cpu, 0x0200, prog, 8);

    step_once(cpu); // MOV R0, 0x05
    assert(cpu->regs[0] == 0x05);

    step_once(cpu); // ADD acc, 0x10
    assert(cpu->acc == 0x10);
    assert(!CHECK_FLAG(cpu->flags, FLAG_Z));

    step_once(cpu); // ADD acc, R0
    assert(cpu->acc == 0x15);

    step_once(cpu); // SUB acc, 0x05
    assert(cpu->acc == 0x10);

    step_once(cpu); // SUB acc, R0
    assert(cpu->acc == 0x0B);

    cpu_destroy(cpu);
}

static void test_jump_execute(void) {
    printf("[3/4] Testing conditional/unconditional jump execution...\n");
    struct CPU_State* cpu = cpu_create();
    assert(cpu != NULL);
    cpu_reset(cpu);

    // Unconditional JMP
    // JMP 0x0300   (OP_JMP=0xD0, low=0x00, high=0x03)
    {
        byte prog[] = { 0xD0, 0x00, 0x03 };
        cpu->pc = 0x0200;
        load_bytes(cpu, 0x0200, prog, 3);
        step_once(cpu);
        assert(cpu->pc == 0x0300);
    }

    // JZ taken (FLAG_Z set)
    {
        byte prog[] = { 0xD1, 0x00, 0x04 }; // JZ 0x0400
        cpu->pc = 0x0210;
        load_bytes(cpu, 0x0210, prog, 3);
        cpu->flags = FLAG_Z;
        step_once(cpu);
        assert(cpu->pc == 0x0400);
    }

    // JZ not taken (FLAG_Z clear) -> falls through to just past operand
    {
        byte prog[] = { 0xD1, 0x00, 0x04 }; // JZ 0x0400
        cpu->pc = 0x0220;
        load_bytes(cpu, 0x0220, prog, 3);
        cpu->flags = 0;
        step_once(cpu);
        assert(cpu->pc == 0x0223); // did NOT jump; pc past the 3-byte instruction
    }

    // JNZ / JC / JNC / JS / JNS spot checks
    {
        byte prog[] = { 0xD2, 0x00, 0x05 }; // JNZ 0x0500
        cpu->pc = 0x0230;
        load_bytes(cpu, 0x0230, prog, 3);
        cpu->flags = 0; // Z clear -> JNZ taken
        step_once(cpu);
        assert(cpu->pc == 0x0500);
    }
    {
        byte prog[] = { 0xD3, 0x00, 0x06 }; // JC 0x0600
        cpu->pc = 0x0240;
        load_bytes(cpu, 0x0240, prog, 3);
        cpu->flags = FLAG_C;
        step_once(cpu);
        assert(cpu->pc == 0x0600);
    }
    {
        byte prog[] = { 0xD4, 0x00, 0x07 }; // JNC 0x0700
        cpu->pc = 0x0250;
        load_bytes(cpu, 0x0250, prog, 3);
        cpu->flags = 0;
        step_once(cpu);
        assert(cpu->pc == 0x0700);
    }
    {
        byte prog[] = { 0xD5, 0x00, 0x08 }; // JS 0x0800
        cpu->pc = 0x0260;
        load_bytes(cpu, 0x0260, prog, 3);
        cpu->flags = FLAG_S;
        step_once(cpu);
        assert(cpu->pc == 0x0800);
    }
    {
        byte prog[] = { 0xD6, 0x00, 0x09 }; // JNS 0x0900
        cpu->pc = 0x0270;
        load_bytes(cpu, 0x0270, prog, 3);
        cpu->flags = 0;
        step_once(cpu);
        assert(cpu->pc == 0x0900);
    }

    cpu_destroy(cpu);
}

static void test_call_ret_execute(void) {
    printf("[4/4] Testing CALL/RET stack integrity...\n");
    struct CPU_State* cpu = cpu_create();
    assert(cpu != NULL);
    cpu_reset(cpu);

    uint8_t sp_before = cpu->sp;

    // At 0x0200: CALL 0x0300
    // At 0x0300: RET
    byte call_instr[] = { 0xD7, 0x00, 0x03 }; // CALL 0x0300
    byte ret_instr[]  = { 0x02 };             // RET

    cpu->pc = 0x0200;
    load_bytes(cpu, 0x0200, call_instr, 3);
    load_bytes(cpu, 0x0300, ret_instr, 1);

    step_once(cpu); // CALL 0x0300
    assert(cpu->pc == 0x0300);
    assert(cpu->sp != sp_before); // stack pointer moved (return addr pushed)

    step_once(cpu); // RET
    assert(cpu->pc == 0x0203); // back to instruction right after the 3-byte CALL
    assert(cpu->sp == sp_before); // stack balanced after matching RET

    cpu_destroy(cpu);
}

// Bit Shifting tests

static void test_shift_execute(void) {
    struct CPU_State* cpu = cpu_create();
    assert(cpu != NULL);
    cpu_reset(cpu);

    // SHL: acc=0x81 -> 0x02, carry set (bit 7 was 1)
    printf("[1/3] Testing SHL execution...\n");
    cpu->acc = 0x81;
    cpu->flags = 0;
    byte prog1[] = { OP_SHL };
    cpu->pc = 0x0200;
    load_bytes(cpu, 0x0200, prog1, 1);
    step_once(cpu);
    assert(cpu->acc == 0x02);
    assert(CHECK_FLAG(cpu->flags, FLAG_C));

    // SHR: acc=0x03 -> 0x01, carry set (bit 0 was 1)
    printf("[2/3] Testing SHR execution...\n");
    cpu->acc = 0x03;
    cpu->flags = 0;
    byte prog2[] = { OP_SHR };
    cpu->pc = 0x0210;
    load_bytes(cpu, 0x0210, prog2, 1);
    step_once(cpu);
    assert(cpu->acc == 0x01);
    assert(CHECK_FLAG(cpu->flags, FLAG_C));

    // ROL: acc=0x80 with carry-in clear -> 0x00, carry-out set (bit 7 was 1)
    printf("[3/3] Testing ROL execution...\n");
    cpu->acc = 0x80;
    cpu->flags = 0;
    byte prog3[] = { OP_ROL };
    cpu->pc = 0x0220;
    load_bytes(cpu, 0x0220, prog3, 1);
    step_once(cpu);
    assert(cpu->acc == 0x00);
    assert(CHECK_FLAG(cpu->flags, FLAG_C));
    assert(CHECK_FLAG(cpu->flags, FLAG_Z));

    // ROL: acc=0x01 with carry-in SET (from previous op) -> 0x03
    byte prog4[] = { OP_ROL };
    cpu->pc = 0x0230;
    load_bytes(cpu, 0x0230, prog4, 1);
    cpu->acc = 0x01;
    // cpu->flags currently has FLAG_C set from the previous ROL result
    step_once(cpu);
    assert(cpu->acc == 0x03);

    cpu_destroy(cpu);
}

// Mov testers

static void test_mov_mem_execute(void) {
    printf("[1/2] Testing MOV reg,[addr] execution...\n");
    struct CPU_State* cpu = cpu_create();
    assert(cpu != NULL);
    cpu_reset(cpu);

    // Pre-seed a data byte at 0x0500
    mem_write(cpu, 0x0500, 0x77);

    // MOV R3, [0x0500]   (0x60 | reg=3, addr_lo=0x00, addr_hi=0x05)
    byte prog1[] = { (byte)(0x60 | 0x03), 0x00, 0x05 };
    cpu->pc = 0x0200;
    load_bytes(cpu, 0x0200, prog1, 3);
    step_once(cpu);
    assert(cpu->regs[3] == 0x77);
    assert(cpu->pc == 0x0203);

    printf("[2/2] Testing MOV [addr],reg execution...\n");

    // R5 = 0x99, then MOV [0x0600], R5  (0x68 | reg=5, addr_lo=0x00, addr_hi=0x06)
    cpu->regs[5] = 0x99;
    byte prog2[] = { (byte)(0x68 | 0x05), 0x00, 0x06 };
    cpu->pc = 0x0210;
    load_bytes(cpu, 0x0210, prog2, 3);
    step_once(cpu);
    assert(mem_read(cpu, 0x0600) == 0x99);
    assert(cpu->pc == 0x0213);

    // Round-trip check: MOV R6,[0x0600] should now read back 0x99
    byte prog3[] = { (byte)(0x60 | 0x06), 0x00, 0x06 };
    cpu->pc = 0x0220;
    load_bytes(cpu, 0x0220, prog3, 3);
    step_once(cpu);
    assert(cpu->regs[6] == 0x99);

    cpu_destroy(cpu);
}

static void test_bitwise_compare_incdec_execute(void) {
    struct CPU_State* cpu = cpu_create();
    assert(cpu != NULL);
    cpu_reset(cpu);

    printf("[1/6] Testing AND/OR/XOR reg execution...\n");

    // acc=0xF0, R0=0x0F -> AND -> 0x00, Z set
    cpu->acc = 0xF0;
    cpu->regs[0] = 0x0F;
    byte prog1[] = { (byte)(0x90 | 0x00) }; // AND acc, R0
    cpu->pc = 0x0200;
    load_bytes(cpu, 0x0200, prog1, 1);
    step_once(cpu);
    assert(cpu->acc == 0x00);
    assert(CHECK_FLAG(cpu->flags, FLAG_Z));

    // acc=0xF0, R1=0x0F -> OR -> 0xFF
    cpu->acc = 0xF0;
    cpu->regs[1] = 0x0F;
    byte prog2[] = { (byte)(0x98 | 0x01) }; // OR acc, R1
    cpu->pc = 0x0210;
    load_bytes(cpu, 0x0210, prog2, 1);
    step_once(cpu);
    assert(cpu->acc == 0xFF);

    // acc=0xFF, R2=0xFF -> XOR -> 0x00, Z set
    cpu->acc = 0xFF;
    cpu->regs[2] = 0xFF;
    byte prog3[] = { (byte)(0xA0 | 0x02) }; // XOR acc, R2
    cpu->pc = 0x0220;
    load_bytes(cpu, 0x0220, prog3, 1);
    step_once(cpu);
    assert(cpu->acc == 0x00);
    assert(CHECK_FLAG(cpu->flags, FLAG_Z));

    printf("[2/6] Testing CMP reg non-destructive behavior...\n");

    // acc=0x50, R3=0x50 -> CMP -> flags show equal (Z set), acc UNCHANGED
    cpu->acc = 0x50;
    cpu->regs[3] = 0x50;
    byte prog4[] = { (byte)(0xA8 | 0x03) }; // CMP acc, R3
    cpu->pc = 0x0230;
    load_bytes(cpu, 0x0230, prog4, 1);
    step_once(cpu);
    assert(cpu->acc == 0x50); // acc not overwritten
    assert(CHECK_FLAG(cpu->flags, FLAG_Z));

    printf("[3/6] Testing CMP imm non-destructive behavior...\n");

    // acc=0x10, imm=0x20 -> CMP -> borrow set, acc UNCHANGED
    cpu->acc = 0x10;
    byte prog5[] = { OP_CMP_IMM, 0x20 };
    cpu->pc = 0x0240;
    load_bytes(cpu, 0x0240, prog5, 2);
    step_once(cpu);
    assert(cpu->acc == 0x10); // acc not overwritten
    assert(CHECK_FLAG(cpu->flags, FLAG_C)); // borrow required

    printf("[4/6] Testing TEST reg non-destructive behavior...\n");

    // acc=0x0F, R4=0xF0 -> TEST -> result 0x00 (Z set), acc UNCHANGED
    cpu->acc = 0x0F;
    cpu->regs[4] = 0xF0;
    byte prog6[] = { (byte)(0xB8 | 0x04) }; // TEST acc, R4
    cpu->pc = 0x0250;
    load_bytes(cpu, 0x0250, prog6, 1);
    step_once(cpu);
    assert(cpu->acc == 0x0F); // acc not overwritten
    assert(CHECK_FLAG(cpu->flags, FLAG_Z));

    printf("[5/6] Testing INC reg execution (incl. rollover)...\n");

    // R5 = 0x05 -> INC -> 0x06
    cpu->regs[5] = 0x05;
    byte prog7[] = { (byte)(0xC0 | 0x05) }; // INC R5
    cpu->pc = 0x0260;
    load_bytes(cpu, 0x0260, prog7, 1);
    step_once(cpu);
    assert(cpu->regs[5] == 0x06);

    // R6 = 0xFF -> INC -> 0x00, Z + C set (rollover)
    cpu->regs[6] = 0xFF;
    byte prog8[] = { (byte)(0xC0 | 0x06) }; // INC R6
    cpu->pc = 0x0270;
    load_bytes(cpu, 0x0270, prog8, 1);
    step_once(cpu);
    assert(cpu->regs[6] == 0x00);
    assert(CHECK_FLAG(cpu->flags, FLAG_Z));
    assert(CHECK_FLAG(cpu->flags, FLAG_C));

    printf("[6/6] Testing DEC reg execution (incl. rollunder)...\n");

    // R7 = 0x01 -> DEC -> 0x00, Z set
    cpu->regs[7] = 0x01;
    byte prog9[] = { (byte)(0xC8 | 0x07) }; // DEC R7
    cpu->pc = 0x0280;
    load_bytes(cpu, 0x0280, prog9, 1);
    step_once(cpu);
    assert(cpu->regs[7] == 0x00);
    assert(CHECK_FLAG(cpu->flags, FLAG_Z));

    // R0 = 0x00 -> DEC -> 0xFF, S + C set (rollunder)
    cpu->regs[0] = 0x00;
    byte prog10[] = { (byte)(0xC8 | 0x00) }; // DEC R0
    cpu->pc = 0x0290;
    load_bytes(cpu, 0x0290, prog10, 1);
    step_once(cpu);
    assert(cpu->regs[0] == 0xFF);
    assert(CHECK_FLAG(cpu->flags, FLAG_S));
    assert(CHECK_FLAG(cpu->flags, FLAG_C));

    cpu_destroy(cpu);
}


int main(void) {

    printf("Group 1 (basic operations):\n");

    test_memory_read_write();
    test_memory_word_operations();
    test_stack_operations();
    test_stack_word_operations();
    test_memory_reset();
    test_state_management();
    test_hard_reset();
    test_multiple_cpu_instances();

    printf("Group 2 (decoder):\n");

    struct CPU_State* cpu = cpu_create();
    assert(cpu != NULL);

    test_all_opcode_bounds(cpu);
    test_immediate_scope_constraints();
    test_category_classifiers();
    test_halted_cpu_guard(cpu);

    cpu_destroy(cpu);

    printf("Group 3 (alu):\n");
    test_add_flags();
    test_sub_flags();
    test_inc_dec_flags();
    test_shift_flags();

    printf("Group 4 (execute):\n");
    test_mov_execute();
    test_add_sub_execute();
    test_jump_execute();
    test_call_ret_execute();
    printf("Group 4a (bit shifting):\n");
    test_shift_execute();
    printf("Group 4b (data mov-ement):\n");
    test_mov_mem_execute();
    printf("Group 4c (bitwise, comparator, counters):\n");
    test_bitwise_compare_incdec_execute();
    return 0;
}
