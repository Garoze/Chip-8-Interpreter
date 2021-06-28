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
        size_t file_size = file.tellg();
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

        return true;
    }

    return false;
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
            switch (KK(opcode))
            {
                case 0xE0: // 00E0
                {
                    printf("Opcode: %04x\tCLS\n", opcode);
                    std::memset(display, 0, sizeof display);
                    drawFlag = true;
                    PC += 2;
                }
                break;

                case 0xEE: // 00EE
                {
                    printf("Opcode: %04x\tRET\n", opcode);
                    PC = stack[--SP];
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

        case 0x2:  //2NNN
        {
           printf("Opcode: %04x\tCALL $%03x\n", opcode, NNN(opcode));
           stack[SP++] = PC;
           PC = NNN(opcode);
        }
        break;

        case 0x3: //3XKK
        {
            printf("Opcode: %04x\tSE V%01x, #%02x\n", opcode, VX(opcode), KK(opcode));
            if (V[VX(opcode)] == KK(opcode))
                PC += 4;
            else
                PC += 2;
        }
        break;

        case 0x4: // 4XKK
        {
            printf("Opcode: %04x\tSNE V%01x, #%02x\n", opcode, VX(opcode), KK(opcode));
            if (V[VX(opcode)] != KK(opcode))
                PC += 4;
            else
                PC += 2;
        }
        break;

        case 0x5: //5XY0
        {
            printf("Opcode: %04x\tSE V%01x, V%01x\n", opcode, VX(opcode), VY(opcode));
            if (V[VX(opcode)] == V[VY(opcode)])
                PC += 4;
            else
                PC += 2;
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

        case 0x8:
        {
            switch (nib(opcode))
            {
                case 0x0: // 8XY0
                {
                    printf("Opcode: %04x\tLD V%01x, V%01x\n", opcode, VX(opcode), VY(opcode));
                    V[VX(opcode)] = V[VY(opcode)];
                    PC += 2;
                }
                break;

                case 0x1: // 8XY1
                {
                    printf("Opcode: %04x\tOR V%01x, V%01x\n", opcode, VX(opcode), VY(opcode));
                    V[VX(opcode)] |= V[VY(opcode)];
                    PC += 2;
                }
                break;

                case 0x2: // 8XY2
                {
                    printf("Opcode: %04x\tAND V%01x, V%01x\n", opcode, VX(opcode), VY(opcode));
                    V[VX(opcode)] &= V[VY(opcode)];
                    PC += 2;
                }
                break;

                case 0x3: // 8XY3
                {
                    printf("Opcode: %04x\tXOR V%01x, V%01x\n", opcode, VX(opcode), VY(opcode));
                    V[VX(opcode)] ^= V[VY(opcode)];
                    PC += 2;
                }
                break;

                case 0x4: // 8XY4
                {
                    printf("Opcode: %04x\tADD V%01x, V%01x\n", opcode, VX(opcode), VY(opcode));
                    std::uint8_t flag = (V[VY(opcode)] > (0xFF - V[VX(opcode)])) ? 1 : 0;
                    V[VX(opcode)] += V[VY(opcode)];
                    V[0xF] = flag;
                    PC += 2;
                }
                break;

                case 0x5: // 8XY5
                {
                    printf("Opcode: %04x\tSUB V%01x, V%01x\n", opcode, VX(opcode), VY(opcode));
                    std::uint8_t flag = (V[VY(opcode)] > V[VX(opcode)]) ? 0 : 1;
                    V[VX(opcode)] -= V[VY(opcode)];
                    V[0xF] = flag;
                    PC += 2;
                }
                break;

                case 0x6: // 8XY6
                {
                    printf("Opcode: %04x\tSHR V%01x\n", opcode, VX(opcode));
                    std::uint8_t flag = V[VX(opcode)] & 0x1;
                    V[VX(opcode)] >>= 1;
                    PC += 2;
                }
                break;

                case 0x7: // 8XY7
                {
                    printf("Opcode: %04x\tSUBN V%01x, V%01x\n", opcode, VX(opcode), VY(opcode));
                    std::uint8_t flag = (V[VY(opcode)] > V[VX(opcode)]) ? 1 : 0;
                    V[VX(opcode)] = V[VY(opcode)] - V[VX(opcode)];
                    V[0xF] = flag;
                    PC += 2;
                }
                break;

                case 0xE: // 8XYE
                {
                    printf("Opcode: %04x\tSHL V%01x {, V%01x}\n", opcode, VX(opcode), VY(opcode));
                    std::uint8_t flag = V[VX(opcode)] >> 7;
                    V[VX(opcode)] <<= 1;
                    V[0xF] = flag;
                    PC += 2;
                }
                break;

                default: printf("Unknown opcode [0x8000]: 0x%X\n", opcode); break;
            }
        }
        break;

        case 0x9: // 9XY0
        {
            printf("Opcode: %04x\tSNE V%01x, V%01x\n", opcode, VX(opcode), VY(opcode));
            if (V[VX(opcode)] != V[VY(opcode)])
                PC += 4;
            else
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

        case 0xB: // BNNN
        {
            printf("Opcode: %04x\tJP V0, $%03x\n", opcode, NNN(opcode));
            PC = V[0x0] + NNN(opcode);
        }
        break;

        case 0xC: // CXKK
        {
            printf("Opcode: %04x\tRND V%01x, #%02x\n", opcode, VX(opcode), KK(opcode));
            V[VX(opcode)] = (rand() % 0xFF) & KK(opcode);
            PC += 2;
        }
        break;

        case 0xD: // DXYN
        {
            printf("Opcode: %04x\tDRAW V%01x, V%01x, %01x\n", opcode, VX(opcode), VY(opcode), nib(opcode));
            V[0xF] = 0;
            std::uint8_t pixel;

            for (int y = 0; y < nib(opcode); ++y)
            {
                pixel = memory[I + y];
                for (int x = 0; x < 8; ++x)
                {
                    if ((pixel & (0x80 >> x)) != 0)
                    {
                        std::uint8_t X = (V[VX(opcode)] + x) % 64;
                        std::uint8_t Y = (V[VY(opcode)] + y) % 32;

                        if (display[X + (Y * 64)])
                            V[0xF] = 1;

                        display[X + (Y * 64)] ^= 1;
                    }
                }
            }
            drawFlag = true;
            PC += 2;
        }
        break;

        case 0xE:
        {
            switch (KK(opcode))
            {
                case 0x9E: // EX9E
                {
                    printf("Opcode: %04x\tSKP V%01x\n", opcode, VX(opcode));
                    if (keypad[V[VX(opcode)]] != 0)
                        PC += 4;
                    else
                        PC += 2;
                }
                break;

                case 0xA1: // EXA1
                {
                    printf("Opcode: %04x\tSKNP V%01x\n", opcode, VX(opcode));
                    if (V[VX(opcode)] == 0)
                        PC += 4;
                    else
                        PC += 2;
                }
                break;

                default: printf("Unknown opcode [0xE000]: 0x%X\n", opcode); break;
            }
        }
        break;

        case 0xF:
        {
            switch (KK(opcode))
            {
                case 0x07: // FX07
                {
                    printf("Opcode: %04x\tLD V%01x, DT\n", opcode, VX(opcode));
                    V[VX(opcode)] = delay_timer;
                    PC += 2;
                }
                break;

                case 0x0A: // FX0A
                {
                    printf("Opcode: %04x\tLD V%01x, K\n", opcode, VX(opcode));
                    bool key_pressed;

                    for (int i = 0; i < 16; ++i)
                        if (keypad[i] != 0)
                        {
                            V[VX(opcode)] = i;
                            key_pressed = true;
                        }

                    if (!key_pressed) return;
                    PC += 2;
                }
                break;

                case 0x15: // FX15
                {
                    printf("Opcode: %04x\tLD DT, V%01x\n", opcode, VX(opcode));
                    delay_timer = V[VX(opcode)];
                    PC += 2;
                }
                break;

                case 0x18: // FX18
                {
                    printf("Opcode: %04x\tLD ST, V%01x\n", opcode, VX(opcode));
                    sound_timer = V[VX(opcode)];
                    PC += 2;
                }
                break;

                case 0x1E: // FX1E
                {
                    printf("Opcode: %04x\tADD I, V%01x\n", opcode, VX(opcode));
                    std::uint8_t flag = (I + V[VX(opcode)] > 0xFFF) ? 1 : 0;
                    I += V[VX(opcode)];
                    V[0xF] = flag;
                    PC += 2;
                }
                break;

                case 0x29: // FX29
                {
                    printf("Opcode: %04x\tLD F, V%01x\n", opcode, VX(opcode));
                    // I = V[VX(opcode)] * 0.5;
                    I = (V[VX(opcode)] * 5) + FONTSET_ADDR;
                    PC += 2;
                }
                break;

                case 0x33: // FX33
                {
                    printf("Opcode: %04x\tLD B, V%01x\n", opcode, VX(opcode));
                    memory[I] = V[VX(opcode)] / 100;
                    memory[I + 1] = (V[VX(opcode)] % 100) / 10;
                    memory[I + 2] = V[VX(opcode)] % 10;
                    PC += 2;
                }
                break;

                case 0x55: // FX55
                {
                    printf("Opcode: %04x\tLD [I], V%01x\n", opcode, VX(opcode));
                    for (int i = 0; i <= VX(opcode); ++i)
                        memory[I + i] = V[i];

                    I += VX(opcode) + 2;
                    PC += 2;
                }
                break;

                case 0x65: // FX65
                {
                    printf("Opcode: %04x\tLD V%01x, [I]\n", opcode, VX(opcode));
                    for (int i = 0; i <= VX(opcode); ++i)
                        V[i] = memory[I + i];

                    I += VX(opcode) + 1;
                    PC += 2;
                }
                break;

                default: printf("Unknown opcode [0xF000]: 0x%X\n", opcode); break;
            }
        }
        break;

        default: printf("Opcode: %04x\tUnknkow\n", opcode); break;
    }
}
