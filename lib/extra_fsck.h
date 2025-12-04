/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
 * Description: enhance log records for fsck-tools
 * Create: 2024-3-4
*/
#ifndef EXTRA_FSCK_H_
#define EXTRA_FSCK_H_

#include <f2fs_fs.h>
#include <linux/types.h>

/* extra_flags is placed at the last block of CP 0 segment */
struct ExtraFlagsBlock {
    __le32 needFsck;
    __u8 reserved[4088];
    __le32 crc;
} __attribute__((packed));

#define EXTRA_NEED_FSCK_FLAG    0x4653434b      // ascii of "FSCK"

void ClearExtraFlag(struct f2fs_super_block *sb, unsigned int flag);
void CheckExtraFlag(struct f2fs_super_block *sb, unsigned int flag);

#endif
