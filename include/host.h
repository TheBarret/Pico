#ifndef HOST_H
#define HOST_H

#include "types.h"
#include "state.h"

/* Layer 4: Host Interface Functions */

/* Initialization & Execution */
int cpu_step(struct CPU_State* cpu);
void cpu_run(struct CPU_State* cpu, unsigned long long max_cycles);

/* Interrupts */
void cpu_trigger_interrupt(struct CPU_State* cpu, byte vector);
void cpu_enable_interrupts(struct CPU_State* cpu);
void cpu_disable_interrupts(struct CPU_State* cpu);

/* Debugging */
void cpu_dump_registers(struct CPU_State* cpu);
void cpu_dump_flags(struct CPU_State* cpu);
void cpu_dump_memory(struct CPU_State* cpu, address start, address end);
void cpu_set_breakpoint(struct CPU_State* cpu, address addr, unsigned int enabled);
unsigned long long cpu_get_cycle_count(struct CPU_State* cpu);

/* File IO */
void cpu_load_binary(struct CPU_State* cpu, const char* filename, address start_addr);
void cpu_load_hex(struct CPU_State* cpu, const char* filename, address start_addr);
void cpu_save_state(struct CPU_State* cpu, const char* filename);
int cpu_load_state(struct CPU_State* cpu, const char* filename);

#endif /* HOST_H */
