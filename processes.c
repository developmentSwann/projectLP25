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
    /*
    if (the_config->is_parallel) {
        p_context->processes_count = the_config->processes_count;
        p_context->main_process_pid = getpid();
        p_context->source_lister_pid = -1;
        p_context->destination_lister_pid = -1;
        p_context->source_analyzers_pids = malloc(sizeof(pid_t) * the_config->processes_count);
        p_context->destination_analyzers_pids = malloc(sizeof(pid_t) * the_config->processes_count);
        p_context->shared_key = ftok(the_config->source, 1);
        p_context->message_queue_id = msgget(p_context->shared_key, IPC_CREAT | 0666);
        if (p_context->message_queue_id < 0) {
            perror("msgget");
            return -1;
        }
        lister_configuration_t *source_lister_config = malloc(sizeof(lister_configuration_t));
        source_lister_config->my_recipient_id = SOURCE_LISTER_ID;
        source_lister_config->my_receiver_id = SOURCE_LISTER_ID;
        source_lister_config->analyzers_count = the_config->processes_count;
        source_lister_config->mq_key = p_context->shared_key;
        p_context->source_lister_pid = make_process(p_context, lister_process_loop, source_lister_config);
        if (p_context->source_lister_pid < 0) {
            perror("make_process");
            return -1;
        }
        lister_configuration_t *destination_lister_config = malloc(sizeof(lister_configuration_t));
        destination_lister_config->my_recipient_id = DESTINATION_LISTER_ID;
        destination_lister_config->my_receiver_id = DESTINATION_LISTER_ID;
        destination_lister_config->analyzers_count = the_config->processes_count;
        destination_lister_config->mq_key = p_context->shared_key;
        p_context->destination_lister_pid = make_process(p_context, lister_process_loop, destination_lister_config);
        if (p_context->destination_lister_pid < 0) {
            perror("make_process");
            return -1;
        }
        for (int i = 0; i < the_config->processes_count; i++) {
            analyzer_configuration_t *source_analyzer_config = malloc(sizeof(analyzer_configuration_t));
            source_analyzer_config->my_recipient_id = SOURCE_ANALYZER_ID;
            source_analyzer_config->my_receiver_id = SOURCE_LISTER_ID;
            source_analyzer_config->mq_key = p_context->shared_key;
            source_analyzer_config->use_md5 = the_config->uses_md5;
            p_context->source_analyzers_pids[i] = make_process(p_context, analyzer_process_loop, source_analyzer_config);
            if (p_context->source_analyzers_pids[i] < 0) {
                perror("make_process");
                return -1;
            }
        }
    }
    return 0;
    */
}

/*!
 * @brief make_process creates a process and returns its PID to the parent
 * @param p_context is a pointer to the processes context
 * @param func is the function executed by the new process
 * @param parameters is a pointer to the parameters of func
 * @return the PID of the child process (it never returns in the child process)
 */
int make_process(process_context_t *p_context, process_loop_t func, void *parameters) {
    /*
        pid_t pid = fork();
        if (pid < 0) {
            // Fonction fork a echoue.
            return -1;
        } else if (pid == 0) {
            // On est dans le processus fils et on execute la fonction.
            func(parameters);
            exit(0);
        } else {
            // On est dans le processus parents et on retourne le pid du fils.
            return pid;
        }
}
     */
}
/*!
 * @brief lister_process_loop is the lister process function (@see make_process)
 * @param parameters is a pointer to its parameters, to be cast to a lister_configuration_t
 */
void lister_process_loop(void *parameters) {
    /*
    lister_configuration_t *config = (lister_configuration_t *) parameters;
    files_list_t *list = malloc(sizeof(files_list_t));
    make_files_list(list, config->path); // TODO
    files_list_entry_t *cursor = list->head;
    while (cursor) {
        printf("%s\n", cursor->path_and_name);
        cursor = cursor->next;
    }
    clear_files_list(list);
    free(list);
*/

}

/*!
 * @brief analyzer_process_loop is the analyzer process function
 * @param parameters is a pointer to its parameters, to be cast to an analyzer_configuration_t
 */
void analyzer_process_loop(void *parameters) {
    /*
    analyzer_configuration_t *config = (analyzer_configuration_t *) parameters;
    files_list_entry_t *entry = malloc(sizeof(files_list_entry_t));
    while (true) {
        if (msgrcv(config->mq_key, entry, sizeof(files_list_entry_t), config->my_receiver_id, 0) < 0) {
            perror("msgrcv");
            exit(1);
        }
        if (entry->entry_type == FICHIER) {
            if (get_file_stats(entry) < 0) {
                perror("get_file_stats");
                exit(1);
            }
            if (config->use_md5) {
                if (compute_file_md5(entry) < 0) {
                    perror("compute_file_md5");
                    exit(1);
                }
            }
        }
        if (send_analyze_file_response(config->mq_key, config->my_recipient_id, entry) < 0) {
            perror("send_analyze_file_response");
            exit(1);
        }
    }
*/
}

/*!
 * @brief clean_processes cleans the processes by sending them a terminate command and waiting to the confirmation
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the processes context
 */
void clean_processes(configuration_t *the_config, process_context_t *p_context) {
    /*
        simple_command_t terminate_command;
        terminate_command.mtype = MSG_TYPE_TO_SOURCE_LISTER;
        terminate_command.message = COMMAND_CODE_TERMINATE;

        // Send terminate command to source lister
        if (msgsnd(p_context->message_queue_id, &terminate_command, sizeof(simple_command_t), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }

        // Wait for source lister to terminate
        waitpid(p_context->source_lister_pid, NULL, 0);

        // Update the message type for destination lister
        terminate_command.mtype = MSG_TYPE_TO_DESTINATION_LISTER;

        // Send terminate command to destination lister
        if (msgsnd(p_context->message_queue_id, &terminate_command, sizeof(simple_command_t), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }

        // Wait for destination lister to terminate
        waitpid(p_context->destination_lister_pid, NULL, 0);

        // Send terminate command to all source analyzers
        terminate_command.mtype = MSG_TYPE_TO_SOURCE_ANALYZERS;
        for (int i = 0; i < the_config->processes_count; i++) {
            if (msgsnd(p_context->message_queue_id, &terminate_command, sizeof(simple_command_t), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }

            // Wait for source analyzer to terminate
            waitpid(p_context->source_analyzers_pids[i], NULL, 0);
        }

        // Send terminate command to all destination analyzers
        terminate_command.mtype = MSG_TYPE_TO_DESTINATION_ANALYZERS;
        for (int i = 0; i < the_config->processes_count; i++) {
            if (msgsnd(p_context->message_queue_id, &terminate_command, sizeof(simple_command_t), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }

            // Wait for destination analyzer to terminate
            waitpid(p_context->destination_analyzers_pids[i], NULL, 0);
        }
        */
}