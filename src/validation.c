#include "../include/types.h"

#include <stdlib.h>
#include <string.h>

/* Layer 0: Implementation Source File */
/* This file contains only the type definitions and constants.
 * No function implementations are present in Layer 0.
 * All functions will be implemented in subsequent layers.
 */

/* Compile-time verification of constants */
_Static_assert(RAM_SIZE == 65536, "RAM_SIZE must be 65536");
_Static_assert(REGISTER_COUNT == 8, "REGISTER_COUNT must be 8");
_Static_assert(STACK_PAGE == 0x0100, "STACK_PAGE must be 0x0100");

/* Compile-time verification of flag bitmasks */
_Static_assert(FLAG_Z == 0x01, "FLAG_Z must be 0x01");
_Static_assert(FLAG_C == 0x02, "FLAG_C must be 0x02");
_Static_assert(FLAG_S == 0x04, "FLAG_S must be 0x04");
_Static_assert(FLAG_O == 0x08, "FLAG_O must be 0x08");

/* Compile-time verification of type sizes */
_Static_assert(sizeof(byte) == 1, "byte must be 8-bit");
_Static_assert(sizeof(address) == 2, "address must be 16-bit");
_Static_assert(sizeof(opcode) == 1, "opcode must be 8-bit");
_Static_assert(sizeof(signed_byte) == 1, "signed_byte must be 8-bit");
_Static_assert(sizeof(pair) == 2, "pair must be 16-bit");

/* Compile-time verification of non-bitfield structure members */
_Static_assert(sizeof(((struct CPU_State*)0)->ram) == sizeof(byte*),
               "ram must be pointer to byte");
_Static_assert(sizeof(((struct CPU_State*)0)->pc) == 2,
               "pc must be 16-bit");
_Static_assert(sizeof(((struct CPU_State*)0)->sp) == 1,
               "sp must be 8-bit");
_Static_assert(sizeof(((struct CPU_State*)0)->regs) == REGISTER_COUNT,
               "regs array must match REGISTER_COUNT");
_Static_assert(sizeof(((struct CPU_State*)0)->acc) == 1,
               "acc must be 8-bit");
_Static_assert(sizeof(((struct CPU_State*)0)->flags) == 1,
               "flags must be 8-bit");
_Static_assert(sizeof(((struct CPU_State*)0)->cycles) == 8,
               "cycles must be 64-bit");

/* Compile-time verification of non-bitfield structure offsets */
_Static_assert(offsetof(struct CPU_State, ram) == 0,
               "ram must be first member");
_Static_assert(offsetof(struct CPU_State, pc) > offsetof(struct CPU_State, ram),
               "pc must follow ram");
_Static_assert(offsetof(struct CPU_State, sp) > offsetof(struct CPU_State, pc),
               "sp must follow pc");
_Static_assert(offsetof(struct CPU_State, regs) > offsetof(struct CPU_State, sp),
               "regs must follow sp");
_Static_assert(offsetof(struct CPU_State, acc) > offsetof(struct CPU_State, regs),
               "acc must follow regs");
_Static_assert(offsetof(struct CPU_State, flags) > offsetof(struct CPU_State, acc),
               "flags must follow acc");

/* Verify that bitfields exist by checking they are after flags */
/* Note: We cannot use offsetof on bitfields, but we can check that
 * cycles is after flags, which implies bitfields are between them */
_Static_assert(offsetof(struct CPU_State, cycles) > offsetof(struct CPU_State, flags),
               "cycles must follow flags, implying bitfields are present");

/* Verify structure has all expected members by checking minimum size */
_Static_assert(sizeof(struct CPU_State) >= (sizeof(byte*) + 2 + 1 + 8 + 1 + 1 + 2 + 8),
               "CPU_State must be at least sum of member sizes");

/* The CPU_State structure must be non-empty */
_Static_assert(sizeof(struct CPU_State) > 0, "CPU_State must be non-empty");

/* Layer 0 validation - ensure no placeholder functions exist */
/* All functions MUST be implemented in Layer 1 through Layer 4 */
/* This file contains NO function definitions, only type and constant definitions */

/* The following assertions ensure no accidental function stubs are left in Layer 0 */
#ifdef __GNUC__
#pragma GCC poison cpu_create cpu_destroy cpu_reset cpu_hard_reset
#pragma GCC poison cpu_step cpu_run cpu_trigger_interrupt
#pragma GCC poison cpu_enable_interrupts cpu_disable_interrupts
#pragma GCC poison cpu_dump_registers cpu_dump_flags cpu_dump_memory
#pragma GCC poison cpu_set_breakpoint cpu_get_cycle_count
#pragma GCC poison cpu_load_binary cpu_load_hex cpu_save_state cpu_load_state
#pragma GCC poison mem_read mem_write mem_read_word mem_write_word mem_reset
#pragma GCC poison push pop push_word pop_word
#pragma GCC poison decode_instruction is_alu_op is_branch_op is_memory_op is_stack_op is_immediate_op
#pragma GCC poison alu_execute execute_instruction
#pragma GCC poison handle_add_imm handle_add_reg handle_sub_imm handle_sub_reg
#pragma GCC poison handle_and_reg handle_or_reg handle_xor_reg
#pragma GCC poison handle_shl handle_shr handle_rol
#pragma GCC poison handle_jmp handle_jz handle_jnz handle_jc handle_jnc handle_js handle_jns
#pragma GCC poison handle_mov_imm handle_mov_reg handle_mov_mem_to_reg handle_mov_reg_to_mem
#pragma GCC poison handle_call handle_ret handle_iret handle_int
#pragma GCC poison handle_cmp_reg handle_cmp_imm handle_test_reg
#pragma GCC poison handle_inc_reg handle_dec_reg
#pragma GCC poison create_expected_state calculate_mathematical_result compare_cpu_states
#pragma GCC poison run_test_sequence report_register_mismatch report_flag_mismatch
#pragma GCC poison report_pc_mismatch report_memory_mismatch
#endif

/* End of Layer 0 implementation */
