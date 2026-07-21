# F2FS Tools 逻辑视图

本仓整体是 OpenHarmony C/GN 工程，核心分层如下：

```text
include/
  -> lib/
    -> fsck/
    -> mkfs/
    -> tools/
```

```plantuml
@startuml
title F2FS Tools Logical View

skinparam componentStyle rectangle
skinparam packageStyle rectangle

actor "OpenHarmony System" as system

package "Header Layer\ninclude" as hdr {
  component "f2fs_fs.h\nCore definitions\nF2FS structures" as core_hdr
  component "f2fs_dmd.h\nf2fs_dmd_errno.h\nDMD report" as dmd_hdr
  component "f2fs_log.h\nf2fs_dfx_common.h\nLog system" as log_hdr
  component "f2fs_ext.h\nExtension macros" as ext_hdr
  component "f2fs_dmd_cfg.h\nDMD config" as cfg_hdr
  component "quota.h\nQuota definitions" as quota_hdr
  component "android_config.h\nAndroid compat" as android_hdr
}

package "Library Layer\nlib" as lib {
  component "libf2fs.c\nCore library functions" as lib_core
  component "libf2fs_io.c\nDevice IO operations" as lib_io
  component "libf2fs_zoned.c\nZoned device support" as lib_zoned
  component "libf2fs_log.c\nSlog/Klog implementation" as lib_log
  component "libf2fs_dmd.c\nDMD report implementation" as lib_dmd
  component "extra_fsck.c\nExtra fsck flags" as lib_extra
  component "nls_utf8.c\nUTF8 NLS support" as lib_nls
}

package "FSCK Layer\nfsck" as fsck {
  component "fsck.c\nmain.c\nmount.c\nCore fsck logic" as fsck_core
  component "fsck_time.c\nTime statistics" as fsck_time
  component "dedup.c\nDeduplication check" as fsck_dedup
  component "queue.c\nAsync readahead queue" as fsck_queue
  component "node.c\ndir.c\nxattr.c\nsegment.c\nNode/Dir/Data handling" as fsck_node
  component "dump.c\ndefrag.c\nresize.c\nsload.c\ndump/defrag/resize/sload" as fsck_util
  component "quotaio.c\nmkquota.c\nQuota handling" as fsck_quota
  component "compress.c\nCompression support" as fsck_compress
}

package "MKFS Layer\nmkfs" as mkfs {
  component "f2fs_format.c\nf2fs_format_main.c\nf2fs_format_utils.c\nFormat implementation" as mkfs_core
}

package "Tools Layer\ntools" as tools {
  component "f2fscrypt.c\nsha512.c\nEncryption tool" as tools_crypt
  component "fibmap.c\nBlock mapping" as tools_fibmap
  component "f2fs_io/\nIO operations" as tools_io
  component "debug_tools/\nfsck_debug.c\nDebug helpers" as tools_debug
}

cloud "OpenHarmony external dependencies" as external {
  component "bounds_checking_function\nlibsec_shared" as sec
  component "e2fsprogs\nlibext2_uuid/libdacconfig" as e2fs
  component "/dev/storage driver\nDMD ioctl" as driver
  component "/dev/kmsg\nKernel log" as kmsg
  component "/proc/cmdline\nHVB device state" as cmdline
}

system --> fsck : fsck.f2fs\nresize.f2fs\nsload.f2fs
system --> mkfs : mkfs.f2fs
system --> tools : f2fscrypt\nfibmap.f2fs

fsck --> lib : libf2fs
mkfs --> lib : libf2fs
tools --> lib : libf2fs

lib --> hdr : include headers
fsck --> hdr : include headers
mkfs --> hdr : include headers

lib_log --> external : kmsg
lib_dmd --> external : driver
lib_dmd --> external : cmdline
lib --> external : sec
mkfs --> external : e2fs

@enduml
```

`include/` 定义核心数据结构、DMD 上报结构、日志系统、错误码和扩展宏。扩展头文件包括 `f2fs_dmd.h`、`f2fs_log.h`、`f2fs_ext.h`、`f2fs_dmd_errno.h`。

`lib/` 是 libf2fs 共享库，提供设备 IO、zoned 设备支持、日志实现、DMD 上报实现、额外 fsck 标志检查和 UTF8 NLS 支持。扩展实现包括 `libf2fs_log.c`、`libf2fs_dmd.c`、`extra_fsck.c`。

`fsck/` 是 fsck.f2fs 核心，提供文件系统检查修复、挂载元数据构建、去重检查、时间统计、异步预读队列、quota 处理、dump、defrag、resize、sload 等能力。扩展模块包括 `fsck_time.c`、`dedup.c`、`queue.c`。

`mkfs/` 是 mkfs.f2fs 格式化工具。

`tools/` 提供辅助工具：f2fscrypt（加密）、fibmap.f2fs（块映射）、f2fs_io（IO 操作）、debug_tools（调试辅助）。扩展模块包括 `debug_tools/fsck_debug.c`。