#ifndef SRC_SRAM4_H_
#define SRC_SRAM4_H_
#define SHARED_MEMORY_ADDRESS 0x38000000

typedef struct
{
	uint32_t State1;		//LED1
	uint32_t DATA[500];		//Test Semaphore
}SharedMemory;

volatile SharedMemory *const SRAM4 = (SharedMemory*)(SHARED_MEMORY_ADDRESS);

#endif
