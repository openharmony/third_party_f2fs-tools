# fsck Agent Notes

`fsck/` 是 fsck.f2fs 核心实现，提供 F2FS 文件系统检查修复能力。扩展模块包括时间统计、去重检查、异步预读队列。

## 知识路由

| 想了解的内容 | 详细文档 |
| --- | --- |
| fsck 检查修复、关键函数、数据结构 | 本文档 `fsck 检查修复流程` |
| resize 扩容缩容、safe resize | 本文档 `resize 扩容缩容流程` |
| dedup 去重机制、检查修复流程 | 本文档 `dedup 去重检查修复` |
| 时间统计实现 | 本文档 `扩展实现 > fsck_time.c/h` |
| 异步预读队列实现 | 本文档 `扩展实现 > queue.c/h` |

## 目录结构

| 路径 | 作用 |
| --- | --- |
| `BUILD.gn` | 构建 `fsck.f2fs` 及 symlink，含扩展源文件 |
| `main.c` | 入口：`main()` 解析参数、调用 fsck、`SlogInit`/`SlogExit`/`DmdReport` |
| `fsck.c` | 核心检查：`fsck_chk_meta`、`fsck_chk_node_blk`、`fsck_chk_data_blk`、`fsck_chk_dentry_blk`、`fsck_verify` |
| `fsck.h` | 结构定义：`f2fs_fsck`、`child_info`（含去重标记）、`hard_link_node`、`dedup_inner_node` |
| `mount.c` | 挂载：`f2fs_do_mount`、`f2fs_do_umount`、NAT/SIT 构建、DMD 字段设置 |
| `fsck_time.c` | 时间统计 |
| `fsck_time.h` | `fsck_time_phase` 枚举、`TIME_TAG_POINT_START/END/WITH_END` 宏 |
| `dedup.c` | 去重检查 |
| `dedup.h` | 去重标志位 `F2FS_DEDUPED_FL` 等、`dedup_inner_node` 结构 |
| `queue.c` | 异步预读队列 |
| `queue.h` | `ra_work` 结构、sum cache 结构 |
| `node.c`/`node.h` | node 块处理 |
| `dir.c` | 目录项处理 |
| `xattr.c`/`xattr.h` | 扩展属性处理 |
| `segment.c` | segment 管理 |
| `dump.c` | NAT/SIT/SSA dump |
| `defrag.c` | 碎片整理 |
| `resize.c` | 文件系统大小调整，入口 `f2fs_resize()`，扩容 `f2fs_resize_grow()`，缩容 `f2fs_resize_shrink()`，safe resize `revert_old_fs_layout()` |
| `sload.c` | 加载文件到文件系统 |
| `compress.c`/`compress.h` | 压缩块检查 |
| `dict.c`/`dict.h` | 字典数据结构 |
| quota 相关文件 | quota 处理 |

## 扩展实现

### fsck_time.c/h

时间统计，统计各阶段耗时并上报 DMD。

- 宏：`TIME_TAG_POINT_START(PHASE)`、`TIME_TAG_POINT_END(PHASE)`、`TIME_TAG_POINT_WITH_END(PHASE)`
- 阶段：MOUNT、BUILD_NAT、BUILD_SIT、FSCK_INIT、CHK_META、CHK_QUOTA、CHK_ORPHAN_NODE、CHK_FULL_FILE、FIX_DEDUP、FSCK_VERIFY、NODE_XATTR

### dedup.c/h

去重 inode 检查和修复。

- 入口函数：`f2fs_is_deduped_inode()`、`f2fs_is_inner_inode()`、`f2fs_fix_dedup_inner_list()`、`check_dedup_data_blkaddr()`

### queue.c/h

异步预读队列，提高读取性能。

- 入口函数：`init_reada_queue()`、`queue_reada_block()`、`build_sum_cache_list()`
- 条件编译：`POSIX_FADV_WILLNEED` 不存在时降级为 `dev_reada_block`

## fsck 检查修复流程

### 核心流程

```text
main()
  -> f2fs_do_mount()              // 挂载、构建元数据
  -> do_fsck()
    -> fsck_init()                // 初始化 bitmap
    -> fsck_chk_checkpoint()      // checkpoint 检查
    -> fsck_chk_quota_node()
    -> fsck_chk_orphan_node()
    -> fsck_chk_node_blk()        // 从 root inode 递归检查
    -> f2fs_fix_dedup_inner_list()// 去重修复（扩展）
    -> fsck_chk_quota_files()
    -> fsck_verify()              // 一致性验证
  -> F2FS_EXT_EXIT()              // SlogExit + DMD 上报（扩展）
```

### 递归检查

```text
fsck_chk_node_blk()
  -> sanity_check_nid()
  -> TYPE_INODE: fsck_chk_inode_blk()        // 检查 i_links/inline/xattr/extent，去重 inode
  -> TYPE_DIRECT_NODE: fsck_chk_dnode_blk()
  -> TYPE_INDIRECT_NODE: fsck_chk_idnode_blk()
  -> TYPE_DOUBLE_INDIRECT_NODE: fsck_chk_didnode_blk()
```

