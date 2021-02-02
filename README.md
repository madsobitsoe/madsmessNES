# Stupid NES emulator

A bad NES emulator, written for learning purposes


## NES_STATE
The entire nes is modelled as a `struct nes_state` in `definitions.h`.
It has a master clock, pointers to memory, rom and the `struct cpu_state`.

## PPU
The ppu is currently not implemented. Cycles and frame-count is handled in the nes_state, for compliance with the nestest.nes file and accompanying log.


High-level overview of NES-rendering:
https://austinmorlan.com/posts/nes_rendering_overview/


## CPU
Every instruction on the NES (6502) take 2-7 cpu cycles.
The goal for the CPU-part of the emulator, is to achieve "cycle accuracy", hoping that will make emulating games that depend on "mid-instruction-states" easier.

To achieve this, I do not emulate instructions, but the cycles of each instruction. The emulation is not at all one-to-one, but the important thing for now, is the timing.

This page lists what all instructions do in their specific cycles - the actions depend on the adressing mode. http://nesdev.com/6502_cpu.txt

### CPU-struct
The cpu is modelled as a `struct cpu_state` in `definitions.h`.


### Structure of fetch-decode-execute
The cpu-struct has a circular array of "actions". An action is something the cpu can perform during a cycle.

When a cpu-step is performed, the following happens:

- If the queue is empty, the emulator will read the byte at PC, store it in the nes_state struct and call `add_instruction_to_queue`.
`add_instruction_to_queue` will decode the instruction and add the appropriate `actions` to the `action_queue`.
All instructions start with the same action: fetching the value at PC and incrementing PC.
- `execute_next_action` is called. The function will grab the next action from the queue, increment the next-action-pointer and perform the current action.


Both the instructions, i.e. groups of actions, to be added to the queue, and the actions themselves, are implemented as giant switch cases.
It is not pretty, but it works, and the compiler should be able to optimise it more than a handcrafted jumptable of function pointers.
It does however mean, that a lot of duplicated code exists right now.


## APU
Not implemented in any way.

## Logger
A very simple logging functionality is implemented.
It generates logs of the cpu-state similar to the nintendulator log for nestest.nes.
## Testing
The goal is to pass all the test suites on this page:
https://wiki.nesdev.com/w/index.php/Emulator_tests

The "test-suite" in `test.sh` runs the nestest.nes rom and compares the log-files with `diff`.

nestest.nes: http://nickmass.com/images/nestest.nes
nestest.log: https://www.qmtpro.com/~nes/misc/nestest.txt
