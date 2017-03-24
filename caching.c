#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define CACHEBLOCKSZ 64
#define MALLOCERROR -1

typedef struct st_linkedlist {
	uint16_t blockid;
	struct st_linkedlist *next;
	uint8_t data[CACHEBLOCKSZ + 1];
} linkedlist_t, *pLinkedList_t;

typedef struct st_filecache {
	uint32_t sz;
	linkedlist_t *list;
} filecache_t, *pFileCache_t;

filecache_t global = { 0 };

void filecache_free (pFileCache_t);
uint32_t filecache_pull (pFileCache_t, uint32_t, uint32_t, uint8_t *);
uint32_t filecache_push (pFileCache_t, uint32_t, uint32_t, const uint8_t *);
uint32_t linkedlist_store (pLinkedList_t *, uint32_t, uint32_t, const uint8_t*);

int main (int argc, char ** argv)
{
	uint8_t buffer[101] = { 0 };

	const uint8_t *data = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
	const uint32_t datalen = strlen(data);

	const uint8_t *data2 = "aaaaa111xxxAaAa";
	const uint32_t datalen2 = strlen(data2);

	filecache_push (&global, 100, datalen, data);
	filecache_push (&global, 300, datalen, data);

	filecache_push (&global, 500, datalen, data);

	filecache_push (&global, 101, 2, "XX");

	filecache_push (&global, 480, datalen2, data2);
	filecache_push (&global, 525, datalen2, data2);

	filecache_push (&global, 1000, datalen2, data2);
	filecache_push (&global, 105, 2, "XX");

	filecache_push (&global, 0, datalen2, data2);
	filecache_push (&global, 5, datalen2, data2);

	filecache_pull (&global, 5, 100, buffer);
	printf ("read: [%s]\n", buffer);

	filecache_push (&global, 808, 2, "XX");

	memset (buffer, 32, sizeof(buffer)-1);
	filecache_pull (&global, 800, 100, buffer);
	printf ("read: [%s]\n", buffer);

	memset (buffer, 32, sizeof(buffer)-1);
	filecache_pull (&global, 700, 100, buffer);
	printf ("read: [%s]\n", buffer);

	memset (buffer, 32, sizeof(buffer)-1);
	filecache_pull (&global, 128, 10, buffer);
	printf ("read: [%s]\n", buffer);

	memset (buffer, 32, sizeof(buffer)-1);
	filecache_pull (&global, 980, 100, buffer);
	printf ("read: [%s]\n", buffer);

	pLinkedList_t this = global.list;
	while (this) {
		printf ("block: %d, data: %s\n", this->blockid, this->data);
		this = this->next;
	}
	printf ("file sz is: %u\n", global.sz);

	filecache_free (&global);

	return 0;
}

void filecache_free (pFileCache_t cache) {
	pLinkedList_t this = cache->list, next;

	while (this != NULL) {
		next = this->next;
		free (this);
		this = next;
	}
}

uint32_t filecache_pull (pFileCache_t cache, uint32_t offset, uint32_t len, uint8_t *data) {
	uint16_t i;
	float blocks;
	uint32_t r = 0;
	
	blocks = ( offset % CACHEBLOCKSZ + len ) / (float) CACHEBLOCKSZ;

	if (blocks == 0.0)
		return 0;

	if (( blocks - (int) blocks) > 0.0)
		blocks = blocks + 1.0;

	for (i = 0; i < (uint16_t) blocks; i++) {
		uint16_t round;
		float relablock;
		linkedlist_t *leaf;
		uint32_t relaoffset, relalen;
		uint8_t * reladata = (uint8_t*) data;

		relalen = len - r;

		reladata = reladata + r;
		relaoffset = offset + r;

		round = CACHEBLOCKSZ - relaoffset%CACHEBLOCKSZ;
		if (relalen > round) relalen = round;

		for (leaf = cache->list; leaf && leaf->next; leaf = leaf->next) {
			if ( ( leaf->next->blockid * CACHEBLOCKSZ ) > relaoffset )
				break;
		}

		relablock = relaoffset/((float)CACHEBLOCKSZ) - leaf->blockid;

		if ( ( relablock >= 0 ) && ( relablock < 1 ) )
			memcpy (data + r, leaf->data + (relaoffset % CACHEBLOCKSZ), relalen);

		r = r + relalen;
	}

	return 0;
}

uint32_t filecache_push (pFileCache_t cache, uint32_t offset, uint32_t len, const uint8_t *data) {
	uint16_t i;
	float blocks;
	uint32_t r = 0;
	uint8_t updateroot = 0x1;

	blocks = ( offset % CACHEBLOCKSZ + len ) / (float) CACHEBLOCKSZ;

	if (blocks == 0.0)
		return 0;

	if (( blocks - (int) blocks) > 0.0)
		blocks = blocks + 1.0;

	for (i = 0; i < (uint16_t) blocks; i++) {
		uint16_t round;
		uint32_t localr;
		linkedlist_t *leaf;
		uint32_t relaoffset, relalen;
		uint8_t * reladata = (uint8_t*) data;

		relalen = len - r;

		reladata = reladata + r;
		relaoffset = offset + r;

		round = CACHEBLOCKSZ - relaoffset%CACHEBLOCKSZ;
		if (relalen > round) relalen = round;

		for (leaf = cache->list; leaf && leaf->next; leaf = leaf->next) {
			if ( ( leaf->next->blockid * CACHEBLOCKSZ ) > relaoffset )
				break;
			updateroot = 0x0;
		}

		localr = linkedlist_store(&leaf, relaoffset, (relalen > CACHEBLOCKSZ) ? CACHEBLOCKSZ : relalen, reladata);
		if (localr == MALLOCERROR)
			return MALLOCERROR;

		r = r + localr;

		if (updateroot & 0x1)
			cache->list = leaf;
	}

	if (offset + len > cache->sz)
		cache->sz = offset + len;

	return r;
}

uint32_t linkedlist_store (linkedlist_t **leaf, uint32_t offset, uint32_t len, const uint8_t *data) {
	linkedlist_t *block;
	uint16_t blockid = offset/CACHEBLOCKSZ;

	block = *leaf;
	if (!block || ( block->blockid != blockid ) ) {
		block = malloc ( sizeof( linkedlist_t ) );
		if (!block)
			return MALLOCERROR;

		memset (block->data, ' ', CACHEBLOCKSZ);
		block->data[CACHEBLOCKSZ]=0;
		block->blockid = blockid;
	}

	if (!*leaf) {
		block->next = NULL;
		*leaf = block;
	} else if (block != *leaf) {
		if (block->blockid > (*leaf)->blockid) {
			block->next = (*leaf)->next;
			(*leaf)->next = block;
		} else {
			block->next = (*leaf);
			(*leaf) = block;
		}
	}

	memcpy (block->data + offset%CACHEBLOCKSZ, data, len);

	return len;
}
