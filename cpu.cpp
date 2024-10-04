#include "cpu.h"
#include <fstream>

const unsigned int FONTSET_SIZE = 80;
const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_START_ADDRESS = 0x50;

uint8_t fontset[FONTSET_SIZE] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0,
	0x20, 0x60, 0x20, 0x20, 0x70,
	0xF0, 0x10, 0xF0, 0x80, 0xF0,
	0xF0, 0x10, 0xF0, 0x10, 0xF0,
	0x90, 0x90, 0xF0, 0x10, 0x10,
	0xF0, 0x80, 0xF0, 0x10, 0xF0,
	0xF0, 0x80, 0xF0, 0x90, 0xF0,
	0xF0, 0x10, 0x20, 0x40, 0x40,
	0xF0, 0x90, 0xF0, 0x90, 0xF0,
	0xF0, 0x90, 0xF0, 0x10, 0xF0,
	0xF0, 0x90, 0xF0, 0x90, 0x90,
	0xE0, 0x90, 0xE0, 0x90, 0xE0,
	0xF0, 0x80, 0x80, 0x80, 0xF0,
	0xE0, 0x90, 0x90, 0x90, 0xE0,
	0xF0, 0x80, 0xF0, 0x80, 0xF0,
	0xF0, 0x80, 0xF0, 0x80, 0x80
};

Chip8::Chip8() : rand_gen(std::chrono::system_clock::now().time_since_epoch().count()) {
	pc = START_ADDRESS;

	for (unsigned int i = 0; i < FONTSET_SIZE; i++)
		memory[FONTSET_START_ADDRESS + i] = fontset[i];

	rand_byte = std::uniform_int_distribution<unsigned int>(0, 255U);

	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1NNN;
	table[0x2] = &Chip8::OP_2NNN;
	table[0x3] = &Chip8::OP_3XKK;
	table[0x4] = &Chip8::OP_4XKK;
	table[0x5] = &Chip8::OP_5XY0;
	table[0x6] = &Chip8::OP_6XKK;
	table[0x7] = &Chip8::OP_7XKK;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9XY0;
	table[0xA] = &Chip8::OP_ANNN;
	table[0xB] = &Chip8::OP_BNNN;
	table[0xC] = &Chip8::OP_CXKK;
	table[0xD] = &Chip8::OP_DXYN;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	for (size_t i = 0; i <= 0xE; i++)
	{
		table0[i] = &Chip8::OP_NULL;
		table8[i] = &Chip8::OP_NULL;
		tableE[i] = &Chip8::OP_NULL;
	}

	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	table8[0x0] = &Chip8::OP_8XY0;
	table8[0x1] = &Chip8::OP_8XY1;
	table8[0x2] = &Chip8::OP_8XY2;
	table8[0x3] = &Chip8::OP_8XY3;
	table8[0x4] = &Chip8::OP_8XY4;
	table8[0x5] = &Chip8::OP_8XY5;
	table8[0x6] = &Chip8::OP_8XY6;
	table8[0x7] = &Chip8::OP_8XY7;
	table8[0xE] = &Chip8::OP_8XYE;

	tableE[0x1] = &Chip8::OP_EXA1;
	tableE[0xE] = &Chip8::OP_EX9E;

	for (size_t i = 0; i <= 0x65; i++)
	{
		tableF[i] = &Chip8::OP_NULL;
	}

	tableF[0x07] = &Chip8::OP_FX07;
	tableF[0x0A] = &Chip8::OP_FX0A;
	tableF[0x15] = &Chip8::OP_FX15;
	tableF[0x18] = &Chip8::OP_FX18;
	tableF[0x1E] = &Chip8::OP_FX1E;
	tableF[0x29] = &Chip8::OP_FX29;
	tableF[0x33] = &Chip8::OP_FX33;
	tableF[0x55] = &Chip8::OP_FX55;
	tableF[0x65] = &Chip8::OP_FX65;
}

void Chip8::Table0()
{
	((*this).*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8()
{
	((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE()
{
	((*this).*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF()
{
	((*this).*(tableF[opcode & 0x00FFu]))();
}

void Chip8::OP_NULL()
{}

void Chip8::LoadROM(char const* filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		std::streampos size = file.tellg();
		char* buffer = new char[size];
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		for (long i = 0; i < size; ++i)
		{
			memory[START_ADDRESS + i] = buffer[i];
		}

		delete[] buffer;
	}
}

std::string Chip8::get_opcode_string(uint16_t opcode)
{
	std::stringstream ss;

	uint8_t first = (opcode & 0xF000u) >> 12;
	uint8_t second = (opcode & 0x0F00u) >> 8;
	uint8_t third = (opcode & 0x00F0u) >> 4;
	uint8_t fourth = (opcode & 0x000Fu);

	ss << std::hex << std::uppercase << static_cast<int>(first) << static_cast<int>(second) << static_cast<int>(third) << static_cast<int>(fourth);

	return ss.str();
}

void Chip8::cycle(std::vector<std::string>& opcode_history)
{
	opcode = (memory[pc] << 8) | memory[pc + 1];

	opcode_history.push_back(get_opcode_string(opcode));

	if (opcode_history.size() > 50)
		opcode_history.erase(opcode_history.begin());

	pc += 2;

	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	if (delay_timer > 0)
		delay_timer--;

	if (sound_timer > 0)
		sound_timer--;
}

void Chip8::print_registers(std::vector<std::string>& register_info)
{
	register_info.clear();
	register_info.push_back("PC: " + std::to_string(pc));
	register_info.push_back("Index: " + std::to_string(index));
	register_info.push_back("Stack: " + std::to_string(sp));
	register_info.push_back("Delay Timer: " + std::to_string(delay_timer));
	register_info.push_back("Sound Timer: " + std::to_string(sound_timer));

	for (int i = 0; i < REGISTER_COUNT; i++) {
		register_info.push_back("V" + std::to_string(i) + ": " + std::to_string(static_cast<int>(registers[i])));
	}
}

void Chip8::OP_00E0()
{
	memset(video, 0, sizeof(video));
}

void Chip8::OP_00EE()
{
	sp--;
	pc = stack[sp];
}

void Chip8::OP_1NNN()
{
	uint16_t addr = opcode & 0xFFFu;
	pc = addr;
}

void Chip8::OP_2NNN()
{
	uint16_t addr = opcode & 0x0FFFu;

	stack[sp] = pc;
	sp++;
	pc = addr;
}

void Chip8::OP_3XKK()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] == byte)
		pc += 2;
}

void Chip8::OP_4XKK()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	if (registers[Vx] != byte)
		pc += 2;
}

void Chip8::OP_5XY0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] == registers[Vy])
		pc += 2;
}

