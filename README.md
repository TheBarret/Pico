# PICO (8bit CPU, rev 0.3)

*Tiny 8-bit CPU emulator written in C*

**Compile (make) command-line:**  
```bash
 > make all test
```

**Create Instance:**  
```c
 #include <stdio.h>
 #include <assert.h>
 
 #include "../include/state.h"
 #include "../include/memory.h"
 #include "../include/types.h"
 #include "../include/decoder.h"
 #include "../include/alu.h"
 #include "../include/execute.h"
 #include "../include/host.h"
 #include "../include/bus.h"
 #include "../devices/stddev.h"
 
 
 int main(void) {
         printf("Loading Pico8...\n");
         struct CPU_State* cpu = cpu_create();
         cpu_reset(cpu);
 
         printf("Loading Pico Bus...\n");
         bus_init(cpu);
 
         printf("Registering stdio...\n");
         bus_register_device(1, stdio_device_get());
 
         // todo: firmware
 
         cpu_destroy(cpu);
         printf("Finished\n");
         return 0;
 }
```
 
### Layer 0: The Core Data Types & Constants

**Header Files:**
- `include/types.h`: All type, constant, CPU state structure definitions

**Source Files:**
- `src/validation.c`: Compile-time validation and poison guards

### Type Definitions

| Type | Description | Size |
|------|-------------|------|
| byte | Primary data width | 8-bit |
| address | Memory address | 16-bit |
| opcode | Instruction opcode | 8-bit |
| signed_byte | Signed byte for comparisons | 8-bit |
| pair | 16-bit register pair (low/high) | 16-bit |

### Constants

| Constant | Value | Description |
|----------|-------|-------------|
| RAM_SIZE | 65536 | Maximum addressable memory |
| REGISTER_COUNT | 8 | General purpose registers R0-R7 |
| STACK_PAGE | 0x0100 | Fixed 256-byte stack page |

### Flag Bitmasks

| Flag | Bit | Description |
|------|-----|-------------|
| FLAG_Z | 0 | Zero flag |
| FLAG_C | 1 | Carry flag |
| FLAG_S | 2 | Sign flag |
| FLAG_O | 3 | Overflow flag |

### CPU State Structure

```c
struct CPU_State {
    byte* ram;                // 64KB dynamic memory
    address pc;               // Program counter
    uint8_t sp;               // Stack pointer (page 0x0100)
    byte regs[8];             // R0-R7 registers
    byte acc;                 // Accumulator
    byte flags;               // Bitmasked FLAG_Z|FLAG_C|FLAG_S|FLAG_O
    unsigned int halted : 1;          // Execution halt
    unsigned int interrupt_enabled : 1; // Interrupts enabled
    unsigned long long cycles;        // Cycle counter
};
```

### Layer 1: Memory Management & CPU State

**Header Files:**
- `include/memory.h`: Memory access and stack operations
- `include/state.h`: CPU state management functions

**Source Files:**
- `src/memory.c`: Memory and stack implementation
- `src/state.c`: CPU initialization and management

**Test Files:**
- `src/test.c`: Testing & Debugging

**Context Structure Definition:**

```c
struct CPU_State {
    // Memory
    byte* ram;                    // Dynamic allocation for 64KB
    address pc;                   // Program counter
    uint8_t sp;                   // 8-bit Stack pointer (wraps within 0x0100-0x01FF page)

    // Registers
    byte regs[REGISTER_COUNT];    // R0 through R7
    byte acc;                     // Accumulator

    // Flags & Control
    byte flags;                   // Bitmasked flags (FLAG_Z, FLAG_C, FLAG_S, FLAG_O)
    unsigned int halted : 1;      // Execution halt flag
    unsigned int interrupt_enabled : 1;
    unsigned long long cycles;    // Cycle counter
};
```

**Memory Access Functions:**

* `byte mem_read(struct CPU_State* cpu, address addr)` - Read byte from RAM
* `void mem_write(struct CPU_State* cpu, address addr, byte value)` - Write byte to RAM
* `pair mem_read_word(struct CPU_State* cpu, address addr)` - Read 16-bit word explicitly in little-endian (`low = mem[addr]`, `high = mem[addr + 1]`)
* `void mem_write_word(struct CPU_State* cpu, address addr, pair value)` - Write 16-bit word explicitly in little-endian
* `void mem_reset(struct CPU_State* cpu)` - Zero out entire RAM

