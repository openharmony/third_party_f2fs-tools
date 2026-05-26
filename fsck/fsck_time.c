/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * Description: enhance log records for fsck-tools
 * Create: 2026-4-30
 */

#include "fsck_time.h"
#include "securec.h"
#include <stdio.h>
#include <string.h>

#ifndef CONF_TARGET_HOST

struct fsck_time_stat g_time_stat;

void fsck_time_start_total()
{
    memset_s(&g_time_stat, sizeof(g_time_stat), 0, sizeof(g_time_stat));
    gettimeofday(&g_time_stat.start_time, NULL);
}

const char *fsck_time_get_phase_name(enum fsck_time_phase phase)
{
    static const char *phase_name[TIME_PHASE_MAX] = {
        [TIME_PHASE_MOUNT] = "DO_MOUNT",
        [TIME_PHASE_BUILD_NAT] = "BUILD_NAT",
        [TIME_PHASE_BUILD_SIT] = "BUILD_SIT",
        [TIME_PHASE_FSCK_INIT] = "FSCK_INIT",
        [TIME_PHASE_CHK_META] = "FSCK_META",
        [TIME_PHASE_CHK_QUOTA] = "FSCK_QUOTA",       /* check quota node and file */
        [TIME_PHASE_CHK_ORPHAN_NODE] = "FSCK_ORPHAN_NODE",
        [TIME_PHASE_CHK_FULL_FILE] = "FSCK_FULL_FILE",   /* recursive check for all files */
        [TIME_PHASE_FIX_DEDUP] = "FSCK_DEDUP",       /* check dedup inner node */
        [TIME_PHASE_FSCK_VERIFY] = "FSCK_VERIFY_CONSISTENCY",
        [TIME_PHASE_NODE_XATTR] = "FSCK_XATTR"
    };

    if (phase >= TIME_PHASE_MAX) {
        return "UNKNOWN";
    }

    return phase_name[phase];
}

void fsck_time_print_stat(void)
{
    enum fsck_time_phase phase;
    double total_time;

    MSG(3, "\n========= FSCK Time Statistics =========\n");

    total_time = fsck_time_get_total(&g_time_stat);
    MSG(3, "Total Time: %.2f ms (%.2f s)\n", total_time, total_time / TIME_UNIT);
    MSG(3, "------------------------------------------\n");

    for (phase = 0; phase < TIME_PHASE_MAX; phase++) {
        if (g_time_stat.phase_count[phase] > 0) {
            MSG(3, "%-25s: %10.2f ms [count: %u]\n", fsck_time_get_phase_name(phase), 
                g_time_stat.phase_time[phase], g_time_stat.phase_count[phase]);
        }
    }
    MSG(3, "==========================================\n");
}

void fsck_time_report_to_dmd(struct f2fs_sb_info *sbi)
{
    enum fsck_time_phase phase;
    double total_time;

    total_time = fsck_time_get_total(&g_time_stat);

    DMD_SET_VALUE(costTime, (unsigned long)total_time);

    PrintToMsg(&g_reportMsg, "TIME=[%.2fms ", total_time);

    for (phase = 0; phase < TIME_PHASE_MAX; phase++) {
        if (g_time_stat.phase_count[phase] > 0 && g_time_stat.phase_time[phase] > 0) {
            PrintToMsg(&g_reportMsg, "%.2fms", g_time_stat.phase_time[phase]);
            if (phase != TIME_PHASE_MAX - 1) {
                PrintToMsg(&g_reportMsg, " ");
            }
        }
    }
    PrintToMsg(&g_reportMsg, "|");
    PrintToMsg(&g_reportMsg, "%u %u %u]",
        sbi->total_valid_node_count, sbi->total_valid_inode_count, sbi->total_valid_block_count);
}
#endif

