# F2FS Tools Agent Links

# Code map

本 AGENTS.md 适用于仓库根目录。子目录 `include/`、`lib/`、`fsck/`、`mkfs/`、`tools/` 的 agent 规则已统一迁移至 `docs/agent-knowledge/<子目录>.md`（include.md、lib.md、fsck.md、mkfs.md、tools.md），进入对应子目录工作时应先读对应规则。

本仓是 OpenHarmony third_party/f2fs-tools，基于上游 f2fs-tools v1.16.0，提供 F2FS 文件系统格式化、检查修复、调试和运维工具。新增 DMD、DFX 日志、去重检查、时间统计、预读队列、额外 fsck 标志等 OH 扩展特性，用于设备启动安全校验、故障诊断上报和性能分析。最重要的架构边界：libf2fs 共享库是公共 API 起点，fsck/mkfs/tools 依赖它——新增公共 API 必先改 lib 再改使用方。

Key areas:
- `include/`: 公共头文件，DMD/日志/错误码定义，API 兼容性敏感
- `lib/`: libf2fs 共享库实现，DMD/日志/额外标志，公共 API 变更必经路径
- `fsck/`: fsck.f2fs 核心，去重/时间统计/预读队列，高频修改
- `mkfs/`: mkfs.f2fs 格式化，低频但影响启动安全
- `tools/`: 辅助工具和 debug_tools

Where to look:
- fsck 检查修复变更 → `fsck/`，`docs/agent-knowledge/fsck.md`
- mkfs 格式化变更 → `mkfs/`，`docs/agent-knowledge/mkfs.md`
- DMD/日志/额外标志实现变更 → `lib/`，`docs/agent-knowledge/lib.md`
- DMD/日志/错误码定义变更 → `include/`，`docs/agent-knowledge/include.md`
- 辅助工具变更 → `tools/`，`docs/agent-knowledge/tools.md`
- 构建配置变更 → `BUILD.gn`、`bundle.json`
- 全仓架构理解 → `docs/agent-knowledge/logical-view.md`
- OH 扩展特性场景 → `docs/agent-knowledge/scenario-view.md`

# Knowledge routing

Before planning or editing, classify the task and read the matching documents.

### Task-based routing
- 全仓架构或模块边界变更 → `docs/agent-knowledge/logical-view.md`，再读相关子目录规则（`docs/agent-knowledge/<子目录>.md`）
- OH 扩展特性（DMD/去重/时间统计/额外标志）变更 → `docs/agent-knowledge/scenario-view.md`
- 编译或条件编译相关 → `docs/agent-knowledge/build-and-test.md`
- DMD 上报或错误码 → `docs/agent-knowledge/include.md`、`docs/agent-knowledge/lib.md`
- 日志系统 → `docs/agent-knowledge/include.md`、`docs/agent-knowledge/lib.md`
- 去重检查 → `docs/agent-knowledge/fsck.md`
- 时间统计 → `docs/agent-knowledge/fsck.md`
- 额外 fsck 标志 → `docs/agent-knowledge/lib.md`

### Path-based routing
- `include/` → `docs/agent-knowledge/include.md`（公共头文件、DMD 结构、日志宏、错误码定义）
- `lib/` → `docs/agent-knowledge/lib.md`（libf2fs 实现、DMD 上报、日志、额外标志）
- `fsck/` → `docs/agent-knowledge/fsck.md`（fsck 核心逻辑、去重、时间统计、预读队列）
- `mkfs/` → `docs/agent-knowledge/mkfs.md`（格式化工具）
- `tools/` → `docs/agent-knowledge/tools.md`（辅助工具、调试）

### Vocabulary-based routing

任务、issue、变更文件中出现以下术语时，先读对应文档再规划：

