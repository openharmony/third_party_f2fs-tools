# F2FS Tools 场景视图

## fsck 检查修复

入口：`fsck/main.c:main()`

| 功能 | 入口函数 | 文件 |
| --- | --- | --- |
| 挂载和元数据构建 | `f2fs_do_mount()` | `fsck/mount.c` |
| fsck 初始化 | `fsck_init()` | `fsck/fsck.c` |
| 元数据检查 | `fsck_chk_meta()`、`fsck_chk_checkpoint()` | `fsck/fsck.c` |
| quota 检查 | `fsck_chk_quota_node()`、`fsck_chk_quota_files()` | `fsck/fsck.c` |
| orphan inode 检查 | `fsck_chk_orphan_node()` | `fsck/fsck.c` |
| 递归文件检查 | `fsck_chk_node_blk()`、`fsck_chk_inode_blk()`、`fsck_chk_dnode_blk()`、`fsck_chk_data_blk()`、`fsck_chk_dentry_blk()` | `fsck/fsck.c` |
| 去重修复 | `f2fs_fix_dedup_inner_list()` | `fsck/dedup.c` |
| 一致性验证 | `fsck_verify()` | `fsck/fsck.c` |
| DMD 上报 | `DmdReport()` | `lib/libf2fs_dmd.c` |
| 时间统计 | `fsck_time_print_stat()`、`fsck_time_report_to_dmd()` | `fsck/fsck_time.c` |
| 日志 | `SlogInit()`/`SlogExit()`/`SlogWrite()` | `lib/libf2fs_log.c` |
| 额外标志检查 | `CheckExtraFlag()`、`ClearExtraFlag()` | `lib/extra_fsck.c` |
| 调试辅助 | `dump_sbi_info()`、`hex_info_dump()`、`dump_bitmap_diff()` | `tools/debug_tools/fsck_debug.c` |

时间阶段：MOUNT、BUILD_NAT、BUILD_SIT、FSCK_INIT、CHK_META、CHK_QUOTA、CHK_ORPHAN_NODE、CHK_FULL_FILE、FIX_DEDUP、FSCK_VERIFY、NODE_XATTR

## DMD 上报

入口：`lib/libf2fs_dmd.c`

| 功能 | 入口函数 |
| --- | --- |
| 错误插入 | `DmdInsertError()` → `SetErrBitmap()`、`g_dmdErrorStore[]`（最多64条） |
| 字段设置 | `DMD_SET_VALUE(field, value)` |
| 耗时检查 | `DmdCheckCostTime()` |
| HVB 状态读取 | `ReadDeviceState()` → 读取 `/proc/cmdline` 检查 locked |
| 上报写入 | `DmdReport()` → `FillReportMsg()` → ioctl `/dev/storage` |

关键文件：`include/f2fs_dmd.h`、`include/f2fs_dmd_errno.h`、`include/f2fs_dmd_cfg.h`、`include/f2fs_ext.h`、`lib/libf2fs_dmd.c`

仅在设备端生效，主机编译时宏定义为空操作。

## 去重（Dedup）检查

入口：`fsck/dedup.c`

| 功能 | 入口函数 |
| --- | --- |
| 去重 inode 识别 | `f2fs_is_deduped_inode()`、`f2fs_is_inner_inode()` |
| 内部 inode 校验 | `f2fs_sanity_check_dedup_inner_nid()` |
| 数据块地址检查 | `check_dedup_data_blkaddr()` |
| 链表修复 | `f2fs_fix_dedup_inner_list()` → 遍历 `dedup_inner_list_head` |

标志位：`F2FS_DEDUPED_FL`、`F2FS_INNER_FL`、`F2FS_REVOKE_FL`、`F2FS_DOING_DEDUP_FL`

## 时间统计

入口：`fsck/fsck_time.c`

| 功能 | 入口函数 |
| --- | --- |
| 开始计时 | `TIME_TAG_POINT_START(PHASE)` → `fsck_time_start_phase()` |
| 结束计时 | `TIME_TAG_POINT_END(PHASE)` → `fsck_time_end_phase()` |
| 自动结束 | `TIME_TAG_POINT_WITH_END(PHASE)`（cleanup属性） |
| 统计输出 | `fsck_time_print_stat()`、`fsck_time_report_to_dmd()` |

仅在设备端生效，主机编译时宏定义为空操作。

## 额外 FSCK 标志

入口：`lib/extra_fsck.c`

| 功能 | 入口函数 |
| --- | --- |
| 标志检查 | `CheckExtraFlag()` → 读取 CP 最后一块，若置位则 `c.fix_on=1` 并 DMD 上报 |
| 标志清除 | `ClearExtraFlag()` → 清除 needFsck → `dev_write_block()` → `f2fs_fsync_device()` |

标志值：`EXTRA_NEED_FSCK_FLAG = 0x4653434B` ("FSCK" ASCII)

## mkfs 格式化

入口：`mkfs/f2fs_format_main.c:main()`

| 功能 | 入口函数 | 文件 |
| --- | --- | --- |
| 格式化 | `f2fs_format_device()` | `mkfs/f2fs_format.c` |
| 写入区域 | 工具函数 | `mkfs/f2fs_format_utils.c` |

## 日志系统

入口：`lib/libf2fs_log.c`

| 功能 | 入口函数 |
| --- | --- |
| 初始化 | `SlogInit(funcType)` → 选择日志文件名、创建目录、写入时间戳头 |
| 文件日志 | `SlogWrite()` → 格式化、检查大小(256KB)、写入 |
| 内核日志 | `KlogWrite()` → writev `/dev/kmsg` |
| 退出 | `SlogExit()` → fsync、ftruncate、close |

关键文件：`include/f2fs_log.h`、`include/f2fs_dfx_common.h`、`lib/libf2fs_log.c`
