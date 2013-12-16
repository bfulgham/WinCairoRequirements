/*
 * The Windows directory handling logic is courtesy of the FreeBSD shttpd sources.
 */
/*
 * Copyright (c) 2004-2005 Sergey Lyubka <valenok@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#if !defined(__COREFOUNDATION_WINDOWS_DIRENT__)
#define __COREFOUNDATION_WINDOWS_DIRENT__ 1

#if defined(__cplusplus)
extern "C" {
#endif

#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <direct.h>
#include <io.h>

/* POSIX dirent interface */
struct dirent {
        char*                   d_name;
        uint8_t                 d_type;
        long                    d_fileno;
};

typedef struct DIR {
        long                    handle;
        struct _finddata_t      info;
        struct dirent           result;
        char*                   name;
} DIR;

extern DIR* opendir (const char* name);
extern struct dirent* readdir (DIR* dir);
extern int closedir (DIR* dir);

#define	DT_UNKNOWN	     0
#define	DT_DIR		     4

#if defined(__cplusplus)
}
#endif

#endif /* __COREFOUNDATION_WINDOWS_DIRENT__ */
