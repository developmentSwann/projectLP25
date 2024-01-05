#include <files-list.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <stdio.h>
#include "file-properties.h"

/*!
 * @brief clear_files_list clears a files list
 * @param list is a pointer to the list to be cleared
 * This function is provided, you don't need to implement nor modify it
 */
void clear_files_list(files_list_t *list) {
    while (list->head) {
        files_list_entry_t *tmp = list->head;
        list->head = tmp->next;
        free(tmp);
    }
}

/*!
 *  @brief add_file_entry adds a new file to the files list.
 *  It adds the file in an ordered manner (strcmp) and fills its properties
 *  by calling stat on the file.
 *  If the file already exists, it does nothing and returns 0
 *  @param list the list to add the file entry into
 *  @param file_path the full path (from the root of the considered tree) of the file
 *  @return a pointer to the added element if success, NULL else (out of memory)
 */


files_list_entry_t *add_file_entry(files_list_t *list, char *file_path) {
    if (list == NULL || file_path == NULL) {
        return NULL;
    }
    files_list_entry_t *new_entry = malloc(sizeof(files_list_entry_t));
    if (new_entry == NULL) {
        return NULL;
    }
    strncpy(new_entry->path_and_name, file_path, sizeof(new_entry->path_and_name) - 1);
    new_entry->path_and_name[sizeof(new_entry->path_and_name) - 1] = '\0'; // Ensure null termination

    struct stat statbuf;
    if (stat(file_path, &statbuf) == -1) {
        free(new_entry);
        return NULL;
    }

    new_entry->mtime.tv_sec = statbuf.st_mtime;
    new_entry->mtime.tv_nsec = statbuf.st_ctime;
    new_entry->size = statbuf.st_size;
    new_entry->mode = statbuf.st_mode;
    new_entry->entry_type = S_ISDIR(statbuf.st_mode) ? DOSSIER : FICHIER;
    memset(new_entry->md5sum, 0, sizeof(new_entry->md5sum));

    new_entry->next = NULL;
    new_entry->prev = NULL;

    if (list->head == NULL) {
        list->head = new_entry;
        list->tail = new_entry;
    } else {
        files_list_entry_t *current = list->head;
        files_list_entry_t *prev = NULL;
        while (current && strcmp(current->path_and_name, file_path) < 0) {
            prev = current;
            current = current->next;
        }
        if (current && strcmp(current->path_and_name, file_path) == 0) {
            free(new_entry);
            return NULL;
        }
        if (prev == NULL) {
            list->head = new_entry;
        } else {
            prev->next = new_entry;
        }
        new_entry->prev = prev;
        new_entry->next = current;
        if (current == NULL) {
            list->tail = new_entry;
        } else {
            current->prev = new_entry;
        }
    }
    return new_entry;
}


/*!
 * @brief add_entry_to_tail adds an entry directly to the tail of the list
 * It supposes that the entries are provided already ordered, e.g. when a lister process sends its list's
 * elements to the main process.
 * @param list is a pointer to the list to which to add the element
 * @param entry is a pointer to the entry to add. The list becomes owner of the entry.
 * @return 0 in case of success, -1 else
 */
int add_entry_to_tail(files_list_t *list, files_list_entry_t *entry) {
    if (list == NULL || entry == NULL) {
        return -1;
    }
    files_list_entry_t *new_entry = malloc(sizeof(files_list_entry_t));
    if (new_entry == NULL) {
        return -1;
    }
    memcpy(new_entry, entry, sizeof(files_list_entry_t));

    if (list->head == NULL) {
        list->head = new_entry;
        list->tail = new_entry;
        list->head->prev = NULL;
        list->head->next = NULL;
    } else {
        files_list_entry_t *temp = list->tail;
        list->tail->next = new_entry;
        list->tail = new_entry;
        list->tail->prev = temp;
        list->tail->next = NULL;
    }

    return 0;
}

/*!
 *  @brief find_entry_by_name looks up for a file in a list
 *  The function uses the ordering of the entries to interrupt its search
 *  @param list the list to look into
 *  @param file_path the full path of the file to look for
 *  @param start_of_src the position of the name of the file in the source directory (removing the source path)
 *  @param start_of_dest the position of the name of the file in the destination dir (removing the dest path)
 *  @return a pointer to the element found, NULL if none were found.
 */
files_list_entry_t *find_entry_by_name(files_list_t *list, char *file_path, size_t start_of_src, size_t start_of_dest) {
    if (list == NULL || file_path == NULL) {
        return NULL;
    }
    //On cherche le fichier dans la liste sans modifier la liste
    files_list_entry_t *cursor = list->head;
    while (cursor) {
        if (strcmp(cursor->path_and_name, file_path) == 0) {
            return cursor;
        }
        if (strcmp(cursor->path_and_name, file_path) > 0) {
            return NULL;
        }
        cursor = cursor->next;
    }
}

/*!
 * @brief display_files_list displays a files list
 * @param list is the pointer to the list to be displayed
 * This function is already provided complete.
 */
void display_files_list(files_list_t *list) {
    if (!list)
        return;

    for (files_list_entry_t *cursor=list->head; cursor!=NULL; cursor=cursor->next) {
        printf("%s\n", cursor->path_and_name);
    }
}

/*!
 * @brief display_files_list_reversed displays a files list from the end to the beginning
 * @param list is the pointer to the list to be displayed
 * This function is already provided complete.
 */
void display_files_list_reversed(files_list_t *list) {
    if (!list)
        return;

    for (files_list_entry_t *cursor=list->tail; cursor!=NULL; cursor=cursor->prev) {
        printf("%s\n", cursor->path_and_name);
    }
}
