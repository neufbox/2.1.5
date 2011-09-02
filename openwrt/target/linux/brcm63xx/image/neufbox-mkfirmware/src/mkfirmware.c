/*
 *      mkfirmware.c
 *      
 *      Copyright 2007 Miguel GAIO <miguel.gaio@efixo.com>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <zlib.h>
#include <getopt.h>
#include <unistd.h>
#include <stdint.h>
#include <libgen.h>

#include <sys/types.h>

#define  BCMTAG_EXE_USE
#include <bcm_hwdefs.h>
#include <bcmTag.h>
#include <box/board.h>
#include <box/partition.h>

size_t firmware_size(FILE * fp)
{
	long size;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	return size;
}

char const *strtoupper(char *s)
{
	char *p = s;

	while (*p) {
		*p = toupper(*p);
		++p;
	}

	return s;
}

uint32_t cc32(uint8_t * p, uint32_t size, uint32_t crc)
{
	while (size-- > 0)
		crc = (crc >> 8) ^ Crc32_table[(crc ^ *p++) & 0xff];

	return crc;
}

int main(int argc, char *argv[])
{
	char const *in;
	char *out;
	uint8_t *buffer;
	FILE *ifp, *ofp;
	size_t size;
	PFILE_TAG tag;
	uint32_t crc = CRC32_INIT_VALUE;

	if (argc < 4) {
		puts("usage: mkfirmware <in> <out> <net infra>");
	}

	in = argv[1];
	out = argv[2];

	ifp = fopen(in, "r");
	if (!ifp) {
		printf("fopen( %s, %s ) %m", in, "r");
		exit(EXIT_FAILURE);
	}
	ofp = fopen(out, "w");
	if (!ofp) {
		printf("fopen( %s, %s ) %m", out, "w");
		exit(EXIT_FAILURE);
	}

	/* set size */
	size = firmware_size(ifp);

	if ((buffer = malloc(size)) == NULL) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	if (fread(buffer, 1, size, ifp) != size) {
		perror("fread()");
		exit(EXIT_FAILURE);
	}

	/* Add Image Version */
	tag = (PFILE_TAG) buffer;
	strncpy((char *)tag->version, strtoupper(basename(out)),
		sizeof(tag->version));

	/* net infra */
	if (!strcmp(argv[3], "adsl")) {
		tag->net_infra[0] = NET_INFRA_ADSL | NET_INFRA_MAGIC;
	} else if (!strcmp(argv[3], "ftth")) {
		tag->net_infra[0] = NET_INFRA_FTTH | NET_INFRA_MAGIC;
	}

	crc = cc32((uint8_t *) tag, (uint32_t) TAG_LEN - TOKEN_LEN, crc);
	crc = htonl(crc);

	memcpy(tag->tagValidationToken, (unsigned char *)&crc, sizeof(CRC_LEN));

	fwrite(buffer, 1, size, ofp);

	fclose(ifp);
	fclose(ofp);

	printf("--\n");
	printf("firwmare: version: %s size %zu\n", out, size);

	/* Check size, we use filename to know if it's rescue or main image */
	if (strstr(out, "MAIN")) {
		if (size >= NEUFBOX_MAINFIRMWARE_SIZE) {
			fprintf(stderr,
				"*** No NO NO !! Your firmware is too big, max size allowed is %d !"
				"\nTry again !\n", NEUFBOX_MAINFIRMWARE_SIZE);
			return -1;
		}
	} else if (strstr(out, "RESCUE")) {
		if (size >= NEUFBOX_RESCUEFIRMWARE_SIZE) {
			fprintf(stderr,
				"*** No NO NO !! Your firmware is too big, max size allowed is %d !"
				"\nTry again !\n", NEUFBOX_RESCUEFIRMWARE_SIZE);
			return -1;
		}
	} else {
		fprintf(stderr,
			"*** Couldn't determine the kind of firmware (MAIN or RESCUE) by looking at filename.\n"
			"What's wrong ?\n");
		return -1;
	}

	return 0;
}