**Stack Operations (Bounded Page 0x0100):**

* `void push(struct CPU_State* cpu, byte value)` - Write to `STACK_PAGE + sp`, then `sp--`
* `byte pop(struct CPU_State* cpu)` - `sp++`, then read from `STACK_PAGE + sp`
* `void push_word(struct CPU_State* cpu, pair value)` - Push high byte then low byte
* `pair pop_word(struct CPU_State* cpu)` - Pop low byte then high byte

### Layer 2: The Decoder & Compact Opcode Mapping

**Header Files:**
- `include/decoder.h`: Opcode definitions
- `include/execute.h`: Dispatcher definitions

**Source Files:**
- `src/decoder.c`: Instruction decoder logic
- `src/execute.c`: Dispatcher logic

**Bit-Pattern Opcode Map:**
```
  version   : rev 0.3, MOV reg,reg widened to 64 slots; all following blocks renumbered
  
  0x00      : NOP
  0x01      : HALT
  0x02      : RET
  0x03      : IRET
  0x04      : SHL acc (shift left by 1)
  0x05      : SHR acc (shift right by 1)
  0x06      : ROL acc (rotate left by 1)
  0x07-0x0F : Reserved

  0x10-0x17 : MOV reg[bbb], imm (0001 0bbb)
  0x18-0x1F : Reserved

  0x20-0x5F : MOV reg[bbb], reg[aaa]
              64 opcodes; opcode = 0x20 + (bbb << 3 | aaa), bbb/aaa each 0-7

  0x60-0x67 : MOV reg[bbb], [addr]   (0110 0bbb)
  0x68-0x6F : MOV [addr], reg[bbb]   (0110 1bbb)

  0x70-0x77 : ADD acc, reg[bbb] (0111 0bbb)
  0x78      : ADD acc, imm (single opcode)
  0x79-0x7F : Reserved

  0x80-0x87 : SUB acc, reg[bbb] (1000 0bbb)
  0x88      : SUB acc, imm (single opcode)
  0x89-0x8F : Reserved

  0x90-0x97 : AND acc, reg[bbb]
  0x98-0x9F : OR  acc, reg[bbb]

  0xA0-0xA7 : XOR acc, reg[bbb]
  0xA8-0xAF : CMP acc, reg[bbb]

  0xB0      : CMP acc, imm (single opcode)
  0xB1-0xB7 : Reserved
  0xB8-0xBF : TEST acc, reg[bbb]

  0xC0-0xC7 : INC reg[bbb]
  0xC8-0xCF : DEC reg[bbb]

  0xD0 : JMP addr
  0xD1 : JZ  addr
  0xD2 : JNZ addr
  0xD3 : JC  addr
  0xD4 : JNC addr
  0xD5 : JS  addr
  0xD6 : JNS addr
  0xD7 : CALL addr
  0xD8 : INT vector
  0xD9-0xFF : Reserved
```

**Immediate-operand scope (explicit constraint, not an omission):** 
Immediate operands are supported only for `MOV`, `ADD`, `SUB`, and `CMP`. `AND`, `OR`, `XOR`, `TEST` are register-only. 
All unassigned opcodes within a block (e.g. `0x79-0x7F`, `0xB1-0xB7`),
and the trailing `0xD9-0xFF` range are illegal opcodes: 
`decode_instruction` must treat them as invalid, not as aliases of a defined instruction.

**Decode Function:**

* `opcode decode_instruction(struct CPU_State* cpu)` - Read opcode at PC, increment PC, return opcode

**Instruction Category Detection:**

* `int is_alu_op(opcode op)`
* `int is_branch_op(opcode op)`
* `int is_memory_op(opcode op)`
* `int is_stack_op(opcode op)`
* `int is_immediate_op(opcode op)`

### Layer 3: Instruction Execution & Bitwise Flag Calculations

**Header Files:**
- `include/alu.h`: ALU interface definition

**Source Files:**
- `src/alu.c`: ALU execution logic

**ALU Core Function:**

