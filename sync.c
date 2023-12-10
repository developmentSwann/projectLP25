#include <sync.h>
#include <dirent.h>
#include <string.h>
#include <processes.h>
#include <utility.h>
#include <messages.h>
#include <file-properties.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <sys/msg.h>
#include "stdlib.h"
#include <stdio.h>


/*!
 * @brief synchronize is the main function for synchronization
 * It will build the lists (source and destination), then make a third list with differences, and apply differences to the destination
 * It must adapt to the parallel or not operation of the program.
 * @param the_config is a pointer to the configuration
 * @param p_context is a pointer to the processes context
 */
void synchronize(configuration_t *the_config, process_context_t *p_context) {
    files_list_t *src_list = malloc(sizeof(files_list_t));
    files_list_t *dst_list = malloc(sizeof(files_list_t));
    files_list_t *diff_list = malloc(sizeof(files_list_t));

    if (the_config->is_parallel) {
        //TODO : Parallel
    } else {
        make_files_list(src_list, the_config->source);
        make_files_list(dst_list, the_config->destination);
    }
    //On affiche les listes
    printf("Liste source :\n");
    display_files_list(src_list);
    printf("Liste destination :\n");
    display_files_list(dst_list);
}

/*!
 * @brief mismatch tests if two files with the same name (one in source, one in destination) are equal
 * @param lhd a files list entry from the source
 * @param rhd a files list entry from the destination
 * @has_md5 a value to enable or disable MD5 sum check
 * @return true if both files are not equal, false else
 */
bool mismatch(files_list_entry_t *lhd, files_list_entry_t *rhd, bool has_md5) {
    if (lhd->entry_type != rhd->entry_type) {
        return true;
    }
    if (lhd->entry_type == DOSSIER) {
        return false;
    }
    if (lhd->size != rhd->size) {
        return true;
    }
    if (lhd->mtime.tv_sec != rhd->mtime.tv_sec) {
        return true;
    }
    if (has_md5  && strcmp(lhd->md5sum, rhd->md5sum) != 0) {
        return true;
    }
    return false;
}

/*!
 * @brief make_files_list buils a files list in no parallel mode
 * @param list is a pointer to the list that will be built
 * @param target_path is the path whose files to list
 */
void  make_files_list(files_list_t *list, char *target_path) {
    make_list(list, target_path);
    files_list_entry_t *cursor = list->head;

    while (cursor) {
        if (get_file_stats(cursor) < 0) {
            printf("Impossible de recuperer les informations du fichier %s\n", cursor->path_and_name);
        }
        cursor = cursor->next;
    }
}

/*!
 * @brief make_files_lists_parallel makes both (src and dest) files list with parallel processing
 * @param src_list is a pointer to the source list to build
 * @param dst_list is a pointer to the destination list to build
 * @param the_config is a pointer to the program configuration
 * @param msg_queue is the id of the MQ used for communication
 */
void make_files_lists_parallel(files_list_t *src_list, files_list_t *dst_list, configuration_t *the_config, int msg_queue) {

}

/*!
 * @brief copy_entry_to_destination copies a file from the source to the destination
 * It keeps access modes and mtime (@see utimensat)
 * Pay attention to the path so that the prefixes are not repeated from the source to the destination
 * Use sendfile to copy the file, mkdir to create the directory
 */
void copy_entry_to_destination(files_list_entry_t *source_entry, configuration_t *the_config) {
    int source_fd, dest_fd;
    struct stat stat_buf;
    off_t offset = 0; // Definie dans sendfile.h
    source_fd = open(source_entry->path_and_name, O_RDONLY);
    if (source_fd == -1) {
        perror("Impossible d'ouvrir le fichier source");
        return;
    }
    if (fstat(source_fd, &stat_buf) == -1) {
        perror("Impossible de lire les informations du fichier source");
        close(source_fd);
        return;
    }
    dest_fd = open(the_config->destination, O_WRONLY , stat_buf.st_mode);
    if (dest_fd == -1) {
        perror("Impossible d'ouvrir le fichier de destination");
        close(source_fd);
        return;
    }
    if (sendfile(dest_fd, source_fd, &offset, stat_buf.st_size) == -1) { // Defini dans sendfile.h
        perror("Impossible de copier le fichier");
    }
    else {
        printf("Fichier bien copie.\n");
    }
    close(source_fd);
    close(dest_fd);
}

/*!
 * @brief make_list lists files in a location (it recurses in directories)
 * It doesn't get files properties, only a list of paths
 * This function is used by make_files_list and make_files_list_parallel
 * @param list is a pointer to the list that will be built
 * @param target is the target dir whose content must be listed
 */
void make_list(files_list_t *list, char *target) {
    DIR *dir = open_dir(target);
    if (dir == NULL) {
        printf("Impossible d'ouvrir le dossier %s\n", target);
        return;
    }
    struct dirent *entry = get_next_entry(dir);
    while (entry) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char *path = malloc(sizeof(char) * (strlen(target) + strlen(entry->d_name) + 2));
            strcpy(path, target);
            strcat(path, "/");
            strcat(path, entry->d_name);
            files_list_entry_t *list_entry = add_file_entry(list, path);
            if (list_entry == NULL) {
                printf("Impossible d'ajouter le fichier %s a la liste\n", path);
            }
            struct stat path_stat;
            stat(path, &path_stat);
            if (S_ISDIR(path_stat.st_mode)) {
                make_list(list, path);
            }

            free(path);
        }
        entry = get_next_entry(dir);
    }
}

/*!
 * @brief open_dir opens a dir
 * @param path is the path to the dir
 * @return a pointer to a dir, NULL if it cannot be opened
 */
DIR *open_dir(char *path) {
    DIR *dir = opendir(path);
    return dir;
}

/*!
 * @brief get_next_entry returns the next entry in an already opened dir
 * @param dir is a pointer to the dir (as a result of opendir, @see open_dir)
 * @return a struct dirent pointer to the next relevant entry, NULL if none found (use it to stop iterating)
 * Relevant entries are all regular files and dir, except . and ..
 */
struct dirent *get_next_entry(DIR *dir) {
    struct dirent *entry = readdir(dir);
    return entry;
}
