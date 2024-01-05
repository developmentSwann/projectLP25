#include "processes.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <stdio.h>
#include <messages.h>
#include <file-properties.h>
#include <sync.h>
#include <string.h>
#include <errno.h>

/*!
 * @brief prepare prepares (only when parallel is enabled) the processes used for the synchronization.
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the program processes context
 * @return 0 if all went good, -1 else
 */
int prepare(configuration_t *the_config, process_context_t *p_context) {
    if (!the_config->is_parallel) {
        return 0;
    }
    //On cree la file de message
    p_context->message_queue_id = msgget(IPC_PRIVATE, IPC_CREAT | 0666);
    if (p_context->message_queue_id < 0) {
        perror("msgget");
        return -1;
    }
    //On cree les processus
    p_context->source_lister_pid = make_process(p_context, lister_process_loop, NULL);
    p_context->destination_lister_pid = make_process(p_context, lister_process_loop, NULL);
    p_context->source_analyzers_pids = malloc(sizeof(pid_t) * the_config->processes_count);
    p_context->destination_analyzers_pids = malloc(sizeof(pid_t) * the_config->processes_count);
    for (int i = 0; i < the_config->processes_count; i++) {
        p_context->source_analyzers_pids[i] = make_process(p_context, analyzer_process_loop, NULL);
        p_context->destination_analyzers_pids[i] = make_process(p_context, analyzer_process_loop, NULL);
    }
    return 0;

}

/*!
 * @brief make_process creates a process and returns its PID to the parent
 * @param p_context is a pointer to the processes context
 * @param func is the function executed by the new process
 * @param parameters is a pointer to the parameters of func
 * @return the PID of the child process (it never returns in the child process)
 */
int make_process(process_context_t *p_context, process_loop_t func, void *parameters) {
    pid_t pid = fork();
    if (pid == 0) {
        func(parameters);
        exit(0);
    } else {
        return pid;
    }

}

/*!
 * @brief lister_process_loop is the lister process function (@see make_process)
 * @param parameters is a pointer to its parameters, to be cast to a lister_configuration_t
 */
void lister_process_loop(void *parameters) {
    //On recupere les parametres
    lister_configuration_t *l_config = (lister_configuration_t *) parameters;
    //On cree la liste des fichiers
    files_list_t *list = malloc(sizeof(files_list_t));
    if (list == NULL) {
        perror("malloc");
        exit(-1);
    }

    list->head = NULL;
    list->tail = NULL;
    //On recupere les fichiers a analyser
    while (true) {
        any_message_t message;
        //Message de terminaison
        if (message.simple_command.message == COMMAND_CODE_TERMINATE) {
            send_terminate_confirm(l_config->mq_key, l_config->my_recipient_id);
            break;
        }
        //Message d'analyse de fichier ou de dossier
        if (message.simple_command.message == COMMAND_CODE_FILE_ANALYZED) {
            files_list_entry_t *entry = malloc(sizeof(files_list_entry_t));
            if (entry == NULL) {
                perror("malloc");
                exit(-1);
            }
            memcpy(entry, &message.analyze_file_command.payload, sizeof(files_list_entry_t));
            add_entry_to_tail(list, entry);

        } else if (message.simple_command.message == COMMAND_CODE_LIST_COMPLETE) { //Message de fin de liste
            break;
        }
    }
}

/*!
 * @brief analyzer_process_loop is the analyzer process function
 * @param parameters is a pointer to its parameters, to be cast to an analyzer_configuration_t
 */


