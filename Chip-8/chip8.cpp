
#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


unsigned char chip8_fontset[80] =
{ 
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};



void chip8::init(){
	pc = 0x200;
	opcode = 0;
	I = 0;
	sp = 0;

	for(int i = 0; i < 2048; ++i)
		gfx[i] = 0;

	for(int i = 0; i < 16; ++i)
		stack[i] = 0;

	for(int i = 0; i < 4096; ++i)
		memory[i] = 0;

	for(int i = 0; i < 80; ++i)
		memory[i] = chip8_fontset[i]; //Loading fonts

	delay_timer = 0;
	sound_timer = 0;

	drawFlag = true;

	srand(time(NULL));
}

void chip8::emulateCycle(){
//Fetch opcode
	opcode = memory[pc] << 8 | memory[pc + 1];
//Decode opcode
	switch(opcode & 0xF000)//0xF000 = only keeps the top 4 bits. Everything else is 0
	{


		case 0x0000:
			switch(opcode & 0x000F)
			{
				case 0x0000: //Clears the screen
				//Execute
				break;

				case 0x000E: //0x00EE: Returns from subroutine
					//Execute opcode
				break;

				default:
					printf("Unknown Opcode [0x0000] : 0x%X\n", opcode);
			}
		break;

		case 0x1000:
			pc = opcode & 0x0FFF;
		break;

		case 0x2000: //Opcode 0x8XY4. Adds the value of VY to VX(they're registers), if >255, adds 1 to carry flag VF
			stack[sp] = pc;
			++sp;
			pc = opcode & 0x0FFF;
		break;

		case 0x3000: // opcode & 0x0F00) >> 8 = top 4 bits, top nibble. If V[top four bits] = value of VX register. if it's equal to the 1 byte opcode, pc +4; 
			if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
		break;
		
		case 0x4000:
			if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				pc += 4;
			else
				pc += 2;
		break;

		case 0x5000: //0x5XY0: Skips if VX = VY
			if(V[(opcode & 0x0F00)>> 8] == V[(opcode & 0x00F0) >> 4])
				pc += 4;
			else
				pc += 2;
		break;

		case 0x6000: //0x6XNN Set VX = NN
			V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
			pc += 2;
		break;

		case 0x7000:
			V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
			pc += 2;
		break;

		case 0x8000:
			switch(opcode & 0x000F){

				case 0x0000:
					V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0001:
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0002;
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;


				case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0004: 
					if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8])) 
						V[0xF] = 1; //carry
					else 
						V[0xF] = 0;					
					V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
					pc += 2;					
				break;

				case 0x0005:
					if(V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8]) 
						V[0xF] = 0; // there is a borrow
					else 
						V[0xF] = 1;					
					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					pc += 2;
				break;

				case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before the shift
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
					V[(opcode & 0x0F00) >> 8] >>= 1;
					pc += 2;
				break;

				case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before the shift
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
					V[(opcode & 0x0F00) >> 8] <<= 1;
					pc += 2;
				break;

				default:
					printf ("Unknown opcode [0x08000]: 0x%X\n", opcode);

			}
		break;

		case 0x9000:
			if(V[(opcode & 0x0F00) >>8] != V[(opcode &0x00F0) >>4])
				pc += 4;
			else
				pc += 2;
		break;


		case 0xA000: //ANNN: Sets I to the address NNN
			I = opcode & 0x0FFF;
			pc += 2;
		break;

		case 0xB000:
			pc = (opcode & 0x00FF) + V[0];
		break;

		case 0xC000:
			V[(opcode & 0x0F00) >> 8] = ((rand()%0xFF) & (opcode & 0x00FF));
		break;

		case 0xD000://Draw sprites? no idea how
			unsigned short x = V[(opcode & 0x0F00) >> 8];
			unsigned short y = V[(opcode & 0x00F0) >> 4];
			unsigned short height = opcode & 0x000F;
			unsigned short pixel;

			V[0xF] = 0;
			for(int yline = 0; yline < height; yline++){
				pixel = memory[I + yline];
				for(int xline = 0; xline < 8; xline++){
					if(pixel & (0x80 >> xline) != 0){//Supposed to check each bit of pixel, one bit at a time(0x80 >> line) does that
					if(gfx[(x + xline + ((y + yline)*64))] == 1)
						V[0xF] = 1;//Collision Detection, VF flag is set to 1 if the thing has to be unset. gfx[(VX + bit number in the row) + VY + row number]
					gfx[x + xline + ((y + yline)*64)] ^= 1;//XOR flag to set pixel values gfx[] is 64 * 32, i.e., 64 rows, 32 columns. therefore, *64
					}
				}
				
			}
			drawFlag = true;//Need to update the screen again as we changed the gfx[] array value
			pc += 2;
		break;

		case 0xE000:
			switch(opcode & 0x00FF){
				case 0x009E: //0xEX9E
					if(key[V[(opcode & 0x0F00) >> 8]] != 0)
						pc += 4;
					else
						pc += 2;
				break;

				case 0x00A1:
					if(key[V[(opcode & 0x0F00) >> 8]] == 0)
						pc += 4;
					else
						pc += 2;
				break;
			}

		case 0xF000:
			switch(opcode & 0x00FF){
				case 0x0007:
					V[(opcode & 0x0F00) >> 8] = delay_timer;
					pc += 2;
				break;
				case 0x000A:
					bool keyPress = false;
					
					for(int i = 0; i < 16; ++i){
						if(key[i] !=0){
							V[(opcode & 0x0F00) >> 8] = i;
							keyPress=true;
						}
					}

					if(!keyPress)
						return;
				pc += 2;
				break;

				case 0x0015:
					delay_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
				break;
				case 0x0018:
					sound_timer = V[(opcode & 0x0F00) >> 8];
					pc += 2;
				break;

				0x001E: 
					if(I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
						V[0xF] = 1;
					else
						V[0xF] = 0;
				I += V[(opcode & 0x0F00) >>8];
				pc += 2;
				break;

				case 0x0029:
					I = V[(opcode & 0x0F00) >> 8] *0x5;
					pc += 2;
				break;

				case 0x0033:
					memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
					memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;		
					pc += 2;
				break;

				case 0x0055:
					for(int i = 0; i <= ((opcode &0x0F00) >>8); ++i)
						memory[I + i] = V[i];

					I+ = ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
				break;
				
				case 0x0065:
					for(int i = 0; i <= ((opcode &0x0F00) >>8); ++i)
						memory[I + i] = V[i];

					I+ = ((opcode & 0x0F00) >> 8) + 1;
					pc += 2;
				break;

				default:
					print("Unknown opcode [0xF000]: 0x%X\n", opcode);
			}
		break;

		default:
			printf("Unknown opcode: 0x%X\n", opcode);
	}
	

	//Update timers
	if(delay_timer > 0)
		--delay_timer;
	if(sound_timer > 0){
		if(sound_timer == 1)
			printf("BEEP!\n");
		--sound_timer;
	}
}
void chip8::debugRenderer(){

	for(int y = 0; y < 32; ++y){
		for(int x = 0; x < 64; ++x){
			if(gfx[(y*64) + x] == 0)
				printf("0");
			else
				printf(" ");
		}
		printf("\n");
	}
	printf("\n");


}
bool chip8::loadApplication(const char * filename){

	init();
	printf("Loading: %s\n", filename);

	FILE * pfile = fopen(filename, "rb");
	if(pFile == NULL){
		fputs("File error", stderr);
		return false;
	}
	//File size
	fseek(pFile, 0, SEEK_END);
	Long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);

	char * buffer = (char*)malloc(sizeof(char) * lSizez);
	if(buffer == NULL){
		fputs("Memory error", stderr);
		return false;
	}

	size_t result = fread (buffer, 1, lSize, pFile);
	if(result != lSize){
		fputs("Reading error", stderr);
		return false;
	}

	if((4096-512) > lSize)
	{
		for(int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error:ROM too big for memory");

	fclose(pFile);
	free(buffer);

	return true;
}