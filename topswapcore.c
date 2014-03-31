/**
 * topswapcore
 * Copyright (C) 2014 Ng Tzy Luen. All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <inttypes.h>
#include <sys/sysctl.h>
#include "topswap.h"


int main(int argc, char **argv)
{
    t_vmswap_info **pp_vmswap_info =
            (t_vmswap_info **) malloc(MAX_PROC_LIST_SIZE * sizeof(t_vmswap_info *));
    if (pp_vmswap_info == NULL) {
        fprintf(stderr, "-E-: failed to allocate memory");
        return -ENOMEM;
    }

    size_t elem = lookup(PROC_DIR_PATH, "[0-9]*", pp_vmswap_info);
    fmt_display(pp_vmswap_info, elem);

    return EXIT_SUCCESS;
}


/**
 * return value:
 * on success, return number of `t_vmswap_info' elements
 * on error, return -ENOMEM, or non-positive value
 */
static size_t lookup(const char *root, const char *regex, t_vmswap_info **pp_vmswap_info)
{
    char **p;
    glob_t res;
    chdir(root);
    glob(regex, GLOB_MARK | GLOB_NOSORT, 0, &res);
    size_t elem = 0;

    size_t n = res.gl_pathc;
    if (n > 0) {
        for (p = res.gl_pathv; n; p++, n--) {
            glob_t xres;
            size_t xlen = strlen(PROC_DIR_PATH) + strlen(*p) + 1;
            char xpath[xlen];
            snprintf(xpath, xlen, "%s/%s", PROC_DIR_PATH, *p);
            chdir(xpath);
            glob(PROC_PID_STAT_FILE, GLOB_MARK | GLOB_NOSORT, 0, &xres);

            size_t xn = xres.gl_pathc;
            char **xp;
            if (xn > 0) {
                for (xp = xres.gl_pathv; xn; xp++, xn--) {
                    size_t xxlen = xlen + strlen(*xp) + 1;
                    char xxpath[xxlen];
                    snprintf(xxpath, xxlen, "%s/%s", xpath, *xp);
                    //printf("  %s\n", xxpath);
                    t_vmswap_info *p_vmswap_info = malloc(sizeof(t_vmswap_info));
                    if (p_vmswap_info == NULL)
                        return -ENOMEM;
                    parse_proc_status_file(xxpath, p_vmswap_info);
                    *pp_vmswap_info = p_vmswap_info;
                    //printf("(%d) %s\n", __LINE__, (*pp_vmswap_info)->Name);
                    pp_vmswap_info++;
                    ++elem;
                }
            }
        }
    }
    globfree(&res);

    return elem;
}


static int parse_proc_status_file(const char *file, t_vmswap_info *p_vmswap_info)
{
    const char *str_re_target = "^(Name|Pid|PPid|State|Uid|VmSwap)";
    const char *str_re_pid = "^Pid";
    FILE *fp = fopen(file, "r");
    if (fp == 0)
        return EXIT_FAILURE;

    regex_t regex_target;
    if (regcomp(&regex_target, str_re_target, REG_EXTENDED) != 0) {
        fprintf(stderr, "-E-: regex %s failed to compile", str_re_target);
        return EXIT_FAILURE;
    }

    regex_t regex_pid;
    if (regcomp(&regex_pid, str_re_pid, REG_EXTENDED) != 0) {
        fprintf(stderr, "-E-: regex %s failed to compile", str_re_pid);
        return EXIT_FAILURE;
    }

    char line[100];
    while ((fgets(line, 100, fp)) != NULL) {
        line[strlen(line)-1] = '\0';
        if (regexec(&regex_target, line, 0, NULL, 0) == 0) {
            char *tok = strtok(line, ":");

            if (strcmp(tok, "Name") == 0) {
                tok = strtok(NULL, ":");
                p_vmswap_info->Name = ltrim(tok);
            } else if (strcmp(tok, "Pid") == 0) {
                tok = strtok(NULL, ":");
                p_vmswap_info->Pid = ltrim(tok);
            } else if (strcmp(tok, "PPid") == 0) {
                tok = strtok(NULL, ":");
                p_vmswap_info->PPid = ltrim(tok);
            } else if (strcmp(tok, "State") == 0) {
                tok = strtok(NULL, ":");
                p_vmswap_info->State = ltrim(tok);
            } else if (strcmp(tok, "Uid") == 0) {
                char *_tok = strtok(NULL, ":");
                _tok = strtok(_tok, "\t");
                p_vmswap_info->Uid = ltrim(_tok);
            } else if (strcmp(tok, "VmSwap") == 0) {
                tok = strtok(NULL, ":");
                p_vmswap_info->VmSwap = ltrim(tok);
            }
        }
    }
    //printf("(%d) %s\n", __LINE__, p_vmswap_info->Pid);
    
    return EXIT_SUCCESS;
}


/** TODO: unused */
static const char *get_full_cmd_arg(char *pid)
{
    char file[15 + strlen(pid)];
    printf("(%d) %zu\n", __LINE__, sizeof(file));
    //snprintf(file, sizeof(file), "/proc/", pid, "/cmdline");
    //FILE *fp = fopen(file, "r");
    return "";
}


static void fmt_display(t_vmswap_info **pp_vmswap_info, size_t elem)
{
    /*printf("%s%5s  %5s  %16s  %9s  %15s  %s%s\n",
                ANSI_COLOR_INVERT,
                "Pid", "PPid", "State", "Uid", "VmSwap", "Name",
                ANSI_COLOR_RESET);*/

    printf("%5s  %5s  %16s  %9s  %15s  %s\n",
                "Pid", "PPid", "State", "Uid", "VmSwap", "Name");
    int i;
    for (i = 0; i < elem; ++i) {
        printf("%5s  %5s  %16s  %9s  %15s  %s\n",
                pp_vmswap_info[i]->Pid,
                pp_vmswap_info[i]->PPid,
                pp_vmswap_info[i]->State,
                pp_vmswap_info[i]->Uid,
                pp_vmswap_info[i]->VmSwap,
                pp_vmswap_info[i]->Name);
    }
}


static char *ltrim(char *s)
{
    char *new = malloc(strlen(s)+1);
    int i, j, lmatch = 0;
    for (i = 0, j = 0; i < strlen(s); ++i) {
        if ((s[i] >= '!') && (s[i] <= '~') && (!lmatch)) {
            new[j++] = s[i];
            lmatch = 1;
        } else if (lmatch) {
            new[j++] = s[i];
        }
    }
    new[j] = '\0';
    
    return new;
}


static char *rtrim(char *s)
{
    char *new = malloc(strlen(s)+1);
    int i, j, rmatch = 0;
    for (i = strlen(s)-1; i >= 0; --i, --j) {
        if (s[i] >= '!' && s[i] <= '~' && (!rmatch)) {
            new[j] = s[i];
            rmatch = 1;
        }
    }
    new[j] = '\0';

    return new;
}


static char *trim(char *s)
{
    char *ltrm_str = ltrim(s);
    char *new = rtrim(ltrm_str);

    return new;
}
