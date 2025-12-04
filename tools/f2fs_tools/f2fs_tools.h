/**
 * f2fs_tools.h
 *
 * Copyright (C) 2024 Huawei Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _F2FS_TOOLS_H_
#define _F2FS_TOOLS_H_

#include <stdbool.h>

enum compress_algorithm_type {
    COMPRESS_ALGO_LZO,
    COMPRESS_ALGO_LZ4,
    COMPRESS_ALGO_ZSTD,
    COMPRESS_ALGO_LZORLE,
    COMPRESS_ALGO_MAX,
};

#endif /* _F2FS_TOOLS_H_ */
