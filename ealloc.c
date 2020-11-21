#include "ealloc.h"

typedef struct chunk {
	int used;
	int endpoint;
} CK;

typedef struct page {
	int size;
	int alloc;
	char *base_addr;
	CK mm[PAGESIZE/MINALLOC];
} PG;

#define PAGE_LEN 4

PG pg[PAGE_LEN];

void init_alloc() {
	memset(pg, 0, sizeof(PG) * PAGE_LEN);
}

void cleanup() {
	int i, j;
	for (i = 0; i < PAGE_LEN; i++) {
		if (pg[i].base_addr != NULL) {
			memset(pg[i].mm, 0, PAGESIZE/MINALLOC);
		}
	}
	memset(pg, 0, sizeof(PG) * PAGE_LEN);
}

char *alloc(int size) {
	int need_chunk, chunk_length, found, start, i, j;
	CK *mm;

	// 최소 8byte, 최대 4KB
	if (size < MINALLOC || size > PAGESIZE) {
		printf("memory size error\n");
		return NULL;
	}

	// 필요한 청크의 개수를 구함
	if (size % MINALLOC == 0) {
		need_chunk = size / MINALLOC;
	}
	// 나누어 떨어지지 않으면 에러임
	else {
		printf("chunk size error\n");
		return NULL;
	}

	for (i = 0; i < PAGE_LEN; i++) {
		if (pg[i].size < PAGESIZE) {
			/* 아직 메모리를 할당하지 않았으면 메모리 할당 */
			if (pg[i].alloc == 0) {
				if ((pg[i].base_addr = (char *)mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == NULL) {
					printf("nmap error\n");
					return NULL;
				}
				pg[i].alloc = 1;
			}
			mm = pg[i].mm;
			// 연속된 청크를 탐색 (FIRST FIT)
			for (j = 0, chunk_length = 0, found = -1, start = 0; j < PAGESIZE/MINALLOC; j++) {
				if (mm[j].used == 0) {
					if (chunk_length == 0) {
						start = j;
					}
					chunk_length++;
				} else {
					chunk_length = 0;
				}
				// 발견
				if (chunk_length == need_chunk) {
					found = start;
					mm[j].endpoint = 1;
					break;
				}
			}
			// 발견했으면 주소를 리턴
			if (found != -1) {
				j = found;
				while(mm[j].endpoint != 1) {
					mm[j++].used = 1;
				}
				mm[j].used = 1;
				pg[i].size += size;
				return pg[i].base_addr + found * MINALLOC;
			}
		}
	}
	printf("alloc error\n");
	return NULL;
}

void dealloc(char *addr) {
	int offset = -1, size, i, j;	
	// 해당 주소에 해당하는 페이지를 검색함
	for (i = 0; i < PAGE_LEN; i++) {
		if (pg[i].alloc == 1) {
			offset = (addr - pg[i].base_addr) / MINALLOC;
			if (offset < PAGESIZE/MINALLOC && offset > -1) {
				break;
			}
		}
	}

	// 잘못된 주소
	if (offset == -1) {
		return;
	}

	// 할당되지 않은 메모리를 초기화 할 경우
	if (pg[i].mm[offset].used == 0) {
		return;
	}

	// chunk 초기화
	j = offset;
	size = 1;
	while (!pg[i].mm[j].endpoint) {
		pg[i].mm[j++].used = 0;
		size++;
	}
	pg[i].mm[j].used = 0;
	pg[i].mm[j].endpoint = 0;
	pg[i].size -= (j + 1) * MINALLOC;

	memset(addr, 0, size);
}
