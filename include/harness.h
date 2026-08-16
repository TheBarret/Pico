#ifndef HARNESS_H
#define HARNESS_H

#include "types.h"
#include "state.h"

/* Layer 4: Test Harness Functions */

struct CPU_State* create_expected_state(void);
void calculate_mathematical_result(struct CPU_State* expected, opcode op, byte operand1, byte operand2);
int compare_cpu_states(struct CPU_State* actual, struct CPU_State* expected);
void run_test_sequence(byte* program, size_t program_size, unsigned int steps);
void report_register_mismatch(int reg_index, byte actual, byte expected);
void report_flag_mismatch(char flag_name, unsigned int actual, unsigned int expected);
void report_pc_mismatch(address actual, address expected);
void report_memory_mismatch(address addr, byte actual, byte expected);

#endif /* HARNESS_H */
