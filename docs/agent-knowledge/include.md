# include Agent Notes

`include/` 是公共头文件层。扩展头文件集中在 DMD、日志、扩展宏和错误码定义。

## 目录结构

| 路径 | 作用 |
| --- | --- |
| `f2fs_fs.h` | 核心数据结构定义：superblock、checkpoint、node、inode、sit、nat、summary 等。使用 `DMD_ASSERT_MSG` 宏。 |
| `f2fs_dmd.h` | DMD 上报结构 `DmdReport`、`DmdMsg`、`DmdFault`；错误位图操作宏 `DMD_SET_VALUE`、`DMD_ADD_ERROR`、`DMD_CHECK_COST_TIME`。 |
| `f2fs_dmd_errno.h` | DMD 错误码定义：`PR_INVALID_SUPER_BLOCK` 到 `PR_FSCK_TIME_OVERCOST`，共 80+ 错误类型。 |
| `f2fs_dmd_cfg.h` | DMD 配置宏：`DMD_ERR`、`DMD_OK`。 |
| `f2fs_dfx_common.h` | DFX 通用定义：`LogType` 枚举（FSCK/DUMP/DEFRAG/RESIZE/MKFS）、`UNUSED` 宏。 |
| `f2fs_log.h` | 日志系统：`LogInfo` 结构、`KLOGE`/`KLOGI`/`SLOG` 宏、日志级别定义。 |
| `f2fs_ext.h` | 扩展头文件，聚合 `f2fs_log.h` 和 `f2fs_dmd.h`，定义 `F2FS_EXT_EXIT()` 宏（组合 `SlogExit()` 和 `DmdReport()`）。 |
| `quota.h` | quota 结构定义。 |
| `android_config.h` | Android 环境兼容配置。 |

## 扩展头文件详解

### f2fs_dmd.h

DMD（Device Management Diagnostics）用于 fsck 过程中的故障诊断上报。

核心结构：
- `DmdReport`：上报数据包，包含错误位图 `errBitmap[3]`、属性位图 `propBitmap`、空间统计 `usedSpace`/`freeSpace`、checkpoint 版本、耗时 `costTime`、消息 `msg[1536]`。
- `DmdMsg`：消息缓冲辅助结构。
- `DmdFault`：错误详情存储结构。

核心宏：
- `DMD_SET_VALUE(field, value)`：设置上报字段。
- `DMD_ADD_ERROR(type, err)`：插入错误并标记上报。
- `DMD_ADD_MSG_ERROR(type, err, fmt, ...)`：插入错误并附加消息。
- `DMD_ASSERT_MSG(func, line, fmt, ...)`：记录断言消息。
- `DMD_CHECK_COST_TIME(sbi, costMs)`：检查耗时是否超限。
- `COMPUTE_SIZE(sbi)`：计算空间使用统计。

条件编译：
- `CONF_TARGET_HOST` 定义时，所有宏定义为空操作。

### f2fs_dmd_errno.h

定义 fsck 检查过程中的各类错误码，用于 DMD 上报。

主要类别：
- superblock/checkpoint 错误：`PR_INVALID_SUPER_BLOCK`、`PR_INVALID_CHECKPOINT`、`PR_POLLUTE_CHECKPOINT`。
- NAT/SIT 错误：`PR_INVALID_NAT_ENTRY0`、`PR_INVALID_SIT_VBLOCKS`、`PR_SIT_VBLOCKS_IS_ERROR`。
- 元数据一致性错误：`PR_FSCK_META_MISMATCH`、`PR_NAT_NODE_COUNT_MISMATCH_WITH_SIT`。
- 去重错误：`PR_RECORD_FSYNC_NODE_LOOP`、`PR_RECORD_FSYNC_WRONG_SIT_BITMAP`。
- 额外标志：`PR_EXTRA_NEED_FSCK_FLAG_SET`、`PR_FULL_DISK_FSCK`、`PR_PERMISSIVE_FSCK`。
- 时间超限：`PR_FSCK_TIME_OVERCOST`。

### f2fs_log.h

日志系统头文件，定义 slog（文件日志）和 klog（内核日志）。

核心结构：
- `LogInfo`：日志运行状态，包含日志类型、文件描述符、日志级别、文件路径。

核心宏：
- `KLOGE(fmt, ...)`：错误级别内核日志。
- `KLOGI(fmt, ...)`：信息级别内核日志。
- `SLOG(fmt, ...)`：文件日志写入。

核心函数：
- `SlogInit(funcType)`：初始化日志系统。
- `SlogExit()`：退出日志系统。
- `SlogWrite(fmt, ...)`：写入文件日志。
- `KlogWrite(level, fmt, ...)`：写入内核日志。

## 修改约束

- 新增 DMD 错误码必须追加到 `f2fs_dmd_errno.h`，不得复用已有错误码值。
- DMD 结构变更要同步检查 `lib/libf2fs_dmd.c` 实现和驱动侧 `EVENT_REPORT_FSCK_CMD` 处理。
- 日志文件名由 `InitLogInfo` 按日志类型选择，修改时同步检查 `g_logTag` 数组。
- `f2fs_fs.h` 是上游核心头文件，修改要评估与上游兼容性。