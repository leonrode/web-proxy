/**
 * Author: Leon Rode
 * Date: 2025-06-18
 * Utility functions for the web proxy.
 */

#ifndef UTILS_H
#define UTILS_H

/**
 * Note: This program assumes the URL is of the form "<host_name>/<path>", i.e.
 * no http:// or https:// as all requests are made in HTTP only
 */

/**
 * Extracts the host name from the URL stored in `url` and stores it in `dest`, returning
 * the number of characters copied to `dest`.
 */
int extract_host_name(char* url, char* dest);

/**
 * Extracts the path from the URL stored in `url` and stores it in `dest`, returning
 * the number of characters copied to `dest`.
 */
int extract_path(char* url, char* dest);

/**
 * Creates a directory with the name `dir_name` at path.
 * E.g. create_directory("/cache/", "hostname") will result in a directory at "/cache/hostname"
 */
void create_directory(char* path, char* dir_name); // will be used to create directory for any kind of path



#endif