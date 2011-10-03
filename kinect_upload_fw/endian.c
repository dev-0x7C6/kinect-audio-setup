#include <stdio.h>
#include <stdlib.h>

static int litend(void) {
	int i = 0;
	((char *) (&i))[0] = 1;
	return (i == 1);
}

static int bigend(void) {
	return !litend();
}

int main(void) {
	printf("#ifndef __ENDIAN_H\n");
	printf("#define __ENDIAN_H\n");
	printf("\n");
	printf("#define __LITTLE_ENDIAN 1234\n");
	printf("#define __BIG_ENDIAN    4321\n");
	printf("#define __BYTE_ORDER __%s_ENDIAN\n",
	       litend() ? "LITTLE" : "BIG");
	printf("\n");
	printf("#endif /* __ENDIAN_H */\n");
	exit(0);
}
