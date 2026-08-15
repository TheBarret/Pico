#include "../include/state.h"
#include "../include/memory.h"

#include <stdlib.h>
#include <string.h>

/* Layer 1: CPU State Management Implementation */

struct CPU_State* cpu_create(void) {
    struct CPU_State* cpu = malloc(sizeof(struct CPU_State));
    if (cpu == NULL) {
        return NULL;
    }

    cpu->ram = calloc(RAM_SIZE, sizeof(byte));
    if (cpu->ram == NULL) {
        free(cpu);
        return NULL;
    }

    cpu_reset(cpu);
    return cpu;
}

void cpu_destroy(struct CPU_State* cpu) {
    if (cpu != NULL) {
        if (cpu->ram != NULL) {
            free(cpu->ram);
        }
        free(cpu);
    }
}

void cpu_reset(struct CPU_State* cpu) {
    cpu->pc = 0x0000;
    cpu->sp = 0xFF;
    cpu->flags = 0x00;
    cpu->halted = 0;
    cpu->interrupt_enabled = 0;
    cpu->cycles = 0;

    memset(cpu->regs, 0, REGISTER_COUNT);
    cpu->acc = 0x00;
}

void cpu_hard_reset(struct CPU_State* cpu) {
    mem_reset(cpu);
    cpu_reset(cpu);
}
