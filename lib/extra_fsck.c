/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * Description: enhance log records for fsck-tools
 * Create: 2024-3-4
*/
#include "extra_fsck.h"

void ClearExtraFlag(struct f2fs_super_block *sb, unsigned int flag)
{
    struct ExtraFlagsBlock *efBlk;
    unsigned long long cpBlkaddr;
    unsigned int blocksPerSeg;
    int ret;

    cpBlkaddr = le32_to_cpu(sb->cp_blkaddr);
    efBlk = calloc(F2FS_BLKSIZE, 1);
    if (!efBlk) {
        ERR_MSG("failed to alloc ExtraFlagsBlock\n");
        return;
    }
    blocksPerSeg = 1 << get_sb(log_blocks_per_seg);
    if (dev_read_block(efBlk, cpBlkaddr + blocksPerSeg - 1) < 0) {
        ERR_MSG("failed to read ExtraFlagsBlock\n");
        goto free;
    }

    switch (flag) {
        case EXTRA_NEED_FSCK_FLAG:
            if (!efBlk->needFsck) {
                goto free;
            }
            efBlk->needFsck = 0;
            break;
        default:
            ERR_MSG("unknown extra flag 0x%x\n", flag);
            goto free;
    }

    /* do not use crc for now */
    ret = dev_write_block(efBlk, cpBlkaddr + blocksPerSeg - 1);
    if (ret < 0) {
        ERR_MSG("failed to write ExtraFlagsBlock\n");
    }
    f2fs_fsync_device();
free:
    free(efBlk);
}

void CheckExtraFlag(struct f2fs_super_block *sb, unsigned int flag)
{
    struct ExtraFlagsBlock *efBlk;
    unsigned long long cpBlkaddr;
    unsigned int blocksPerSeg;

    cpBlkaddr = le32_to_cpu(sb->cp_blkaddr);
    efBlk = calloc(F2FS_BLKSIZE, 1);
    if (!efBlk) {
        return;
    }
    blocksPerSeg = 1 << get_sb(log_blocks_per_seg);
    if (dev_read_block(efBlk, cpBlkaddr + blocksPerSeg - 1) < 0) {
        goto free;
    }

    /* do not use crc for now */
    switch (flag) {
        case EXTRA_NEED_FSCK_FLAG:
            if (le32_to_cpu(efBlk->needFsck) == flag) {
                ASSERT_MSG("EXTRA_NEED_FSCK_FLAG is set");
                c.fix_on = 1;
                DMD_ADD_ERROR(LOG_TYP_FSCK, PR_EXTRA_NEED_FSCK_FLAG_SET);
            }
            break;
        default:
            break;
    }

free:
    free(efBlk);
}
