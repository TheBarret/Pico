#include "../include/memory.h"
#include "../include/types.h"

#include <string.h>

/* Layer 1: Memory Management Implementation */

/* Memory Access Functions */

byte mem_read(struct CPU_State* cpu, address addr) {
    return cpu->ram[addr];
}

void mem_write(struct CPU_State* cpu, address addr, byte value) {
    cpu->ram[addr] = value;
}

pair mem_read_word(struct CPU_State* cpu, address addr) {
    pair result;
    result.low = mem_read(cpu, addr);
    result.high = mem_read(cpu, addr + 1);
    return result;
}

void mem_write_word(struct CPU_State* cpu, address addr, pair value) {
    mem_write(cpu, addr, value.low);
    mem_write(cpu, addr + 1, value.high);
}

void mem_reset(struct CPU_State* cpu) {
    memset(cpu->ram, 0, RAM_SIZE);
}

/* Stack Operations (Bounded Page 0x0100) */

void push(struct CPU_State* cpu, byte value) {
    address stack_addr = STACK_PAGE + cpu->sp;
    mem_write(cpu, stack_addr, value);
    cpu->sp--;
}

byte pop(struct CPU_State* cpu) {
    cpu->sp++;
    address stack_addr = STACK_PAGE + cpu->sp;
    return mem_read(cpu, stack_addr);
}

void push_word(struct CPU_State* cpu, pair value) {
    push(cpu, value.high);
    push(cpu, value.low);
}

pair pop_word(struct CPU_State* cpu) {
    pair result;
    result.low = pop(cpu);
    result.high = pop(cpu);
    return result;
}