* `byte alu_execute(byte a, byte b, opcode operation, byte* flags)` - Computes 8-bit result using 16-bit intermediate integer variables and applies masks (`FLAG_Z`, `FLAG_C`, `FLAG_S`, `FLAG_O`) to the `flags` byte.

**Flag Calculation Rules:**

* `FLAG_Z`: `((result & 0xFF) == 0)`
* `FLAG_C`: For ADD `(temp_16 > 0xFF)`, for SUB `(a < b)`
* `FLAG_S`: `((result & 0x80) != 0)` calculated after masking result to 8 bits
* `FLAG_O` (ADD): `(((a ^ result) & (b ^ result) & 0x80) != 0)`
* `FLAG_O` (SUB): `(((a ^ b) & (a ^ result) & 0x80) != 0)`

**Interrupt Entry/Exit State (explicit):**

* `handle_int`: push `pc` (word, via `push_word`), then push `flags` (byte, via `push`), then clear `interrupt_enabled`.
* `handle_iret`: pop `flags` (byte, via `pop`), then pop `pc` (word, via `pop_word`), then set `interrupt_enabled`.

**Instruction Execution Handlers:**

* `void execute_instruction(struct CPU_State* cpu, opcode op)`
* `void handle_add_imm(struct CPU_State* cpu, byte immediate)`
* `void handle_add_reg(struct CPU_State* cpu, byte reg_index)`
* `void handle_sub_imm(struct CPU_State* cpu, byte immediate)`
* `void handle_sub_reg(struct CPU_State* cpu, byte reg_index)`
* `void handle_and_reg(struct CPU_State* cpu, byte reg_index)`
* `void handle_or_reg(struct CPU_State* cpu, byte reg_index)`
* `void handle_xor_reg(struct CPU_State* cpu, byte reg_index)`
* `void handle_shl(struct CPU_State* cpu)`
* `void handle_shr(struct CPU_State* cpu)`
* `void handle_rol(struct CPU_State* cpu)`
* `void handle_jmp(struct CPU_State* cpu, address target)`
* `void handle_jz(struct CPU_State* cpu, address target)`
* `void handle_jnz(struct CPU_State* cpu, address target)`
* `void handle_jc(struct CPU_State* cpu, address target)`
* `void handle_jnc(struct CPU_State* cpu, address target)`
* `void handle_js(struct CPU_State* cpu, address target)`
* `void handle_jns(struct CPU_State* cpu, address target)`
* `void handle_mov_imm(struct CPU_State* cpu, byte reg_index, byte value)`
* `void handle_mov_reg(struct CPU_State* cpu, byte dest_reg, byte src_reg)`
* `void handle_mov_mem_to_reg(struct CPU_State* cpu, byte reg_index, address addr)`
* `void handle_mov_reg_to_mem(struct CPU_State* cpu, address addr, byte reg_index)`
* `void handle_call(struct CPU_State* cpu, address target)`
* `void handle_ret(struct CPU_State* cpu)`
* `void handle_iret(struct CPU_State* cpu)`
* `void handle_int(struct CPU_State* cpu, byte vector)`
* `void handle_cmp_reg(struct CPU_State* cpu, byte reg_index)`
* `void handle_cmp_imm(struct CPU_State* cpu, byte immediate)`
* `void handle_test_reg(struct CPU_State* cpu, byte reg_index)`
* `void handle_inc_reg(struct CPU_State* cpu, byte reg_index)`
* `void handle_dec_reg(struct CPU_State* cpu, byte reg_index)`

### Layer 4: Host Interface & Testing Framework

**Header Files:**
- `include/host.h`: Host Interface Functions
- `include/harness.h`: Test Harness Functions 

**Source Files:**
- `src/host.c`: Host Interface Implementation
- `src/harness.c`: Test Harness Implementation

**Initialization & Execution:**

* `struct CPU_State* cpu_create()`
* `void cpu_destroy(struct CPU_State* cpu)`
* `void cpu_reset(struct CPU_State* cpu)`
* `void cpu_hard_reset(struct CPU_State* cpu)`
* `int cpu_step(struct CPU_State* cpu)`
* `void cpu_run(struct CPU_State* cpu, unsigned long long max_cycles)`

**Interrupts, Debugging & File I/O:**

