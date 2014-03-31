/**
 * topswap
 * Copyright (C) 2014 Ng Tzy Luen. All Rights Reserved.
 */
#ifndef TOPSWAP_H
#define TOPSWAP_H

#define PROC_DIR_PATH           "/proc"
#define PROC_PID_STAT_FILE      "status"
#define MAX_PROC_LIST_SIZE      100

#define ANSI_COLOR_INVERT       "\x1b[7m"
#define ANSI_COLOR_RED          "\x1b[31m"
#define ANSI_COLOR_GREEN        "\x1b[32m"
#define ANSI_COLOR_YELLOW       "\x1b[33m"
#define ANSI_COLOR_BLUE         "\x1b[34m"
#define ANSI_COLOR_MAGENTA      "\x1b[35m"
#define ANSI_COLOR_CYAN         "\x1b[36m"
#define ANSI_COLOR_RESET        "\x1b[0m"

typedef struct {
    char *Name;
    char *Pid;
    char *PPid;
    char *State;
    char *Uid;
    char *VmSwap;
} t_vmswap_info;

static size_t lookup(const char *, const char *arg, t_vmswap_info **);
static int parse_proc_status_file(const char *, t_vmswap_info *);
static const char *get_full_cmd_arg(char *);
static void fmt_display(t_vmswap_info **, size_t elem);
static char *ltrim(char *);
static char *rtrim(char *);
static char *trim(char *);

#endif
