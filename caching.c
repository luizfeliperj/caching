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
	const uint8_t *data2 = "aaaaa";
	const uint32_t datalen = strlen(data);
	const uint32_t datalen2 = strlen(data2);

	mysave (&global, 100, datalen, data);

	mysave (&global, 300, datalen, data);

	mysave (&global, 500, datalen, data);

	mysave (&global, 100, datalen2, data2);

	mysave (&global, 480, datalen2, data2);
	mysave (&global, 525, datalen2, data2);

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
	struct linkedlist *leaf;
	uint8_t updateroot = 0x1;

	blocks = ( offset % BLOCKSZ + len ) / (float) BLOCKSZ;

	if (blocks == 0.0)
		return 0;

	if (( blocks - (int) blocks) > 0.0)
		blocks = blocks + 1.0;

	for (i = 0; i < (uint16_t) blocks; i++) {
		uint32_t localr;
		uint32_t roffset, rlen;
		uint8_t * rdata = (uint8_t*) data;;

		rlen = len - r;
		if (i == 0) {
			int round = BLOCKSZ - offset%BLOCKSZ;
			if (rlen > round) rlen = round;
		}

		rdata = rdata + r;
		roffset = offset + r;

		for (leaf = *root; leaf && leaf->next; leaf = leaf->next) {
			if ( ( leaf->next->blockid * BLOCKSZ ) >= roffset )
				break;
			updateroot = 0x0;
		}


		localr = add_block(&leaf, roffset, (rlen > BLOCKSZ) ? BLOCKSZ : rlen, rdata);
		r = r + localr;

		if (updateroot & 0x1)
			*root = leaf;
	}

/*
	for ( leaf = *root; leaf->next; leaf = leaf->next ) {
		struct linkedlist *clean;

		if ( leaf->blockid != leaf->next->blockid )
			continue;

		clean = leaf->next;

		memcpy ( leaf, leaf->next, sizeof (struct linkedlist) );

		free ( clean );
	}
*/

	return r;
}

uint32_t add_block (struct linkedlist **leaf, uint32_t offset, uint32_t len, const uint8_t *data) {
	struct linkedlist *block;
	uint16_t blockid = offset/BLOCKSZ;

	printf ("%d %d\n", offset, len);

	block = *leaf;
	if (!block || ( block->blockid != blockid ) ) {
		block = malloc ( sizeof( struct linkedlist ) );
		memset (block->data, ' ', BLOCKSZ);
		block->blockid = blockid;
	}

	if (!*leaf) {
		block->next = NULL;
		*leaf = block;
	} else if (block != *leaf) {
		block->next = (*leaf)->next;
		(*leaf)->next = block;
	}

	memcpy (block->data + offset%BLOCKSZ, data, len);

	return len;
}
