#include <random>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <stdio.h>

#include "../include/Chip8.hpp"
#include "../include/Fontset.hpp"

#define KK(opcode)(opcode & 0x00FF)
#define NNN(opcode)(opcode & 0x0FFF)
#define nib(opcode)(opcode & 0x000F)

#define VX(opcode)((opcode & 0x0F00) >> 8)
#define VY(opcode)((opcode & 0x00F0) >> 4)

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

void CHIP8::EmulateCycle()
{
    // Fetch opcode
    opcode = memory[PC] << 8 | memory[PC + 1];

    // Decode
    switch ((opcode & 0xF000) >> 12)
    {
        case 0x0:
        {
            switch (nib(opcode))
            {
                case 0xE0: // 00E0
                {
                    printf("Opcode: %04x\tCLS\n", opcode);
                    std::memset(display, 0, sizeof display);
                    drawFlag = true;
                    PC += 2;
                }
                break;

                default: printf("Unknown opcode [0x0000]: 0x%X\n", opcode); break;
            }
        }
        break;

        case 0x1: // 1NNN
        {
            printf("Opcode: %04x\tJMP $%03x\n", opcode, NNN(opcode));
            PC = NNN(opcode);
        }
        break;

        case 0x6: // 6XKK
        {
            printf("Opcode: %04x\tLD V%01x, #%02x\n", opcode, VX(opcode), KK(opcode));
            V[VX(opcode)] = KK(opcode);
            PC += 2;
        }
        break;

        case 0x7: // 7XKK
        {
            printf("Opcode: %04x\tADD V%01x, #%02x\n", opcode, VX(opcode), KK(opcode));
            V[VX(opcode)] += KK(opcode);
            PC += 2;
        }
        break;

        case 0xA: // ANNN
        {
            printf("Opcode: %04x\tLD I, $%03x\n", opcode, NNN(opcode));
            I = NNN(opcode);
            PC += 2;
        }
        break;

        case 0xD: // DXYN
        {
            V[0xF] = 0;
            std::uint16_t pixel;

            for (int y = 0; y < nib(opcode); ++y)
            {
                pixel = memory[I + y];
                for (int x = 0; x < 8; ++x)
                {
                    if (display[(V[VX(opcode)] + x) % 64] + ((V[VY(opcode)] + y) % 32) * 64)
                    {
                        V[0xF] = 1;
                    }

                    display[((V[VX(opcode)] + x) % 64) + (((V[VY(opcode)] + y) % 32) * 64)] ^= 1;
                }
            }
        }
        break;
    }
}
