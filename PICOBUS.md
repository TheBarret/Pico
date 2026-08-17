# PICO8 BUS (rev 0.2)

### Core Principle

Like stdio's fixed 3-channel interface regardless of how many files are open, PICO-BUS reserves a small,  
fixed address window regardless of how many physical devices sit on the chassis.  
Adding a device never costs additional host address space.  

---

### Layer 0: Bus Control (6 bytes)

| Address | Name | Direction | Purpose |
|---|---|---|---|
| `0xFE00` | `BUS_SELECT` | write | Device ID (0-255) currently addressed |
| `0xFE01` | `BUS_TYPE` | read | Type tag of the selected device (single-byte, locked in, see Layer 4) |
| `0xFE02` | `BUS_STATUS` | read | `0x00`=ready, `0x01`=busy, `0x02`=error, `0xFF`=absent |
| `0xFE03` | `BUS_COMMAND` | write | bit 7 = direction (`0`=write host→device, `1`=read device→host); bits 6-0 = device-specific command |
| `0xFE04` | `BUS_DATA` | read/write | Payload channel, one byte at a time, little-endian for multi-byte values |
| `0xFE05` | `BUS_INT_SOURCE` | read | Device ID of the most recent interrupt; see Layer 3 for delivery guarantee |

---

### Layer 1: Addressing Protocol

```
  1. MOV [BUS_SELECT], device_id
  2. read BUS_STATUS                 ; present? ready?
  3. MOV [BUS_COMMAND], cmd_byte     ; bit7=dir, bits6-0=device opcode
  4. read/write BUS_DATA, repeated   ; transfer payload
```

`BUS_COMMAND`'s split resolves the v0.1 ambiguity:  
A thruster's "set throttle" is opcode `0x01` under the write direction (`0x01`),  
and if a read-back exists it's the same opcode under the read direction (`0x81`),  
one byte, no collision between bus-level direction and device-level meaning.  

---

### Layer 2: Device-Side Contract 

Every device, real peripheral or a full second PICO instance running its own firmware,  
implements the same four behaviors: report `TYPE`/`STATUS` when selected,  
interpret `BUS_COMMAND` per its own opcode space, transfer via `BUS_DATA`, optionally signal via interrupt.  
This is still the property that makes a device and a co-processor indistinguishable from the main CPU's point of view.  

---

### Layer 3: Interrupt Delivery Rule

Without a rule, a second device's interrupt can overwrite `BUS_INT_SOURCE` before firmware reads the first one,  
silently losing an event.  

**Rule:**  
`cpu_trigger_interrupt` checks `interrupt_enabled` before acting.  

- If `interrupt_enabled == 1`: proceed normally, bus layer sets `BUS_INT_SOURCE` to the triggering device's ID,  
  then `cpu_trigger_interrupt` fires as usual (which, via `handle_int`, clears `interrupt_enabled` on entry,  
  this is what protects the ISR from being overwritten mid-handler).  
- If `interrupt_enabled == 0`: the interrupt attempt is **dropped**. `BUS_INT_SOURCE` is not updated. No queueing.  

**Dropping is safe, not lossy in practice:**  
`BUS_STATUS` is the durable record, a device that hit an error or completed an operation holds that state,  
until firmware explicitly reads/clears it, the interrupt is a *notification*, not the only delivery path.  
Firmware's main loop should still poll `BUS_STATUS` for devices it cares about;  
A dropped interrupt just means that device gets noticed on the next poll instead of immediately.  
This keeps the bus consistent with PICO's existing `interrupt_enabled` semantics (Phase 7),  
without adding a new register or ack protocol.  

**ISR pattern this enables:**  
```
; ISR entry (interrupt_enabled already cleared by handle_int)
  MOV acc, [BUS_INT_SOURCE]
  MOV [BUS_SELECT], acc      ; no scan, direct addressing
; ...handle whatever BUS_STATUS says for this device...
  IRET                        ; re-enables interrupts on exit
```

---

### Layer 4: Discovery / Type Query

**`TYPE` is single-byte, locked in.** A device reports exactly one type;  
Composite functionality is a firmware-level domain layered on top via a future `GET_INFO` command,  
not a bitmask on `BUS_TYPE` itself.  

**Discovery is dynamic, not count-based**  
Matches the hot-swap goal, avoids needing a controller-maintained count that could drift from physical reality:  
```
  for id = 1 to 255:
      MOV [BUS_SELECT], id
      status = read [BUS_STATUS]
      if status != 0xFF:              ; present
          type = read [BUS_TYPE]
          if type == TARGET_TYPE:
              ; device found, id is now selected, ready to command
```
> Device `0` remains reserved (bus controller / chassis-level role, not a queryable peripheral).  

---