* `void cpu_trigger_interrupt(struct CPU_State* cpu, byte vector)`
* `void cpu_enable_interrupts(struct CPU_State* cpu)`
* `void cpu_disable_interrupts(struct CPU_State* cpu)`
* `void cpu_dump_registers(struct CPU_State* cpu)`
* `void cpu_dump_flags(struct CPU_State* cpu)`
* `void cpu_dump_memory(struct CPU_State* cpu, address start, address end)`
* `void cpu_set_breakpoint(struct CPU_State* cpu, address addr, unsigned int enabled)`
* `unsigned long long cpu_get_cycle_count(struct CPU_State* cpu)`
* `void cpu_load_binary(struct CPU_State* cpu, const char* filename, address start_addr)`
* `void cpu_load_hex(struct CPU_State* cpu, const char* filename, address start_addr)`
* `void cpu_save_state(struct CPU_State* cpu, const char* filename)`
* `int cpu_load_state(struct CPU_State* cpu, const char* filename)`

**Test Harness Functions:**

* `struct CPU_State* create_expected_state()`
* `void calculate_mathematical_result(struct CPU_State* expected, opcode op, byte operand1, byte operand2)`
* `int compare_cpu_states(struct CPU_State* actual, struct CPU_State* expected)`
* `void run_test_sequence(byte* program, size_t program_size, unsigned int steps)`
* `void report_register_mismatch(int reg_index, byte actual, byte expected)`
* `void report_flag_mismatch(char flag_name, unsigned int actual, unsigned int expected)`
* `void report_pc_mismatch(address actual, address expected)`
* `void report_memory_mismatch(address addr, byte actual, byte expected)`

### Pico Bus

See [Bus Readme](PICOBUS.md)

**Header Files:**
- `include/bus.h`: Bus controller definitions
- `devices/stddev.h`: Stdio wrapper device definitions

**Source Files:**
- `src/bus.c`: Bus controller logic
- `devices/stddev.c`: Stdio wrapper control logic 

**Stdio commands recognized by BUS_COMMAND (0xFE03):**  
```
 #define STDIO_CMD_FLUSH 0x01  /* Force flush host stdout buffer */
 #define STDIO_CMD_CLEAR 0x02  /* Clear host terminal screen    */
```

### Re-ordered Sequential Implementation Order

1. **Phase 1: Foundation & RAM**
* Define type aliases, constants, and flag bitmasks.
* Implement `struct CPU_State` and bounded page memory/stack functions.
* Create `cpu_create()` and `cpu_destroy()`.
* Test memory read/write and stack operations in isolation.

2. **Phase 2: Minimal ALU & Test Harness**
* Implement `alu_execute()` with mask-based flag calculations, using the separate ADD/SUB overflow formulas.
* Set up test comparison functions.
* Run unit tests for Z, C, S, O flags on ADD/SUB boundary conditions before opcode integration.

3. **Phase 3: Fetch-Decode-Execute Loop & Basic Instructions**
* Implement `pc`, `decode_instruction()`, and basic MOV operations (`MOV imm`, `MOV reg, reg` across the full 64-opcode range).
* Implement ADD and SUB handlers with register/immediate operands.
* Test simple sequential instruction flows.

4. **Phase 4: Stack & Flow Control**
* Implement `push` / `pop` memory helpers using `STACK_PAGE`.
* Implement all jump instructions (`JMP`, `JZ`, `JNZ`, `JC`, `JNC`, `JS`, `JNS`).
* Implement `CALL` and `RET`.
* Test loop constructs and subroutines.

5. **Phase 5: Extended Instructions**
* Implement bitwise single-byte operations (`SHL`, `SHR`, `ROL`).
* Implement memory load/store operations (`MOV reg, [addr]`, `MOV [addr], reg`).
* Implement `CMP`, `TEST`, `INC`, and `DEC`.

6. **Phase 6: Host Interface & Loaders**
* Implement binary and Intel HEX file loaders.
* Implement debugging functions and state serialization (`cpu_save_state` / `cpu_load_state`).

7. **Phase 7: Interrupts & Automated Testing**
* Implement software interrupts (`INT`, `IRET`) per the defined push/pop-order and `interrupt_enabled` handling above.
* Integrate random instruction test generation script and verify 100% instruction coverage, including confirming illegal/reserved opcodes are rejected by `decode_instruction`.
