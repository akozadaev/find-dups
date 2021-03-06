/*
 *  main.c
 *  Author: Alex Kozadaev (2015)
 */

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#include "build_host.h"
#include "common.h"
#include "hasharray.h"

#define check_error(a) if((a) != R_OK) { goto error; }

static void usage(void);
static int print_callback(const char *str);
static void update_progress(void);
static int handle_file(char *fname);
static int walk_dir(const char *dir, int (*cb)(char *));

int
main(int argc, char **argv)
{
    if (argc == 1) {
        usage();
    } else {
        hasharray_init();
        while (--argc > 0) {
            if (handle_file(*(++argv)) != R_OK) {
                usage();
                die("error: invalid agruments\n");
            }
        }
        fputc('\r', stderr);    /* make sure the cursor is at the begining */
        hasharray_finddups(print_callback);
        hasharray_free();
    }
    return EXIT_SUCCESS;
}

static void
usage(void)
{
    puts("finddup v" BUILD_VERSION "\n"
         "Usage: finddup [directory/files to search]\n");
}

/* file name printing callback */
static int
print_callback(const char *str)
{
    return printf("\t%s\n", str);
}

static void
update_progress(void)
{
    static unsigned fcount = 0;
    fprintf(stderr, "\rProcessed files: %d   ", ++fcount);
}

/* handle the files in the given directory
 * calls walk_dir in case a directory is found and recursively
 * looking through in the subdirectories */
static int
handle_file(char *fname)
{
    struct stat stbuf;
    md5_t chksum = { 0 };

    if (stat(fname, &stbuf) == -1) {
        perror(strerror(errno));
        goto error;
    }

    if (S_ISDIR(stbuf.st_mode)) {
        check_error(walk_dir(fname, handle_file));
    } else {
        check_error(md5_get(fname, chksum));
        hasharray_add(chksum, fname);
        update_progress();
    }

    return R_OK;

error:
    return R_ERR;
}

/* got to the directory, make necessary checks and call
 * (handle_files) to handle the contents */
static int
walk_dir(const char *dir, int (*cb)(char *))
{
    char name[MAXPATH];
    struct dirent *dp;
    DIR *dfd;
    size_t dirlen;

    if ((dfd = opendir(dir)) == NULL) {
        goto error;
    }

    dirlen = strlen(dir);
    while ((dp = readdir(dfd)) != NULL) {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
            continue;
        }
        if ((dirlen + strlen(dp->d_name)+2) > MAXPATH) {
            goto error;
        } else {
            if (dp->d_type == DT_REG || dp->d_type == DT_DIR) {
                sprintf(name, "%s/%s", dir, dp->d_name);
                cb(name);
            }
        }
    }
    closedir(dfd);
    return R_OK;

error:
    if (dfd) { closedir(dfd); }
    perror(strerror(errno));
    return R_ERRDIR;
}

