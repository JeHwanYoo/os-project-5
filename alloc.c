#include "alloc.h"

struct chunk {
	int used;
	int endpoint;
};

struct chunk mm[PAGESIZE/MINALLOC];

char *base_addr = NULL; // 페이지 시작 주소

// mmap(2) OS 4KB 을 통해 에서 페이지를 할당하고 필요한 다른 데이터 구조를 초기화하는 등 
// 메모리 관리자 초기화 이 함수는 메모리 관리자에게 메모리를 요청하기 전에 사용자가 호출 
// 성공 시 0을 리턴 에러 시 이 아닌 오류 코드 리턴
int init_alloc() {
	// 4KB 메모리 할당
	if ((base_addr = (char *)mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == NULL) {
		return 1;
	}
	return 0;
}
// 메모리 관리자의 상태를 정리하고 페이지 커널에게 다시 반환 
// 성공 시 0을 리턴 에러 시 0이 아닌 오류 코드 리턴
int cleanup() {
	if (base_addr != NULL) {
		memset(mm, 0, PAGESIZE/MINALLOC);
		return munmap(base_addr, PAGESIZE);
	}
	return -1;
}
// 할당할 integer의 buffer 크기를 갖고 성공 시 버퍼에 대한 포인터를 리턴, 에러 시 NULL 리턴
char *alloc(int size) {
	int need_chunk, chunk_length, found, start, i;

	// 최소 8byte, 최대 4KB
	if (size < MINALLOC || size > PAGESIZE) {
		return NULL;
	}
	
	// 필요한 청크의 개수를 구함
	if (size % MINALLOC == 0) {
		need_chunk = size / MINALLOC;
	}
	// 나누어 떨어지지 않으면 에러임
	else {
		return NULL;
	}

	// 연속된 청크를 탐색 (FIRST FIT)
	for (i = 0, chunk_length = 0, found = -1, start = 0; i < PAGESIZE/MINALLOC; i++) {
		if (mm[i].used == 0) {
			if (chunk_length == 0) {
				start = i;
			}
			chunk_length++;
		} else {
			chunk_length = 0;
		}
		// 발견
		if (chunk_length == need_chunk) {
			found = start;
			mm[i].endpoint = 1;
			break;
		}
	}
	// 발견했으면 주소를 리턴
	if (found != -1) {
		i = found;
		while(mm[i].endpoint != 1) {
			mm[i++].used = 1;
		}
		mm[i].used = 1;
		return base_addr + found * MINALLOC;
	}
	// 할당할 여유 공간이 부족함
	else {
		return NULL;
	}
	
}
// 이전에 할당 된 메모리 chunk에 대한 포인터로 전체 chunk를 해제
void dealloc(char *addr) {
	int offset, size, i;	
	offset = (addr - base_addr) / MINALLOC;
	
	// 계산시 offset은 청크의 index 범위내에 존재하여야함 
	if (offset >= PAGESIZE/MINALLOC || offset < 0) {
		printf("%d\n", offset);
		return;
	}

	// 할당되지 않은 메모리를 초기화 할 경우
	if (mm[offset].used == 0) {
		return;
	}

	// chunk 초기화
	i = offset;
	size = 0;
	while (!mm[i].endpoint) {
		mm[i++].used = 0;
		size+=MINALLOC;
	}
	size+=MINALLOC;
	mm[i].used = 0;
	mm[i].endpoint = 0;

	printf("dealloc %d bytes\n", size);
	memset(addr, 0, size);
}
