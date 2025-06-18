/**
 * Author: Leon Rode
 * Date: 2025-06-18
 * Utility functions for the web proxy.
 */

#include "utils.h"
#include <string.h>
#include <sys/stat.h> // for mkdir
#include <stdlib.h> // for malloc

int extract_host_name(char* url, char* dest) {
    int i = 0;
    for (; i < strlen(url); i++) {
        if (url[i] == '/') {
            return i;
        }
        dest[i] = url[i];
    }
    return -1;
}

int extract_path(char* url, char* dest) {
    int i = 0, j = 0, f = 0;
    for (; i < strlen(url); i++) {
        if (f == 1) {
            dest[j++] = url[i];
        }
        if (url[i] == '/') {
            f = 1;
        }
    }
    return j;
}

void create_directory(char* path, char* dir_name) {
    char* full_path = malloc(strlen(path) + strlen(dir_name) + 2); // +2 for the '/' and '\0'
    strcpy(full_path, path);
    strcat(full_path, dir_name);
    mkdir(full_path, 0777);
    free(full_path);
}