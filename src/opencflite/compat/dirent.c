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

#include <errno.h>
#include <malloc.h>
#include <stdint.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "dirent.h"

DIR* opendir (const char *name)
{
   DIR* dir = NULL;
   size_t          base_length;
   const char* all;

   if (name && name[0]) {
      base_length = strlen(name);
      all = strchr("/\\", name[base_length - 1]) ? "*" : "/*";

      if ((dir = (DIR*)malloc(sizeof *dir)) != NULL &&
          (dir->name = (char*)malloc(base_length + strlen(all) + 1)) != 0) {
         (void) strcat(strcpy(dir->name, name), all);

         if ((dir->handle = (long) _findfirst(dir->name, &dir->info)) != -1) {
            dir->result.d_name = 0;
         } else {
            free(dir->name);
            free(dir);
            dir = 0;
         }
      } else {
         free(dir);
         dir = NULL;
         errno = ENOMEM;
      }
   } else {
      errno = EINVAL;
   }

   return (dir);
}

int closedir (DIR* dir)
{
   int result = -1;

   if (dir) {
      if(dir->handle != -1)
         result = _findclose(dir->handle);

      free(dir->name);
      free(dir);
   }

   if (result == -1)
      errno = EBADF;

   return (result);
}

struct dirent* readdir (DIR *dir)
{
   struct dirent* result = 0;

   if (dir && dir->handle != -1) {
      if (!dir->result.d_name ||
         _findnext(dir->handle, &dir->info) != -1) {
         result = &dir->result;
         result->d_name = dir->info.name;
         result->d_fileno = 1;   // Not real!

         if (dir->info.attrib & FILE_ATTRIBUTE_DIRECTORY)
            result->d_type = DT_DIR;
         else
            result->d_type = DT_UNKNOWN;
      }
   } else {
      errno = EBADF;
   }

   return (result);
}
