#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "alloc.h"

int main()
{
	if (init_alloc())
		return 1;

	char *str = alloc(2999);
	char *str2 = alloc(1097); 

	if (str == NULL) {
		printf("fail1\n");
		return 1;
	}
	
	if (str2 == NULL) {
		printf("fail2\n");
		return 1;
	}


	strcpy(str, "1");
	strcpy(str2, "2");
	printf("%s\n", str);
	printf("%s\n", str2);

	dealloc(str);
}
