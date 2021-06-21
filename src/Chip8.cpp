#include <random>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <iostream>

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
        memory[FONTSET_ADDR + i] = FONTSET[i];
    }

    // Reseting timers
    delay_timer = 0;
    sound_timer = 0;

    // Seed
    srand(time(NULL));
}

CHIP8::~CHIP8() {}

bool CHIP8::LoadROM(const char *filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::in);

    if (file.good())
    {
        file.seekg(0, std::ios::end);
        int file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        char* buffer = new char[file_size];

        file.read(buffer, file_size);
        file.close();

        if (file_size < (4096 - 512))
            for (int i = 0; i < file_size; ++i)
                memory[START_ADDR + i] = buffer[i];
        else
            std::cerr << "The file is too big!" << std::endl;

        delete[] buffer;

        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

