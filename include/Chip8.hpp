#pragma once

#include <cstdint>

class CHIP8
{
public:
    CHIP8();
    ~CHIP8();

    void EmulateCycle();
    bool LoadROM(const char* filename);
public:
    bool drawFlag;
    std::uint8_t keypad[16];
    std::uint8_t display[64 * 32];
private:
    std::uint8_t V[16];
    std::uint8_t memory[4096];

    std::uint16_t I;
    std::uint16_t PC;
    std::uint16_t SP;
    std::uint16_t opcode;

    std::uint16_t stack[16];

    std::uint8_t delay_timer;
    std::uint8_t sound_timer;
};
