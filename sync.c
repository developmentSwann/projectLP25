#include <sync.h>
#include <dirent.h>
#include <string.h>
#include <processes.h>
#include <utility.h>
#include <messages.h>
#include <file-properties.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <sys/msg.h>



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

    printf("Liste destination :\n");
    display_files_list(dst_list);
    //On compare les listes
    files_list_entry_t *src_cursor = src_list->head;
    while (src_cursor) {
        files_list_entry_t *dst_entry = find_entry_by_name(dst_list, src_cursor->path_and_name, 0, 0);
        printf("Liste source :\n");
        display_files_list(src_list);
        if (dst_entry == NULL || mismatch(src_cursor, dst_entry, the_config->uses_md5)) {
            printf("Fichier different\n");
            //On ajoute le fichier a la liste des fichiers a copier
            add_entry_to_tail(diff_list, src_cursor->path_and_name);
        }

        src_cursor = src_cursor->next;
        printf("Fichier source suivant : %s\n", src_cursor->path_and_name);
    }
    //On affiche la liste des fichiers a copier
    printf("Liste des fichiers a copier :\n");
    display_files_list(diff_list);
    //On copie les fichiers
    files_list_entry_t *diff_cursor = diff_list->head;
    while (diff_cursor) {
        copy_entry_to_destination(diff_cursor, the_config);
        diff_cursor = diff_cursor->next;
    }

    return;
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
    return;
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
    // On ouvre le fichier source
    if (source_fd == -1) {
        perror("Impossible d'ouvrir le fichier source");
        return;
    }
    // On recupere les informations du fichier source
    if (fstat(source_fd, &stat_buf) == -1) {
        perror("Impossible de lire les informations du fichier source");
        close(source_fd);
        return;
    }
    // On cree le fichier de destination (nom de fichier destination = chemin de destination + nom de fichier source)
    printf("Chemin de destination : %s\n", the_config->destination);
    printf("Nom de fichier source : %s\n", source_entry->path_and_name);
    //Récupérer juste le nom du fichier source
    char *file_name = strrchr(source_entry->path_and_name, '/');
    printf("Nom de fichier source : %s\n", file_name);
    //On concatene le chemin de destination avec le nom de fichier source
    char *dest_path = malloc(strlen(the_config->destination) + strlen(file_name) + 1);
    strcpy(dest_path, the_config->destination);
    strcat(dest_path, file_name);
    printf("Chemin de destination : %s\n", dest_path);
    dest_fd = open(dest_path, O_WRONLY | O_CREAT, stat_buf.st_mode);
    // On ouvre le fichier de destination
    if (dest_fd == -1) {
        perror("Impossible de creer le fichier de destination");
        close(source_fd);
        return;
    }
    // On copie le fichier
    if (sendfile(dest_fd, source_fd, &offset, stat_buf.st_size) == -1) {
        perror("Impossible de copier le fichier");
        close(source_fd);
        close(dest_fd);
        return;
    }
    // On ferme les fichiers
    close(source_fd);
    close(dest_fd);
    return;
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
    while (entry != NULL && strcmp(entry->d_name, "..") != 0) {
        printf("Entry : %s\n", entry->d_name);

        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("Ajout de %s\n", entry->d_name);

            // Allouer dynamiquement de la mémoire pour le chemin
            size_t path_size = strlen(target) + strlen(entry->d_name) + 2;


            files_list_entry_t *path = malloc(sizeof(files_list_entry_t));
            if (path == NULL) {
                printf("Failed to allocate memory for path\n");
                return;
            }
            //Check si c'est un dossier

            struct stat path_stat;
            stat(path, &path_stat);
            bool is_directory = S_ISDIR(path_stat.st_mode);

            if (is_directory) {
                make_list(list,entry->d_name);
            } else {
                add_entry_to_tail(list, concat_path(path->path_and_name, target, entry->d_name));
            }



        }

        // Obtenir la prochaine entrée
        entry = get_next_entry(dir);
        if (strcmp(entry->d_name, "..") == 0) {
            printf("Fin du dossier\n");
            // Fermer le dossier
            closedir(dir);
            return;
        }


    }
    closedir(dir);
    return;

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