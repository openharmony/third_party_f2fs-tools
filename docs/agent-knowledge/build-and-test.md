# F2FS Tools 编译和测试

本仓是 OpenHarmony third_party 子仓，使用 GN/ohos.gni 构建，不使用传统 autotools 主路径。

## 常用目标

| 目标 | 用途 |
| --- | --- |
| `f2fs-tools` | 聚合目标，构建 libf2fs、fsck.f2fs、mkfs.f2fs。 |
| `f2fs-tools_host_toolchain` | 主机工具链构建，用于主机端格式化和检查。 |
| `//third_party/f2fs-tools/lib:libf2fs` | 构建 libf2fs 共享库。 |
| `//third_party/f2fs-tools/fsck:fsck.f2fs` | 构建 fsck.f2fs 及 symlink。 |
| `//third_party/f2fs-tools/mkfs:mkfs.f2fs` | 构建 mkfs.f2fs。 |
| `//third_party/f2fs-tools/tools:f2fscrypt` | 构建加密工具。 |
| `//third_party/f2fs-tools/tools:fibmap.f2fs` | 构建块映射工具。 |

## 示例命令

```bash
# 在 OpenHarmony 源码根目录执行
./build.sh --product-name <product> --build-target f2fs-tools
./build.sh --product-name <product> --build-target f2fs-tools_host_toolchain
./build.sh --product-name <product> --build-target //third_party/f2fs-tools/fsck:fsck.f2fs
```

## 条件编译

| 宏 | 作用 |
| --- | --- |
| `CONF_TARGET_HOST` | 主机工具链编译时定义，禁用设备端特性（DMD、时间统计、预读队列）。 |
| `WITH_OHOS` | OpenHarmony 环境标识，部分代码使用该宏区分 Linux 原生和 OpenHarmony 行为。 |
| `POSIX_FADV_WILLNEED` | 预读队列依赖该宏，不存在时预读队列降级为简单 dev_reada_block。 |

## 外部依赖

| 依赖 | 用途 |
| --- | --- |
| `bounds_checking_function:libsec_shared` | 安全函数库（strncpy_s、vsnprintf_s 等）。 |
| `e2fsprogs:libext2_uuid` | UUID 生成。 |
| `e2fsprogs:libdacconfig` | DAC 配置。 |

## 安装位置

| 工具 | 安装镜像 |
| --- | --- |
| fsck.f2fs、mkfs.f2fs | `system`、`updater` |
| f2fscrypt、fibmap.f2fs | `system` |

## 传统 autotools 构建（非主路径）

```bash
./autogen.sh
./configure
make
make install
```

说明：OpenHarmony 环境下不使用 autotools，仅在需要传统 Linux 构建时参考。