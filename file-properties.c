#include <file-properties.h>

#include <sys/stat.h>
#include <dirent.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <defines.h>
#include <fcntl.h>
#include <stdio.h>
#include <utility.h>
#include "files-list.h"
#include "defines.h"
#include "utility.h"
#include <stdlib.h>

/*!
 * @brief get_file_stats gets all of the required information for a file (inc. directories)
 * @param the files list entry
 * You must get:
 * - for files:
 *   - mode (permissions)
 *   - mtime (in nanoseconds)
 *   - size
 *   - entry type (FICHIER)
 *   - MD5 sum
 * - for directories:
 *   - mode
 *   - entry type (DOSSIER)
 * @return -1 in case of error, 0 else
 */
int get_file_stats(files_list_entry_t *entry) {
    struct stat fileStat;
    if (stat(entry->path_and_name, &fileStat) < 0) {
        perror("stat");
        return -1;
    }
    entry->mode = fileStat.st_mode;
    if (S_ISDIR(fileStat.st_mode)) {
        entry->entry_type = DOSSIER;
    } else {
        entry->mtime.tv_sec = fileStat.st_mtime;
        entry->mtime.tv_nsec = fileStat.st_mtime;

        entry->size = fileStat.st_size;
        entry->entry_type = FICHIER;
        if (compute_file_md5(entry) < 0) {
            return -1;
        }
    }

    return 0;
}

/*!
 * @brief compute_file_md5 computes a file's MD5 sum
 * @param the pointer to the files list entry
 * @return -1 in case of error, 0 else
 * Use libcrypto functions from openssl/evp.h
 */

int compute_file_md5(files_list_entry_t *entry) {
    unsigned char digest[16];
    char mdString[33];
    int i;
    FILE *inFile = fopen(entry->path_and_name, "rb");
    if (!inFile) {
        return -1;
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if(mdctx == NULL) {
        fclose(inFile);
        return -1;
    }

    if(1 != EVP_DigestInit_ex(mdctx, EVP_md5(), NULL)) {
        EVP_MD_CTX_free(mdctx);
        fclose(inFile);
        return -1;
    }

    unsigned char data[1024];
    int bytes;
    while((bytes = fread(data, 1, 1024, inFile)) != 0) {
        if(1 != EVP_DigestUpdate(mdctx, data, bytes)) {
            EVP_MD_CTX_free(mdctx);
            fclose(inFile);
            return -1;
        }
    }

    if(1 != EVP_DigestFinal_ex(mdctx, digest, NULL)) {
        EVP_MD_CTX_free(mdctx);
        fclose(inFile);
        return -1;
    }

    for(i = 0; i < 16; i++) {
        sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
    }

    memcpy(entry->md5sum, mdString, 33);

    EVP_MD_CTX_free(mdctx);
    fclose(inFile);

    return 0;
}

/*!
 * @brief directory_exists tests the existence of a directory
 * @path_to_dir a string with the path to the directory
 * @return true if directory exists, false else
 */
bool directory_exists(char *path_to_dir) {
    DIR *dir = opendir(path_to_dir);
    if (dir != NULL) {
        closedir(dir);
        return true;
    }
    return false;
}

/*!
 * @brief is_directory_writable tests if a directory is writable
 * @param path_to_dir the path to the directory to test
 * @return true if dir is writable, false else
 * Hint: try to open a file in write mode in the target directory.
 */
bool is_directory_writable(char *path_to_dir) {
    if (path_to_dir == NULL) {
        return false;
    }

    char *test_file = "/test.txt";
    if (strlen(path_to_dir) + strlen(test_file) >= PATH_SIZE) {
        return false;
    }

    char path[PATH_SIZE];
    snprintf(path, PATH_SIZE, "%s%s", path_to_dir, test_file);

    int fd = open(path, O_WRONLY | O_CREAT, 0666);
    if (fd < 0) {
        return false;
    }
    close(fd);
    unlink(path);
    return true;
}
