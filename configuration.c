#include <configuration.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

typedef enum {DATE_SIZE_ONLY, NO_PARALLEL} long_opt_values;

/*!
 * @brief function display_help displays a brief manual for the program usage
 * @param my_name is the name of the binary file
 * This function is provided with its code, you don't have to implement nor modify it.
 */
void display_help(char *my_name) {
    printf("%s [options] source_dir destination_dir\n", my_name);
    printf("Options: \t-n <processes count>\tnumber of processes for file calculations\n");
    printf("         \t-h display help (this text)\n");
    printf("         \t--date_size_only disables MD5 calculation for files\n");
    printf("         \t--no-parallel disables parallel computing (cancels values of option -n)\n");
}

/*!
 * @brief init_configuration initializes the configuration with default values
 * @param the_config is a pointer to the configuration to be initialized
 */
void init_configuration(configuration_t *the_config) {
    the_config->processes_count = 1;
    the_config->is_parallel = false;
    the_config->uses_md5 = false;
    the_config->source[0] = '\0';
    the_config->destination[0] = '\0';
    the_config->is_dry_run = false;
    the_config->is_verbose = false;
}





/*!
 * @brief set_configuration updates a configuration based on options and parameters passed to the program CLI
 * @param the_config is a pointer to the configuration to update
 * @param argc is the number of arguments to be processed
 * @param argv is an array of strings with the program parameters
 * @return -1 if configuration cannot succeed, 0 when ok
 */
int set_configuration(configuration_t *the_config, int argc, char *argv[]){
    //Check si y'a au moins le chemin destination et source
    if(argc < 3){
        printf("Il manque des arguments\n");
        return -1;
    }
    //Check si la taille des chemins est pas trop grande
    if(strlen(argv[1]) > STR_MAX || strlen(argv[2]) > STR_MAX){
        printf("Les chemins sont trop longs\n");
        return -1;
    }
    //Check si les chemins sont pas les mêmes
    if(strcmp(argv[1], argv[2]) == 0){
        printf("Les chemins sont les mêmes\n");
        return -1;
    }
    strcpy(the_config->source, argv[1]);
    strcpy(the_config->destination, argv[2]);

    int opt;

    struct option my_opts[] = {
            {.name="verbose",.has_arg=0,.flag=0,.val='v'},
            {.name="cpu-mult",.has_arg=1,.flag=0,.val='m'},
            {.name="date-size-only",.has_arg=0,.flag=0,.val='t'},
            {.name="no-parallel",.has_arg=0,.flag=0,.val='p'},
            {.name="is-dry-run",.has_arg=0,.flag=0,.val='d'},
            {.name=NULL,.has_arg=0,.flag=0,.val=0},

    };


    while((opt = getopt_long(argc, argv, "", my_opts, NULL)) != -1) {

        switch (opt) {
            case 'v':
                the_config->is_verbose = true;
                break;

            case 'm':
                the_config->processes_count = atoi(optarg);
                break;

            case 't':
                the_config->uses_md5 = false;
                break;

            case 'p':
                the_config->is_parallel = false;
                break;
            case 'd':
                the_config->is_dry_run = true;
                break;


        }
    }

    return 0;

}
