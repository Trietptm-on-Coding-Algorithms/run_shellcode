/*
on MACOS:
	gcc -dynamiclib callbuf.c -o callbuf.dylib
*/

#ifdef OS_IS_WINDOWS
	#include <Windows.h>
#else
	#include <sys/mman.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>

int doit(unsigned char *shellcode, int length)
{
	/* allocate executable buffer */
	unsigned char *buf_exec = NULL;

	/* alloc */
	#ifdef OS_IS_WINDOWS
	printf("DLL: VirtualAlloc()\n");
	buf_exec = (unsigned char *)VirtualAlloc(0, length, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
	if(!buf_exec) {
		printf("DLL: ERROR VirtualAlloc()\n");
		return -1;
	}
	#else
	printf("DLL: mmap()\n");
	buf_exec = (unsigned char *)mmap(0, length, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if(!buf_exec) {
		printf("DLL: ERROR mmap()\n");
		return -2;
	}
	#endif

	/* copy to executable buffer */
	printf("DLL: memcpy()\n");
	memcpy(buf_exec, shellcode, length);

	#ifdef OS_IS_WINDOWS
	uint32_t oldProt;
	printf("DLL: VirtualProtect()\n");
	VirtualProtect(buf_exec, length, PAGE_EXECUTE_READWRITE, &oldProt);
	#else
	printf("DLL: mprotect()\n");
	mprotect(buf_exec, length, PROT_READ | PROT_EXEC);
	#endif

	/* execute the buffer */
	printf("DLL: calling %p\n", buf_exec);
	((void (*)(void))buf_exec)();
	printf("DLL: returned\n");

	return 0;
}
