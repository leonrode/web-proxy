/**
 * Author: Leon Rode
 * Date: 2025-06-18
 * Utility functions for the web proxy.
 */

#ifndef UTILS_H
#define UTILS_H

#define CACHE_DIR "./cache/"

/**
 * Note: This program assumes the URL is of the form "<host_name>/<path>", i.e.
 * no http:// or https:// as all requests are made in HTTP only
 */

/**
 * Extracts the host name from the <object> retrieved from `extract_path` stored in `path` and stores it in `dest`, returning
 * the number of characters copied to `dest`.
 */
int extract_host_name_from_object(char* object, char* dest, int n);

/**
 * Extracts the object in "GET <object> HTTP/1.1" from the request `buffer`
 * of size `n` and stores it in `dest`, returning the number of characters copied to `dest`.
 */
int extract_object_from_buffer(char* buffer, char* dest, int n);

/** 
 * Extracts the path from the <object> retrieved from `extract_path` stored in `path` and stores it in `dest`, returning
 * the number of characters copied to `dest`.
 */
int extract_path_from_object(char* object, char* dest, int n);
/**
 * Creates a directory with the name `dir_name` at path.
 * E.g. create_directory("/cache/", "hostname") will result in a directory at "/cache/hostname"
 */
void create_directory(char* path, char* dir_name); // will be used to create directory for any kind of path

/**
 * Checks if the file at `path` is in the cache.
 * Returns 1 if the file exists in the cache, 0 otherwise.
 * @param path: the path to the file to check (in the format "<hostname>/<path>")
 */
int cache_check(char* path);



#endif