| 术语 | 风险提示 | 读 |
| --- | --- | --- |
| DMD / DmdReport | 设备诊断上报，依赖 /dev/storage 驱动，退出前必须调用 | `docs/agent-knowledge/include.md`、`docs/agent-knowledge/lib.md` |
| HVB / hvb.device_state | High Verification Boot 锁定状态，影响上报字段 | `docs/agent-knowledge/lib.md` |
| CONF_TARGET_HOST | 主机编译宏，设备端特性必须提供空实现隔离 | `docs/agent-knowledge/build-and-test.md` |
| Dedup / 去重 / F2FS_DEDUPED_FL | 去重 inode 检查修复，涉及链表维护和标志位 | `docs/agent-knowledge/fsck.md` |
| errBitmap / DMD_ADD_ERROR | 错误位图有容量限制（64条），超限丢弃 | `docs/agent-knowledge/include.md`、`docs/agent-knowledge/lib.md` |
| TIME_TAG_POINT / fsck_time_phase | 时间统计阶段枚举，修改要同步更新名称表 | `docs/agent-knowledge/fsck.md` |
| KLOGE / SLOG / SlogInit | 日志宏，敏感数据不可写入，热路径要限流 | `docs/agent-knowledge/include.md`、`docs/agent-knowledge/lib.md` |
| ExtraFlagsBlock / EXTRA_NEED_FSCK_FLAG | 额外 fsck 标志，清除后必须 fsync | `docs/agent-knowledge/lib.md` |
| WITH_OHOS | OH 环境标识，部分行为区分 Linux 和 OH | `docs/agent-knowledge/build-and-test.md` |

In the plan, state:
- task category
- documents read
- constraints found

# Constraints and boundaries

### Architecture/domain invariants
- GN/ohos.gni 是唯一构建路径，不使用 autotools；修改构建逻辑只改 BUILD.gn
- libf2fs 是共享库，fsck/mkfs/tools 依赖它；新增公共 API 必先改 lib 再改使用方
- CONF_TARGET_HOST 区分目标机和主机编译；主机编译禁用 DMD/时间统计等设备端特性
- DMD 上报依赖 /dev/storage 驱动和 ohos.boot.hvb.device_state cmdline 参数；主机环境不存在这些依赖，必须条件编译隔离
- DmdReport() 必须在 fsck 退出前调用，否则诊断数据不上报
- errBitmap 和 g_dmdErrorStore 有容量限制（64 条），超限丢弃并打印警告
- DMD 时间超限阈值按文件系统大小分级，修改阈值要同步更新 g_overtimeThresholdBySpace
- HVB 锁定状态通过读取 /proc/cmdline 检测，仅设备锁定时上报 FB_LOCKED_FL
- 去重 inode 有 4 种标志位（DEDUPED_FL / INNER_FL / REVOKE_FL / DOING_DEDUP_FL），f2fs_fsck 维护 dedup_inner_list_head 链表，修复时遍历并检查 actual_links 计数
- 去重检查属于 TIME_PHASE_FIX_DEDUP 阶段，统计打点必须正确
- ExtraFlagsBlock 位于 CP segment 最后一块，EXTRA_NEED_FSCK_FLAG 置位触发 DMD 上报并启用修复模式；清除后必须 fsync 设备确保落盘
- fsck_time_phase 枚举从 TIME_PHASE_MOUNT 到 TIME_PHASE_MAX，修改阶段要同步更新枚举和名称表
- TIME_TAG_POINT_WITH_END 使用 cleanup 属性自动结束阶段，避免遗漏
- ohos_executable/ohos_shared_library 必须声明 subsystem_name 和 part_name
- fsck.f2fs/mkfs.f2fs 安装到 system 和 updater，辅助工具只安装到 system

### Do not
- 不要使用裸 strcpy/sprintf，扩展代码必须用 securec.h 安全函数（strncpy_s、vsnprintf_s、memset_s）
- 不要顺手大重构；变更范围要小：接口+实现+BUILD+测试成组修改
- 不要混入无关格式化
- 不要绕过 CONF_TARGET_HOST 条件编译隔离；新增设备端特性必须同时提供空实现
- 不要复用 DMD 错误码值；新增错误码必须追加到 f2fs_dmd_errno.h
- 不要在热路径手动调用 gettimeofday；使用 TIME_TAG_POINT 宏
- 不要在日志中写入敏感数据；热路径日志要考虑 buffer 限流
- 不要修改 autotools 构建文件来改构建逻辑
- 不要绕过 DMD/日志/安全检查让测试通过
- 新增源文件不要遗漏对应 BUILD.gn 和使用方 deps

### Ask before
- 新增第三方依赖
- 修改 libf2fs 公共 API 签名或语义
- 修改 DMD 上报结构或错误码（涉及 /dev/storage 驱动侧兼容）
- 修改 HVB 锁定状态检测逻辑
- 修改 fsck 时间阶段枚举
- 修改额外 fsck 标志处理逻辑（涉及启动安全校验）
- 删除 CONF_TARGET_HOST 条件编译分支
- 执行影响真实设备的操作
