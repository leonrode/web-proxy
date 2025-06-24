/**
 * Author: Leon Rode
 * Date: 2025-06-18
 * Utility functions for the web proxy.
 */

#include "utils.h"
#include <string.h>
#include <sys/stat.h> // for mkdir
#include <stdlib.h> // for malloc
#include <stdio.h> // for fopen

// path is of the form "<host_name>/<path>"
int extract_host_name_from_object(char* object, char* dest, int n) {
    int i = 0;
    for (; i < n; i++) {
        if (object[i] == '/') {
            return i;
        }
        dest[i] = object[i];
    }
    return -1;
}

int extract_object_from_buffer(char* buffer, char* dest, int n) {
    int i = 5, j = 0; // 5 is the index of the first character after "GET /"
    for (; i < n; i++) {
        if (buffer[i] == ' ') return j;
        dest[j++] = buffer[i];
    }
    return j;
}

int extract_path_from_object(char* object, char* dest, int n) {
    char* junk = malloc(n);
    int j = 0;
    int host_name_len = extract_host_name_from_object(object, junk, n);
    for (int i = host_name_len + 1; i < n; i++) {
        dest[j++] = object[i];
    }
    free(junk);

    return j;


}

void create_directory(char* path, char* dir_name) {
    char* full_path = malloc(strlen(path) + strlen(dir_name) + 2); // +2 for the '/' and '\0'
    strcpy(full_path, path);
    strcat(full_path, dir_name);
    mkdir(full_path, 0777);
    free(full_path);
}

int cache_check(char* path) {
    // attempt to open the file

    char* cache_path = malloc(strlen(CACHE_DIR) + strlen(path) + 2);
    strcpy(cache_path, CACHE_DIR);
    strcat(cache_path, path);

    printf("Checking cache presence at %s\n", cache_path);

    FILE* fp = fopen(cache_path, "r");
    if (fp == NULL) {
        free(cache_path);   
        return 0; // file does not exist in cache
    }

    fclose(fp);
    free(cache_path);
    return 1;
}