### 一致性验证（fsck_verify）
- write pointer（zoned 设备）、unreachable NIDs、硬链接链表
- 对比 SIT bitmap 与 main area bitmap
- valid_block/node/inode count 与 CP 对齐

### 修复阶段
硬链接 i_links、NAT entries、checkpoint、checksum；扩展：清除额外 fsck 标志（`ClearExtraFlag`）。

## resize 扩容缩容流程

入口 `resize.c:f2fs_resize()`。参数：`-s` 启用safe resize，`-t <sectors>` 指定目标大小。

### 核心流程

```text
f2fs_resize()
  -> 扩容: f2fs_resize_grow()
    -> flush_journal_entries -> get_new_sb -> f2fs_resize_check
    -> shrink_nats(如必要) -> revert_old_fs_layout(safe resize) -> f2fs_defragment
    -> update_superblock -> rebuild_checkpoint
  -> 缩容: f2fs_resize_shrink()
    -> flush_journal_entries -> get_new_sb -> f2fs_resize_check
    -> 检查数据是否超出新边界 -> f2fs_defragment(迁移越界数据)
    -> update_superblock -> rebuild_checkpoint
```

### 关键函数

| 函数 | 作用 |
| --- | --- |
| `get_new_sb()` | 按目标大小计算新 superblock 参数 |
| `f2fs_resize_check()` | 检查 resize 合法性（valid_block_count、main area 空间） |
| `revert_old_fs_layout()` | 扩展：保持 SIT/NAT/SSA 地址不变，仅扩展 main segment |
| `rebuild_checkpoint()` | 重建 checkpoint，递增版本号 |
| `f2fs_defragment()` | 迁移超出边界的数据块 |

### safe resize (-s)
保持元数据布局不变（SIT/NAT/SSA 地址不变），仅扩展 main segment count，受限于 SIT/SSA 最大覆盖范围，适用于 OTA 场景，避免大量数据迁移。

约束：resize 前文件系统须 clean；缩容须检查越界数据；safe resize 新大小受 SIT/SSA 覆盖能力限制；checkpoint 版本须递增；写入后须 fsync。

## dedup 去重检查修复

### 去重 inode 类型

| 标志位 | 说明 |
| --- | --- |
| `F2FS_DEDUPED_FL` | 去重 inode |
| `F2FS_INNER_FL` | 内部 inode（存实际数据，被多个输出 inode 共享） |
| `F2FS_REVOKE_FL` | 撤销标志 |
| `F2FS_DOING_DEDUP_FL` | 正在进行去重 |

关系：输出 inode（`F2FS_DEDUPED_FL`）的 `i_inner_ino` 指向内部 inode（`DEDUPED_FL|INNER_FL`）；输出 inode 数据块为 `DEDUP_ADDR`，内部 inode 的 `i_links` 为引用次数。

### 检查流程

```text
fsck_chk_inode_blk()
  -> f2fs_is_deduped_inode / f2fs_is_inner_inode / f2fs_is_out_inode
  -> 输出 inode: f2fs_inc_inner_actual_links()          // 增加引用计数
  -> 内部 inode: f2fs_sanity_check_dedup_inner_nid()    // 检查合法性
  -> check_dedup_data_blkaddr()                          // 检查 DEDUP_ADDR/NULL_ADDR
```

### 修复流程

```text
f2fs_fix_dedup_inner_list()
  -> TIME_TAG_POINT_WITH_END(TIME_PHASE_FIX_DEDUP)
  -> 遍历 dedup_inner_list_head
  -> actual_links==0: drop_node_blk()    // 删除无引用内部 inode
  -> links!=actual_links: 修复 i_links   // 修复链接计数
```

关键函数：`f2fs_is_deduped_inode`、`f2fs_is_inner_inode`、`f2fs_is_out_inode`、`f2fs_sanity_check_dedup_inner_nid`、`f2fs_inc_inner_actual_links`、`check_dedup_data_blkaddr`、`f2fs_fix_dedup_inner_list`。

约束：新增去重检查逻辑须在 `dedup.c`；修复时须正确处理 `actual_links`；删除内部 inode 须递归删除所有数据块。

## 修改约束

- 扩展源文件必须在 `BUILD.gn` 中包含
- 时间统计修改要同步更新 `fsck_time_phase` 枚举和名称表
- 去重检查修改要同步 `dedup_inner_list_head` 链表和 `f2fs_fsck` 结构
- 预读队列修改要检查 `POSIX_FADV_WILLNEED` 条件编译
- `mount.c` 中 `DMD_SET_VALUE` 要与 `f2fs_dmd.h` 字段对应
- `WITH_OHOS` 条件编译分支要同步检查
- 新增源文件要同步 `BUILD.gn`
