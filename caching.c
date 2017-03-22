#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define BLOCKSZ 64

struct linkedlist {
	uint16_t blockid;
	uint8_t data[BLOCKSZ];
	struct linkedlist *prev, *next;
} *global = NULL;

uint32_t mysave (struct linkedlist **, uint32_t, uint32_t, const uint8_t *);
uint32_t add_block (struct linkedlist **, uint32_t, uint32_t, const uint8_t*);

int main (int argc, char ** argv)
{
	const uint8_t *data = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
	const uint32_t datalen = strlen(data);

	mysave (&global, 100, datalen, data);

	mysave (&global, 300, datalen, data);

	mysave (&global, 500, datalen, data);

	while (global) {
		printf ("blockid: %d, data: %s\n", global->blockid, global->data);
		global = global->next;
	}

	return 0;
}

uint32_t mysave (struct linkedlist **root, uint32_t offset, uint32_t len, const uint8_t *data) {
	uint16_t i;
	float blocks;
	uint32_t r = 0;
	uint8_t updateroot = 0x1;

	blocks = ( offset % BLOCKSZ + len ) / (float) BLOCKSZ;

	if (blocks == 0.0)
		return 0;

	if (( blocks - (int) blocks) > 0.0)
		blocks = blocks + 1.0;

	for (i = 0; i < (uint16_t) blocks; i++) {
		uint32_t localr;
		uint32_t roffset, rlen;
		struct linkedlist *leaf;
		uint8_t * rdata = (uint8_t*) data;;

		rlen = len - r;
		if (i == 0) {
			int round = BLOCKSZ - offset%BLOCKSZ;
			if (rlen > round) rlen = round;
		}

		rdata = rdata + r;
		roffset = offset + r;

		leaf = *root;
		while (leaf && leaf->next) {
			if ( ( (leaf->next)->blockid * BLOCKSZ ) > roffset ) break;
			(leaf) = (leaf)->next;
			updateroot = 0x0;
		}


		localr = add_block(&leaf, roffset, (rlen > BLOCKSZ) ? BLOCKSZ : rlen, rdata);
		r = r + localr;

		if (updateroot & 0x1)
			*root = leaf;
	}

	return r;
}

uint32_t add_block (struct linkedlist **leaf, uint32_t offset, uint32_t len, const uint8_t *data) {
	struct linkedlist *block;

	printf ("%d %d\n", offset, len);

	block = malloc ( sizeof( struct linkedlist ) );

	if (!*leaf) {
		block->next = NULL;
		block->blockid = offset/BLOCKSZ;
		*leaf = block;
	} else {
		block->next = (*leaf)->next;
		block->blockid = offset/BLOCKSZ;
		(*leaf)->next = block;
	}

	memset (block->data, ' ', BLOCKSZ);
	memcpy (block->data + offset%BLOCKSZ, data, len);

	return len;
}
