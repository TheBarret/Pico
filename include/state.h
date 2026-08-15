#ifndef STATE_H
#define STATE_H

#include "types.h"

/* Layer 1: CPU State Management Functions */

/* Initialization Functions */
struct CPU_State* cpu_create(void);
void cpu_destroy(struct CPU_State* cpu);
void cpu_reset(struct CPU_State* cpu);
void cpu_hard_reset(struct CPU_State* cpu);

#endif /* STATE_H */
