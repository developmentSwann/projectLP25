#pragma once

#include <files-list.h>
#include <defines.h>
#include <stdio.h>

// Ici on définit la taille maximale d'un chemin de fichier pour ne pas trop consommer
#define PATH_SIZE 4096
#define COMMAND_CODE_TERMINATE 0x0
#define COMMAND_CODE_TERMINATE_OK 0x10
#define COMMAND_CODE_ANALYZE_FILE 0x01
#define COMMAND_CODE_FILE_ANALYZED 0x11
#define COMMAND_CODE_ANALYZE_DIR 0x02
#define COMMAND_CODE_FILE_ENTRY 0x12
#define COMMAND_CODE_LIST_COMPLETE 0x22

#define MSG_TYPE_TO_MAIN 1
#define MSG_TYPE_TO_SOURCE_LISTER 2
#define MSG_TYPE_TO_DESTINATION_LISTER 3
#define MSG_TYPE_TO_SOURCE_ANALYZERS 4
#define MSG_TYPE_TO_DESTINATION_ANALYZERS 5

typedef struct {
    long mtype;
    char message;
} simple_command_t;

typedef struct {
    long mtype;
    char op_code;
    files_list_entry_t payload;
} analyze_file_command_t;

typedef struct {
    long mtype;
    char op_code;
    files_list_entry_t payload;
    int reply_to;
} files_list_entry_transmit_t;

typedef struct {
    long mtype;
    char op_code;
    char target[PATH_SIZE];
} analyze_dir_command_t;

typedef union {
    simple_command_t simple_command;
    analyze_file_command_t analyze_file_command;
    analyze_dir_command_t analyze_dir_command;
    files_list_entry_transmit_t list_entry;
} any_message_t;

// Ici on définit la structure qui gère chaque entrée dans la liste des fichiers
typedef struct {
    char path[PATH_SIZE];  // Chemin complet du fichier ou du répertoire
    char name[256];        // Nom du fichier ou du répertoire
    char md5[33];          // Somme de contrôle MD5 (Empreinte digitale unique pour identifier le contenu du fichier)
    time_t modification_time; // Date de dernière modification
    off_t size;            // Taille du fichier (zéro pour les répertoires)
} files_list_entry;

int msgsnd(int queue, analyze_dir_command_t *ptr, size_t i, int i1);

void strncpy(char target[4096], char *dir, int i);

// Fonction qui permet d'envoyer une commande pour analyser des répertoires
int send_analyze_dir_command(int msg_queue, int recipient, char *target_dir) {
    analyze_dir_command_t command;
    command.mtype = recipient;
    command.op_code = COMMAND_CODE_ANALYZE_DIR;
    strncpy(command.target, target_dir, PATH_SIZE - 1);
    command.target[PATH_SIZE - 1] = '\0';
    if (msgsnd(msg_queue, &command, sizeof(analyze_dir_command_t) - sizeof(long), 0) == -1) {
        perror("Erreur d'analuse de la commande");
        return -1;
    }
    return 0;
}

void strncpy(char target[4096], char *dir, int i) {

}

int msgsnd(int queue, analyze_dir_command_t *ptr, size_t i, int i1) {
    return 0;
}

// Fonction qui permet d'envoyer une entrée de fichier à un destinataire avec un code de commande spécifié
int send_file_entry(int msg_queue, int recipient, files_list_entry_t *file_entry, int cmd_code) {
    files_list_entry_transmit_t transmit;
    transmit.mtype = recipient;
    transmit.op_code = cmd_code;
    transmit.payload = *file_entry;
    if (msgsnd(msg_queue, &transmit, sizeof(files_list_entry_transmit_t) - sizeof(long), 0) == -1) {
        perror("Erreur d'entrée des fichiers");
        return -1;
    }
    return 0;
}

// Fonction pour envoyer une commande d'analyse de fichier
int send_analyze_file_command(int msg_queue, int recipient, files_list_entry_t *file_entry) {
    return send_file_entry(msg_queue, recipient, file_entry, COMMAND_CODE_ANALYZE_FILE);
}

// Fonction pour envoyer une réponse d'analyse de fichier
int send_analyze_file_response(int msg_queue, int recipient, files_list_entry_t *file_entry) {
    return send_file_entry(msg_queue, recipient, file_entry, COMMAND_CODE_FILE_ANALYZED);
}

// Fonction pour envoyer une entrée de liste de fichiers à un destinataire
int send_files_list_element(int msg_queue, int recipient, files_list_entry_t *file_entry) {
    return send_file_entry(msg_queue, recipient, file_entry, COMMAND_CODE_FILE_ENTRY);
}

// Fonction pour signaler la fin de la transmission de la liste des fichiers
int send_list_end(int msg_queue, int recipient) {
    simple_command_t list_end_command;
    list_end_command.mtype = recipient;
    list_end_command.message = COMMAND_CODE_LIST_COMPLETE;
    if (msgsnd(msg_queue, &list_end_command, sizeof(simple_command_t) - sizeof(long), 0) == -1) {
        perror("Erreur pour envoyer les commandes");
        return -1;
    }
    return 0;
}

// Fonction pour envoyer une commande de fin d'éxécution
int send_terminate_command(int msg_queue, int recipient) {
    simple_command_t terminate_command;
    terminate_command.mtype = recipient;
    terminate_command.message = COMMAND_CODE_TERMINATE;
    if (msgsnd(msg_queue, &terminate_command, sizeof(simple_command_t) - sizeof(long), 0) == -1) {
        perror("Erreur pour envoyer les commandes terminales");
        return -1;
    }
    return 0;
}

// Fonction pour confirmer la fin de l'exécution
int send_terminate_confirm(int msg_queue, int recipient) {
    simple_command_t terminate_confirm;
    terminate_confirm.mtype = recipient;
    terminate_confirm.message = COMMAND_CODE_TERMINATE_OK;
    if (msgsnd(msg_queue, &terminate_confirm, sizeof(simple_command_t) - sizeof(long), 0) == -1) {
        perror("Erreur de confirmation des commandes");
        return -1;
    }
    return 0;
}
