# mkfs Agent Notes

`mkfs/` 是 mkfs.f2fs 格式化工具实现。

## 目录结构

| 路径 | 作用 |
| --- | --- |
| `BUILD.gn` | 构建 `mkfs.f2fs`，依赖 `libf2fs` 和 `e2fsprogs:libext2_uuid`、`e2fsprogs:libdacconfig`。 |
| `f2fs_format_main.c` | 入口：解析参数、调用格式化、处理返回值。 |
| `f2fs_format.c` | 格式化逻辑：配置 superblock、checkpoint、NAT、SIT、SSA、main area、root inode。包含 `ohos` 标记注释。 |
| `f2fs_format_utils.c` | 工具函数：写入各区域、初始化 block。 |
| `f2fs_format_utils.h` | 工具函数头文件。 |

## OpenHarmony 相关内容

`f2fs_format.c` 包含 `/* ohos */` 注释，标识 OpenHarmony 相关配置。

## 修改约束

- mkfs.f2fs 安装到 `system` 和 `updater` 镜像。
- 依赖 `libf2fs` 共享库，lib 修改要检查 mkfs 编译。
- 外部依赖 `e2fsprogs:libext2_uuid` 用于 UUID 生成。
- 外部依赖 `e2fsprogs:libdacconfig` 用于 DAC 配置。
- 新增源文件要同步 `BUILD.gn`。