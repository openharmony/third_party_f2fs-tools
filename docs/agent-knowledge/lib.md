# lib Agent Notes

`lib/` 是 libf2fs 共享库，提供 F2FS 工具的核心功能。独有实现包括日志、DMD 上报和额外 fsck 标志。

## 目录结构

| 路径 | 作用 |
| --- | --- |
| `BUILD.gn` | 构建 `libf2fs` 共享库，定义 `libf2fs-headers` config。 |
| `libf2fs.c` | 核心库函数：初始化、校验、bitmap 操作、segment 管理。包含 `WITH_OHOS` 条件编译分支。 |
| `libf2fs_io.c` | 设备 IO 操作：read、write、readahead、fsync、discard、zoned 设备 IO。 |
| `libf2fs_zoned.c` | zoned 设备支持：zone 报告、zone 重置、写指针管理。 |
| `libf2fs_log.c` | 【独有】日志系统实现：SlogInit、SlogWrite、KlogWrite、日志文件管理、大小控制、时间戳写入。 |
| `libf2fs_dmd.c` | 【独有】DMD 上报实现：DmdReport、DmdInsertError、DmdCheckCostTime、错误位图操作、HVB 状态读取。 |
| `extra_fsck.c` | 【独有】额外 fsck 标志：CheckExtraFlag、ClearExtraFlag，读取和清除 CP segment 最后一块的 needFsck 标志。 |
| `extra_fsck.h` | 【独有】额外 fsck 标志头文件：定义 `ExtraFlagsBlock` 结构和 `EXTRA_NEED_FSCK_FLAG`。 |
| `nls_utf8.c` | UTF8 NLS（National Language Support）实现。 |
| `utf8data.h` | UTF8 数据表（大文件，330KB）。 |

## 独有实现详解

### libf2fs_log.c

日志系统实现，提供 slog（文件日志）和 klog（内核日志）。

核心流程：
- `SlogInit(funcType)`：按功能类型（FSCK/DUMP/DEFRAG/RESIZE）选择日志文件名，创建 `/log/f2fs-tools/` 或 `/dev/f2fs-tools/` 目录，打开日志文件，写入时间戳头。
- `SlogWrite(fmt, ...)`：格式化日志内容，检查文件大小（256KB 限制），必要时 rename 为 `.1 备份，写入日志。
- `KlogWrite(level, fmt, ...)`：写入 `/dev/kmsg`，内核日志级别过滤。
- `SlogExit()`：fsync、ftruncate、关闭文件、释放内存。

关键常量：
- `LOG_MAX_SIZE = 0x40000`（256KB）
- `LOG_BUF_SIZE_PROTECT_THRESHOLD = 2048`

### libf2fs_dmd.c

DMD 上报实现，通过 ioctl 写入 `/dev/storage` 驱动。

核心流程：
- `DmdInsertError(type, err, func, line)`：设置错误位图 `errBitmap`，记录错误详情到 `g_dmdErrorStore[]`（最多 64 条），标记 `g_dmdMarkReport`。
- `DmdReport()`：填充消息 `FillReportMsg()`，读取 HVB 状态 `ReadDeviceState()`，打开 `/dev/storage`，执行 `ioctl(fd, EVENT_REPORT_FSCK_CMD, &g_dmdReport)`。
- `DmdCheckCostTime(func, line)`：按文件系统大小分级检查耗时，超限则插入 `PR_FSCK_TIME_OVERCOST` 错误。
- `ReadDeviceState()`：读取 `/proc/cmdline`，检查 `ohos.boot.hvb.device_state=locked`，设置 `FB_LOCKED_FL`。

关键常量：
- `HIEVENT_DRIVER_NODE = "/dev/storage"`
- `FAULT_STORE_SIZE = 64`
- `ASSERT_MSG_SIZE = 1536`
- 时间超限阈值按文件系统大小分级（512GB/1TB/2TB → 1s/3s/5s）

条件编译：
- `CONF_TARGET_HOST` 定义时，所有函数返回空实现。

### extra_fsck.c

额外 fsck 标志检查和清除。

`ExtraFlagsBlock` 结构：
- 位于 CP segment 最后一块。
- `needFsck`：标志位，值为 `0x4653434B`（"FSCK" ASCII）。
- `reserved[4088]`：预留空间。
- `crc`：CRC 校验（当前未使用）。

核心函数：
- `CheckExtraFlag(sb, flag)`：读取 CP segment 最后一块，检查 needFsck，若置位则设置 `c.fix_on = 1` 并上报 DMD。
- `ClearExtraFlag(sb, flag)`：清除 needFsck，写入并 fsync。

## 修改约束

- lib 是共享库，修改要检查 fsck、mkfs、tools 的 deps 和编译。
- 新增公共 API 必须同时提供 `CONF_TARGET_HOST` 条件编译的空实现。
- DMD 上报依赖 `/dev/storage` 驱动，修改要同步检查驱动侧兼容性。
- 日志文件大小限制为 256KB，修改阈值要评估磁盘空间。
- 安全函数使用 `securec.h` 的 `strncpy_s`、`vsnprintf_s`、`memset_s`。
- 新增源文件要同步 `BUILD.gn`。