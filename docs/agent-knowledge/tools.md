# tools Agent Notes

`tools/` 提供辅助工具：加密、块映射、IO 操作、调试辅助。模块包括 `debug_tools/` 和 `f2fs_tools/`。

## 目录结构

| 路径 | 作用 |
| --- | --- |
| `BUILD.gn` | 构建 `f2fscrypt`、`fibmap.f2fs`。 |
| `f2fscrypt.c` | 加密工具：F2FS 文件加密配置。 |
| `sha512.c` | SHA512 实现（用于加密密钥）。 |
| `fibmap.c` | 块映射工具：文件块地址映射查询。 |
| `f2fs_io_parse.c` | IO 解析辅助。 |
| `f2fs_io/` | IO 操作工具：f2fs_io 命令实现。 |
| `f2fs_tools/` | 工具公共代码：`f2fs_tools.h`（压缩算法枚举）、`f2fs_tools.c`（`f2fs_enable_large_nat_bitmap()`）。 |
| `debug_tools/` | 调试辅助：`fsck_debug.c` 提供 dump_sbi_info、hex_info_dump、dump_bitmap_diff。 |

## 实现详解

### f2fs_tools/f2fs_tools.h

定义压缩算法枚举：

```c
enum compress_algorithm_type {
    COMPRESS_ALGO_LZO,
    COMPRESS_ALGO_LZ4,
    COMPRESS_ALGO_ZSTD,
    COMPRESS_ALGO_LZORLE,
    COMPRESS_ALGO_MAX,
};
```

### debug_tools/fsck_debug.c/h

调试辅助实现，用于 fsck 过程中的调试输出。

核心函数：
- `dump_sbi_info(sbi)`：输出 SBI 关键信息（total_count、resvd_segs、overp_segs、valid_count、utilization）和 hex dump。
- `hex_info_dump(prompts, buf, len)`：hex dump 输出，格式为 `===HEX DUMP START=== ... ===HEX DUMP END===`。
- `dump_bitmap_diff(sbi, sit_area_bitmap, main_area_bitmap)`：对比并输出 SIT bitmap 和 main bitmap 差异。

辅助函数：
- `total_segments(sbi)`：计算总 segment 数。
- `reserved_segments(sbi)`：获取预留 segment 数。
- `overprov_segments(sbi)`：获取超额预留 segment 数。
- `of_valid_block_count(sbi)`：获取有效 block 计数。
- `f2fs_utilization(sbi)`：计算利用率百分比。

## 修改约束

- f2fscrypt、fibmap.f2fs 只安装到 `system` 镜像。
- 新增调试工具要同步 `fsck/BUILD.gn`（fsck 编译时包含 `debug_tools/fsck_debug.c`）。
- f2fs_tools.h 定义公共类型，修改要检查使用方。
- 新增源文件要同步 `BUILD.gn`。