void analyzer_process_loop(void *parameters) {
    //On recupere les parametres
    analyzer_configuration_t *a_config = (analyzer_configuration_t *) parameters;
    //On cree la liste des fichiers
    files_list_t *list = malloc(sizeof(files_list_t));
    if (list == NULL) {
        perror("malloc");
        exit(-1);
    }
    list->head = NULL;
    list->tail = NULL;
    //On recupere les fichiers a analyser
    while (true) {
        any_message_t message;
        //Message de terminaison


        if (message.simple_command.message == COMMAND_CODE_TERMINATE) {
            send_terminate_confirm(a_config->mq_key, a_config->my_recipient_id);
            break;
        }
        //Message d'analyse de fichier ou de dossier
        if (message.simple_command.message == COMMAND_CODE_FILE_ANALYZED) {
            files_list_entry_t *entry = malloc(sizeof(files_list_entry_t));
            if (entry == NULL) {
                perror("malloc");
                exit(-1);
            }
            memcpy(entry, &message.analyze_file_command.payload, sizeof(files_list_entry_t));
            add_entry_to_tail(list, entry);

        } else if (message.simple_command.message == COMMAND_CODE_LIST_COMPLETE) { //Message de fin de liste
            break;
        }
    }
    //On envoie la liste des fichiers a copier
    files_list_entry_t *cursor = list->head;
    while (cursor) {
        send_files_list_element(a_config->mq_key, a_config->my_receiver_id, cursor);
        cursor = cursor->next;
    }
    //On envoie le message de fin de liste
    send_list_end(a_config->mq_key, a_config->my_receiver_id);
    free(list);





}

/*!
 * @brief clean_processes cleans the processes by sending them a terminate command and waiting to the confirmation
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the processes context
 */
void clean_processes(configuration_t *the_config, process_context_t *p_context) {
    // Do nothing if not parallel
    if (!the_config->is_parallel) {
        return;
    }
    // Send terminate

    //typedef struct {
    //    uint8_t processes_count;
    //    pid_t main_process_pid;
    //    pid_t source_lister_pid;
    //    pid_t destination_lister_pid;
    //    pid_t *source_analyzers_pids;
    //    pid_t *destination_analyzers_pids;
    //    key_t shared_key;
    //    int message_queue_id;
    //} process_context_t;

    //typedef struct {
    //    char source[STR_MAX];
    //    char destination[STR_MAX];
    //    uint8_t processes_count;
    //    bool is_parallel;
    //    bool uses_md5;
    //    bool is_dry_run;
    //    bool is_verbose;
    //} configuration_t;

    int recipient = p_context->source_lister_pid;
    if (send_terminate_command(p_context->message_queue_id, recipient) < 0) {
        perror("send_terminate_command");
    }

    // Wait for responses
    any_message_t message;
    if (msgrcv(p_context->message_queue_id, &message, sizeof(any_message_t), recipient, 0) < 0) {
        perror("msgrcv");
    }
    recipient = p_context->destination_lister_pid;
    if (send_terminate_command(p_context->message_queue_id, recipient) < 0) {
        perror("send_terminate_command");
    }
    if (msgrcv(p_context->message_queue_id, &message, sizeof(any_message_t), recipient, 0) < 0) {
        perror("msgrcv");
    }

    for (int i = 0; i < the_config->processes_count; i++) {
        recipient = p_context->source_analyzers_pids[i];
        if (send_terminate_command(p_context->message_queue_id, recipient) < 0) {
            perror("send_terminate_command");
        }
        if (msgrcv(p_context->message_queue_id, &message, sizeof(any_message_t), recipient, 0) < 0) {
            perror("msgrcv");
        }
        recipient = p_context->destination_analyzers_pids[i];
        if (send_terminate_command(p_context->message_queue_id, recipient) < 0) {
            perror("send_terminate_command");
        }
        if (msgrcv(p_context->message_queue_id, &message, sizeof(any_message_t), recipient, 0) < 0) {
            perror("msgrcv");
        }
    }

    // Free allocated memory
    free(p_context->source_analyzers_pids);
    free(p_context->destination_analyzers_pids);
    // Destroy message queue
    if (msgctl(p_context->message_queue_id, IPC_RMID, NULL) < 0) {
        perror("msgctl");
    }


}