/**
 * fsck_debug.c
 *
 * Copyright (C) 2024 Huawei Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "fsck_debug.h"

void dump_sbi_info(struct f2fs_sb_info *sbi)
{
	if (sbi == NULL) {
		MSG(0, "sbi is null\n");
		return;
	}

	MSG(0, "\n");
	MSG(0, "+--------------------------------------------------------+\n");
	MSG(0, "| SBI                                                    |\n");
	MSG(0, "+--------------------------------------------------------+\n");
	MSG(0, "total_count %u\n", total_segments(sbi));
	MSG(0, "resvd_segs %u\n", reserved_segments(sbi));
	MSG(0, "overp_segs %u\n", overprov_segments(sbi));
	MSG(0, "valid_count %u\n", of_valid_block_count(sbi));
	MSG(0, "utilization %u\n", f2fs_utilization(sbi));
	MSG(0, "\n");
	hex_info_dump("f2fs_sb_info", sbi,
		sizeof(struct f2fs_sb_info));
	MSG(0, "\n");
}

#define LINE_MAX_LEN     80
#define LINE_MAX_INTS    16
#define BATCH_INTS       8
#define HEX_SHIFT_12     12
#define HEX_SHIFT_8      8
#define HEX_SHIFT_4      4
#define HEX_MASK         0x0F
void hex_info_dump(const char *prompts, const unsigned char *buf,
			unsigned int len)
{
	static const unsigned char hex_ascii[] = "0123456789abcdef";
	unsigned char line[LINE_MAX_LEN];
	unsigned int i, j, k, line_len;
	unsigned int rest = len;

	MSG(0, "===HEX DUMP START: %.25s, len %u===\n",
		prompts, len);
	for (i = 0; i < len; i += LINE_MAX_INTS) {
		line_len = rest > LINE_MAX_INTS ? LINE_MAX_INTS : rest;
		k = 0;
		line[k++] = hex_ascii[(i >> HEX_SHIFT_12) & HEX_MASK];
		line[k++] = hex_ascii[(i >> HEX_SHIFT_8) & HEX_MASK];
		line[k++] = hex_ascii[(i >> HEX_SHIFT_4) & HEX_MASK];
		line[k++] = hex_ascii[i & HEX_MASK];
		line[k++] = ':';
		for (j = 0; j < line_len; j++) {
			j % BATCH_INTS == 0 ? line[k++] = ' ' : 1;
			line[k++] = hex_ascii[(buf[i + j] >> HEX_SHIFT_4) & HEX_MASK];
			line[k++] = hex_ascii[(buf[i + j] & HEX_MASK)];
		}
		line[k++] = '\0';
		rest -= line_len;
		MSG(0, "%s\n", line);
	}
	MSG(0, "===HEX DUMP END===\n");
}