void Chip8::OP_6XKK()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = byte;
}

void Chip8::OP_7XKK()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] += byte;
}

void Chip8::OP_8XY0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

void Chip8::OP_8XY1()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] |= registers[Vy];
}

void Chip8::OP_8XY2()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}

void Chip8::OP_8XY3()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}

void Chip8::OP_8XY4()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255U)
		registers[0xF] = 1;
	else
		registers[0xF] = 0;

	registers[Vx] = sum & 0xFFu;
}

void Chip8::OP_8XY5()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] > registers[Vy])
		registers[0xF] = 1;
	else
		registers[0xF] = 0;

	registers[Vx] -= registers[Vy];
}

void Chip8::OP_8XY6()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[0xF] = (registers[Vx] & 0x1u);

	registers[Vx] >>= 1;
}

void Chip8::OP_8XY7()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vy] > registers[Vx])
		registers[0xF] = 1;
	else
		registers[0xF] = 0;

	registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8XYE()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] <<= 1;
}

void Chip8::OP_9XY0()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	if (registers[Vx] != registers[Vy])
		pc += 2;
}

void Chip8::OP_ANNN()
{
	uint16_t addr = opcode & 0x0FFFu;

	index = addr;
}

void Chip8::OP_BNNN()
{
	uint16_t addr = opcode & 0x0FFFu;

	pc = registers[0] + addr;
}

void Chip8::OP_CXKK()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	registers[Vx] = static_cast<uint8_t>(rand_byte(rand_gen)) & byte;
}

void Chip8::OP_DXYN()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	uint8_t x_pos = registers[Vx] % VIDEO_WIDTH;
	uint8_t y_pos = registers[Vy] % VIDEO_HEIGHT;

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; row++) {
		uint8_t sprite_byte = memory[index + row];

		for (unsigned int col = 0; col < 8; col++) {
			uint8_t sprite_pixel = sprite_byte & (0x80u >> col);
			uint32_t* screen_pixel = &video[(y_pos + row) * VIDEO_WIDTH + (x_pos + col)];

			if (sprite_pixel) {
				if (*screen_pixel == 0xFFFFFFFF) {
					registers[0xF] = 1;
				}
				*screen_pixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void Chip8::OP_EX9E()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];

	if (keypad[key])
		pc += 2;
}

void Chip8::OP_EXA1()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t key = registers[Vx];

	if (!keypad[key])
		pc += 2;
}

void Chip8::OP_FX07()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = delay_timer;
}

void Chip8::OP_FX0A()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (keypad[0])
		registers[Vx] = 0;
	else if (keypad[1])
		registers[Vx] = 1;
	else if (keypad[2])
		registers[Vx] = 2;
	else if (keypad[3])
		registers[Vx] = 3;
	else if (keypad[4])
		registers[Vx] = 4;
	else if (keypad[5])
		registers[Vx] = 5;
	else if (keypad[6])
		registers[Vx] = 6;
	else if (keypad[7])
		registers[Vx] = 7;
	else if (keypad[8])
		registers[Vx] = 8;
	else if (keypad[9])
		registers[Vx] = 9;
	else if (keypad[10])
		registers[Vx] = 10;
	else if (keypad[11])
		registers[Vx] = 11;
	else if (keypad[12])
		registers[Vx] = 12;
	else if (keypad[13])
		registers[Vx] = 13;
	else if (keypad[14])
		registers[Vx] = 14;
	else if (keypad[15])
		registers[Vx] = 15;
	else
		pc -= 2;
}

void Chip8::OP_FX15()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delay_timer = registers[Vx];
}

void Chip8::OP_FX18()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	sound_timer = registers[Vx];
}

void Chip8::OP_FX1E()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	index += registers[Vx];
}

void Chip8::OP_FX29()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	index = FONTSET_START_ADDRESS + (5 * digit);
}

void Chip8::OP_FX33()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	memory[index + 2] = value % 10;
	value /= 10;

	memory[index + 1] = value % 10;
	value /= 10;

	memory[index] = value % 10;

}

void Chip8::OP_FX55()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; i++)
		memory[index + i] = registers[i];
}

void Chip8::OP_FX65()
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; i++)
		registers[i] = memory[index + i];
}
