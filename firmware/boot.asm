; ==============================================================================
; Pico8 Firmware Template (Peudo/Mockup Model)
; Setup stdio bus discovery & string Printer
;
; Memory Map:
;   0x0000 - Entry Point (_start)
;   0x0100 - String Data Buffer
;   0xFA00 - Top of Kernel Stack (grows down toward 0x0100)
;   0xFE00 - 0xFE05 MMIO Bus Space
; ==============================================================================

.org 0x0000

_start:
    ; 1. Initialize Stack Pointer (Top of Stack RAM space)
    MOV SP, 0xFA00

    ; 2. Discover Stdio Device (Device Type 0x01) on PICO-BUS
    ;    Scans IDs 1 to 255 until it finds type 0x01
    MOV reg1, 1              ; Start searching at Bus ID 1

_bus_scan_loop:
    CMP reg1, 255            ; Reached bus scan ceiling?
    JZ _no_device_found      ; Fallback if no stdio device exists

    MOV [0xFE00], reg1       ; Write current ID to BUS_SELECT (0xFE00)
    LDR reg2, [0xFE02]       ; Read BUS_STATUS (0xFE02)
    CMP reg2, 0              ; Check if BUS_ABSENT (0)
    JZ _next_bus_id

    LDR reg2, [0xFE01]       ; Read BUS_TYPE (0xFE01)
    CMP reg2, 0x01           ; Is it a STDIO device (Type 0x01)?
    JZ _device_found         ; Match! Keep ID in reg1 and proceed

_next_bus_id:
    INC reg1                 ; Increment device ID
    JMP _bus_scan_loop

_device_found:
    ; reg1 currently holds the active stdio device ID and remains selected
    ; 3. Print the boot message string
    MOV reg2, _msg_hello     ; Load pointer to string message into reg2

_print_loop:
    LDR acc, [reg2]          ; Read character at pointer [reg2]
    CMP acc, 0               ; Check for null-terminator (0x00)
    JZ _done                 ; End of string reached

    MOV [0xFE04], acc        ; Write character to BUS_DATA (0xFE04)
    INC reg2                 ; Move pointer to next character
    JMP _print_loop

_no_device_found:
    ; Halt execution if hardware discovery fails
    HLT

_done:
    ; 4. Send flush command to stdio device and halt
    MOV acc, 0x01            ; STDIO_CMD_FLUSH (0x01)
    MOV [0xFE03], acc        ; Write to BUS_COMMAND (0x03)
    HLT

; ==============================================================================
; Data Section
; ==============================================================================
.org 0x0100
_msg_hello:
    .string "Hello, from Pico8!\n"
    .db 0x00                 ; Null terminator
