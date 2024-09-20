#ifndef CPU
#define CPU
#include <cstdint>
#include <string>
#include <chrono>
#include <random>

const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;
const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;

class Chip8 {
public:
	Chip8();
	void LoadROM(char const* filename);
	void cycle();
	uint8_t get_soundtimer() {
		return sound_timer;
	}

	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};
	uint8_t keypad[KEY_COUNT]{};
private:
	uint8_t registers[REGISTER_COUNT]{};
	uint8_t memory[MEMORY_SIZE]{};
	uint16_t index{};
	uint16_t pc{};
	uint16_t stack[STACK_LEVELS]{};
	uint8_t sp{};
	uint8_t delay_timer{};
	uint8_t sound_timer{};

	uint16_t opcode{};

	void Table0();
	void Table8();
	void TableE();
	void TableF();
	void OP_NULL();

	void OP_1NNN();
	void OP_2NNN();
	void OP_3XKK();
	void OP_4XKK();
	void OP_5XY0();
	void OP_6XKK();
	void OP_7XKK();
	void OP_9XY0();
	void OP_ANNN();
	void OP_BNNN();
	void OP_CXKK();
	void OP_DXYN();

	void OP_00E0();
	void OP_00EE();

	void OP_8XY0();
	void OP_8XY1();
	void OP_8XY2();
	void OP_8XY3();
	void OP_8XY4();
	void OP_8XY5();
	void OP_8XY6();
	void OP_8XY7();
	void OP_8XYE();

	void OP_EX9E();
	void OP_EXA1();

	void OP_FX07();
	void OP_FX0A();
	void OP_FX15();
	void OP_FX18();
	void OP_FX1E();
	void OP_FX29();
	void OP_FX33();
	void OP_FX55();
	void OP_FX65();

	std::default_random_engine rand_gen;
	std::uniform_int_distribution<unsigned int> rand_byte;

	typedef void (Chip8::* Chip8Func)();
	Chip8Func table[0xF + 1];
	Chip8Func table0[0xE + 1];
	Chip8Func table8[0xE + 1];
	Chip8Func tableE[0xE + 1];
	Chip8Func tableF[0x65 + 1];

};

#endif // !CPU