#include <cstdlib>
#include <cstring>
#include <random>

#include "../include/Chip8.hpp"
#include "../include/Fontset.hpp"

const std::uint32_t START_ADDR   = 0x200;
const std::uint32_t FONTSET_ADDR = 0x50;

CHIP8::CHIP8()
{
    I = 0;              // Clear the I register
    SP = 0;             // Clear the SP
    PC = START_ADDR;    // Set the PC to the start address
    opcode = 0;         // Clear the opcode

    // Clear the stack
    for (int i = 0; i < 16; ++i)
    {
        stack[i] = 0;
    }

    std::memset(V, 0, sizeof V);
    std::memset(memory, 0, sizeof memory);
    std::memset(display, 0, sizeof display);
    std::memset(keypad, 0, sizeof keypad);

    // Load the font on memory
    for (int i = 0; i < 80; ++i)
    {
        memory[i + FONTSET_ADDR] = FONTSET[i];
    }

    // Reseting timers
    delay_timer = 0;
    sound_timer = 0;

    // Seed
    srand(time(NULL));
}

CHIP8::~CHIP8() {}


