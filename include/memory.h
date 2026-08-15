#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* Layer 1: Memory Management Functions */

/* Memory Access Functions */
byte mem_read(struct CPU_State* cpu, address addr);
void mem_write(struct CPU_State* cpu, address addr, byte value);
pair mem_read_word(struct CPU_State* cpu, address addr);
void mem_write_word(struct CPU_State* cpu, address addr, pair value);
void mem_reset(struct CPU_State* cpu);

/* Stack Operations (Bounded Page 0x0100) */
void push(struct CPU_State* cpu, byte value);
byte pop(struct CPU_State* cpu);
void push_word(struct CPU_State* cpu, pair value);
pair pop_word(struct CPU_State* cpu);

#endif /* MEMORY_H */
