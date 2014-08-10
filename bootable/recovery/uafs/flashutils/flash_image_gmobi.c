/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 /*
 - 这里使用了CM7的代码.
 - 目前支持MTD,eMMC,BML
 - notmmao@gmail.com > 2013-02-01
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cutils/log.h"

extern int 
restore_raw_partition(const char* partitionType, const char* partition, const char* filename);

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "usage: %s partition file.img\n", argv[0]);
        return 2;
    }

    int ret = restore_raw_partition(NULL, argv[1], argv[2]);
    if (ret != 0) {
        fprintf(stderr, "failed with error: %d\n(%s)", ret, strerror(errno));
    }
    return ret;
}
