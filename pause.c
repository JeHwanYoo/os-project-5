#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int main(void) {
	char *base_addr;
	// 메모리 페이지 매핑 (100KB)
	if ((base_addr = mmap(NULL, 102400, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == NULL) {
		return 1;
	}
	// 데이터 쓰기
	memcpy(base_addr, "Hello, World!", 13);
	strcat(base_addr, "concat!!!!!!!!!!");
	strcat(base_addr, "concat!!!!!!!!!!");
	pause();
}
