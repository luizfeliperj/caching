#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BLOCKSZ 64

struct linkedlist {
	int blockid;
	char data[BLOCKSZ];
	struct linkedlist *next;
} *global = NULL;

int mysave (struct linkedlist *, int, int, const char *);

int main (int argc, char ** argv)
{
	mysave (global, 100, 30, "012345678901234567890123456789");
	return 0;
}

int mysave (struct linkedlist *g, int offset, int len, const char *data) {
	int i,j;
	float blocks;
	struct linkedlist *pplist, *list = g;

	while (list) {
		if ((list->blockid * BLOCKSZ) > offset) break;
		list = list->next;
	}

	blocks = (offset%BLOCKSZ + len)/(float)BLOCKSZ;

	if (blocks == 0.0)
		return 0;

	printf ("%d %d %f\n", offset%BLOCKSZ, len, blocks);

	if (( blocks - (int)blocks) > 0.0) {
		i = (int)blocks + 1;
		printf ("case 1 [%d]\n", i);
	} else {
		i = (int)blocks;
		printf ("case 2 [%d]\n", i);
	}

	pplist = malloc ( ( (int)blocks ) * ( sizeof (struct linkedlist) ) );
	for (j = 0; j < i; j++) {
		pplist[j].blockid = 0;
		memset (pplist[j].data, 0, BLOCKSZ);
		pplist[j].next = &pplist[j+1];
	}
	pplist[i].next = NULL;

	if (list) {
		if (list->next)
			pplist[i].next = list->next;

		list->next = pplist;
	} else {
		list = pplist;
	}
}
