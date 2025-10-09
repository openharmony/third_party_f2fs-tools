/**
 * f2fs_tools.c
 *
 * Copyright (C) 2024 Huawei Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "f2fs_tools.h"
#include "f2fs_fs.h"
#include "../../fsck/fsck.h"

void f2fs_enable_large_nat_bitmap(struct f2fs_sb_info *sbi, unsigned long long total_size, unsigned long long cur_size)
{
    /* has large_nat_bitmap before */
    if (sbi != NULL) {
        struct f2fs_checkpoint *cp = F2FS_CKPT(sbi);
        u32 flags = get_cp(ckpt_flags);
        if (flags & CP_LARGE_NAT_BITMAP_FLAG) {
            c.large_nat_bitmap = 1;
        }
    }
    /* when in first resize */
    if (cur_size > (total_size >> 1)) {
        return;
    }

    if (total_size > F2FS_LARGE_NAT_BITMAP_MIN_SIZE) {
        c.large_nat_bitmap = 1;
    }
}
