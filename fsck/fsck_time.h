/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * Description: enhance log records for fsck-tools
 * Create: 2026-4-30
 */
#ifndef _FSCK_TIME_H_
#define _FSCK_TIME_H_

#include <time.h>
#include <sys/time.h>

#include "fsck.h"
#ifndef CONF_TARGET_HOST
#include "f2fs_dmd.h"
#endif

enum fsck_time_phase {
    TIME_PHASE_MOUNT = 0,
    TIME_PHASE_BUILD_NAT,
    TIME_PHASE_BUILD_SIT,
    TIME_PHASE_FSCK_INIT,
    TIME_PHASE_CHK_META,
    TIME_PHASE_CHK_QUOTA,       /* check quota node and file */
    TIME_PHASE_CHK_ORPHAN_NODE,
    TIME_PHASE_CHK_FULL_FILE,   /* recursive check for all files */
    TIME_PHASE_FIX_DEDUP,       /* check dedup inner node */
    TIME_PHASE_FSCK_VERIFY,
    TIME_PHASE_NODE_XATTR,
    TIME_PHASE_MAX
};

struct fsck_time_stat {
    struct timeval start_time;
    struct timeval phase_start[TIME_PHASE_MAX];
    double phase_time[TIME_PHASE_MAX]; /* total time */
    unsigned int phase_count[TIME_PHASE_MAX]; /* check count */
    bool in_phase[TIME_PHASE_MAX]; /* in phase tag process */
};

#ifndef CONF_TARGET_HOST
#define TIME_UNIT 1000.0
extern struct fsck_time_stat g_time_stat;

static inline double get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}

void fsck_time_start_total();

static inline int fsck_time_start_phase(int phase)
{
    if (phase >= TIME_PHASE_MAX) {
        return phase;
    }

    gettimeofday(&g_time_stat.phase_start[phase], NULL);
    g_time_stat.phase_count[phase]++;
    g_time_stat.in_phase[phase] = true;
    return phase;
}

static inline void fsck_time_end_phase(int phase)
{
    struct timeval end;
    double elapsed;

    if (phase >= TIME_PHASE_MAX || !g_time_stat.in_phase[phase]) {
        return;
    }

    gettimeofday(&end, NULL);
    elapsed = (double)(end.tv_sec - g_time_stat.phase_start[phase].tv_sec) * TIME_UNIT +
        (double)(end.tv_usec - g_time_stat.phase_start[phase].tv_usec) / TIME_UNIT;
    g_time_stat.phase_time[phase] += elapsed;
    g_time_stat.in_phase[phase] = false;
}

static inline void fsck_time_end_phase_in_cleanup(int *phase)
{
    fsck_time_end_phase(*phase);
}

static inline double fsck_time_get_total(struct fsck_time_stat *stat)
{
    struct timeval end;
    gettimeofday(&end, NULL);
    return (double)(end.tv_sec - stat->start_time.tv_sec) * TIME_UNIT +
        (double)(end.tv_usec - stat->start_time.tv_usec) / TIME_UNIT;
}

void fsck_time_print_stat(void);
const char *fsck_time_get_phase_name(enum fsck_time_phase phase);
void fsck_time_report_to_dmd(struct f2fs_sb_info *sbi);

static inline void report_fsck_phase(struct f2fs_sb_info *sbi)
{
    if (c.func == FSCK && (c.fix_on || c.bug_on)) {
        fsck_time_print_stat();
        fsck_time_report_to_dmd(sbi);
    }
}

#define TIME_TAG_POINT_START(PHASE_TYPE) fsck_time_start_phase(PHASE_TYPE)
#define TIME_TAG_POINT_END(PHASE_TYPE) fsck_time_end_phase(PHASE_TYPE)
#define TIME_TAG_POINT_WITH_END(PHASE_TYPE) \
    int phase __attribute__((cleanup(fsck_time_end_phase_in_cleanup))) = \
        fsck_time_start_phase(PHASE_TYPE)
#else

static inline void fsck_time_start_total()
{
    return;
}
static inline int fsck_time_start_phase(int phase)
{
    return 0;
}
static inline void fsck_time_end_phase(int phase)
{
    return;
}
static inline void fsck_time_start_phase_flag(enum fsck_time_phase phase, uint32_t nid)
{
    return;
}
static inline void fsck_time_end_phase_flag(enum fsck_time_phase phase)
{
    return;
}
static inline void report_fsck_phase(struct f2fs_sb_info *sbi)
{
    (void)sbi;
    return;
}
#define TIME_TAG_POINT_START(PHASE_TYPE) do {} while (0)
#define TIME_TAG_POINT_END(PHASE_TYPE) do {} while (0)
#define TIME_TAG_POINT_WITH_END(PHASE_TYPE) do {} while (0)
#endif

#endif /* _FSCK_TIME_H_ */