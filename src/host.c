#include "../include/host.h"
#include "../include/decoder.h"
#include "../include/execute.h"
#include "../include/memory.h"
#include "../include/state.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Layer 4: Host Interface Implementation */

/* Initialization & Execution */

int cpu_step(struct CPU_State* cpu) {
    if (cpu->halted) {
        return 1;
    }

    opcode op = decode_instruction(cpu);
    if (op == OP_INVALID) {
        cpu->halted = 1;
        return 1;
    }

    execute_instruction(cpu, op);
    cpu->cycles++;

    return 0;
}

void cpu_run(struct CPU_State* cpu, unsigned long long max_cycles) {
    unsigned long long cycles_run = 0;

    while (!cpu->halted && cycles_run < max_cycles) {
        if (cpu_step(cpu)) {
            break;
        }
        cycles_run++;
    }
}

/* Interrupts */

void cpu_trigger_interrupt(struct CPU_State* cpu, byte vector) {
    if (!cpu->interrupt_enabled) {
        return;
    }

    address vector_addr = (address)(vector * 2);
    pair handler_addr = mem_read_word(cpu, vector_addr);

    handle_int(cpu, vector);
    cpu->pc = (handler_addr.low | (handler_addr.high << 8));
}

void cpu_enable_interrupts(struct CPU_State* cpu) {
    cpu->interrupt_enabled = 1;
}

void cpu_disable_interrupts(struct CPU_State* cpu) {
    cpu->interrupt_enabled = 0;
}

/* Debugging */

void cpu_dump_registers(struct CPU_State* cpu) {
    printf("Registers:\n");
    for (int i = 0; i < REGISTER_COUNT; i++) {
        printf("  R%d: 0x%02X (%3d)\n", i, cpu->regs[i], cpu->regs[i]);
    }
    printf("  ACC: 0x%02X (%3d)\n", cpu->acc, cpu->acc);
    printf("  PC:  0x%04X\n", cpu->pc);
    printf("  SP:  0x%02X (0x%04X)\n", cpu->sp, (address)(STACK_PAGE + cpu->sp));
}

void cpu_dump_flags(struct CPU_State* cpu) {
    printf("Flags:\n");
    printf("  Z: %d\n", (cpu->flags & FLAG_Z) ? 1 : 0);
    printf("  C: %d\n", (cpu->flags & FLAG_C) ? 1 : 0);
    printf("  S: %d\n", (cpu->flags & FLAG_S) ? 1 : 0);
    printf("  O: %d\n", (cpu->flags & FLAG_O) ? 1 : 0);
    printf("  Halted: %d\n", cpu->halted);
    printf("  Interrupts: %d\n", cpu->interrupt_enabled);
    printf("  Cycles: %llu\n", cpu->cycles);
}

void cpu_dump_memory(struct CPU_State* cpu, address start, address end) {
    if (start > end) {
        return;
    }

    printf("Memory 0x%04X - 0x%04X:\n", start, end);

    address addr = start;
    while (addr <= end) {
        printf("  0x%04X: ", addr);
        for (int i = 0; i < 16 && addr <= end; i++) {
            printf("%02X ", mem_read(cpu, addr));
            addr++;
        }
        printf("\n");
    }
}

void cpu_set_breakpoint(struct CPU_State* cpu, address addr, unsigned int enabled) {
    (void)cpu;
    (void)addr;
    (void)enabled;
}

unsigned long long cpu_get_cycle_count(struct CPU_State* cpu) {
    return cpu->cycles;
}

/* File I/O */

void cpu_load_binary(struct CPU_State* cpu, const char* filename, address start_addr) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(file);
        return;
    }

    size_t max_bytes = (size_t)(RAM_SIZE - start_addr);
    if ((size_t)file_size > max_bytes) {
        fclose(file);
        return;
    }

    address addr = start_addr;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        mem_write(cpu, addr, (uint8_t)ch);
        addr++;
    }

    fclose(file);
}

void cpu_load_hex(struct CPU_State* cpu, const char* filename, address start_addr) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return;
    }

    char line[256];
    address addr = start_addr;

    while (fgets(line, sizeof(line), file)) {
        char* endptr;
        unsigned long val = strtoul(line, &endptr, 16);
        if (endptr > line) {
            mem_write(cpu, addr, (byte)val);
            addr++;
        }
    }

    fclose(file);
}

void cpu_save_state(struct CPU_State* cpu, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        return;
    }

    byte halted_byte = cpu->halted ? 1 : 0;
    byte interrupt_enabled_byte = cpu->interrupt_enabled ? 1 : 0;

    fwrite(&cpu->pc, sizeof(cpu->pc), 1, file);
    fwrite(&cpu->sp, sizeof(cpu->sp), 1, file);
    fwrite(cpu->regs, sizeof(cpu->regs), 1, file);
    fwrite(&cpu->acc, sizeof(cpu->acc), 1, file);
    fwrite(&cpu->flags, sizeof(cpu->flags), 1, file);
    fwrite(&halted_byte, sizeof(halted_byte), 1, file);
    fwrite(&interrupt_enabled_byte, sizeof(interrupt_enabled_byte), 1, file);
    fwrite(&cpu->cycles, sizeof(cpu->cycles), 1, file);
    fwrite(cpu->ram, RAM_SIZE, 1, file);

    fclose(file);
}

int cpu_load_state(struct CPU_State* cpu, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return 0;
    }

    byte halted_byte;
    byte interrupt_enabled_byte;
    size_t read_count;

    read_count = fread(&cpu->pc, sizeof(cpu->pc), 1, file);
    if (read_count != 1) { fclose(file); return 0; }

    read_count = fread(&cpu->sp, sizeof(cpu->sp), 1, file);
    if (read_count != 1) { fclose(file); return 0; }

    read_count = fread(cpu->regs, sizeof(cpu->regs), 1, file);
    if (read_count != 1) { fclose(file); return 0; }

    read_count = fread(&cpu->acc, sizeof(cpu->acc), 1, file);
    if (read_count != 1) { fclose(file); return 0; }

    read_count = fread(&cpu->flags, sizeof(cpu->flags), 1, file);
    if (read_count != 1) { fclose(file); return 0; }

    read_count = fread(&halted_byte, sizeof(halted_byte), 1, file);
    if (read_count != 1) { fclose(file); return 0; }
    cpu->halted = halted_byte ? 1 : 0;

    read_count = fread(&interrupt_enabled_byte, sizeof(interrupt_enabled_byte), 1, file);
    if (read_count != 1) { fclose(file); return 0; }
    cpu->interrupt_enabled = interrupt_enabled_byte ? 1 : 0;

    read_count = fread(&cpu->cycles, sizeof(cpu->cycles), 1, file);
    if (read_count != 1) { fclose(file); return 0; }

    read_count = fread(cpu->ram, RAM_SIZE, 1, file);
    if (read_count != 1) { fclose(file); return 0; }

    fclose(file);
    return 1;
}
