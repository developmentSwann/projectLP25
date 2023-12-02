#include <configuration.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

typedef enum { DATE_SIZE_ONLY, NO_PARALLEL } long_opt_values;

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
    the_config->is_parallel = true;
    the_config->uses_md5 = true;
    the_config->source[0] = '\0';
    the_config->destination[0] = '\0';
}

/*!
 * @brief are_parameters_valid tests if the parameters are valid
 * @param parameter
 * @return true if parameters are valid, false else
 * This function is provided with its code, you don't have to implement nor modify it.
 */
bool are_parameters_valid(parameter_t *params, int params_count) {
    for (int i = 0; i < params_count; i++) {
        switch (params[i].parameter_type) {
            case LPARAM_SOURCE:
            case LPARAM_DESTINATION:
                if (params[i].parameter_value.str_param[0] == '\0') {
                    return false;
                }
                break;
            case LPARAM_DATE_SIZE_ONLY:
            case LPARAM_NO_PARALLEL:
            case PARAM_VERBOSE:
            case PARAM_CPU_MULT:
                if (params[i].parameter_value.int_param < 1 || params[i].parameter_value.int_param > 4) {
                    return false;
                }
                break;

            default:
                break;
        }
    }

    return true;
}

/*!
 * @brief set_configuration updates a configuration based on options and parameters passed to the program CLI
 * @param the_config is a pointer to the configuration to update
 * @param argc is the number of arguments to be processed
 * @param argv is an array of strings with the program parameters
 * @return -1 if configuration cannot succeed, 0 when ok
 */
int set_configuration(configuration_t *the_config, int argc, char *argv[]) {
    int opt;
    parameter_t default_parameters[] = {
            {.parameter_type = PARAM_VERBOSE, .parameter_value.flag_param = false},
            {.parameter_type = PARAM_CPU_MULT, .parameter_value.int_param = 1},
            {.parameter_type = LPARAM_DATE_SIZE_ONLY, .parameter_value.flag_param = false},
            {.parameter_type = LPARAM_NO_PARALLEL, .parameter_value.flag_param = false},
            {.parameter_type = LPARAM_SOURCE, .parameter_value.str_param = ""},
            {.parameter_type = LPARAM_DESTINATION, .parameter_value.str_param = ""},
    };
    int default_parameters_count = sizeof(default_parameters) / sizeof(parameter_t);

    struct option my_opts[] = {
            {.name = "verbose", .has_arg = 0, .flag = 0, .val = 'v'},
            {.name = "cpu-mult", .has_arg = 1, .flag = 0, .val = 'm'},
            {.name = "date-size-only", .has_arg = 0, .flag = 0, .val = 't'},
            {.name = "no-parallel", .has_arg = 0, .flag = 0, .val = 'p'},
            {.name = "source", .has_arg = 1, .flag = 0, .val = 's'},
            {.name = "destination", .has_arg = 1, .flag = 0, .val = 'd'},
            {.name = NULL, .has_arg = 0, .flag = 0, .val = 0},
    };

    if (!are_parameters_valid(default_parameters, default_parameters_count)) {
        fprintf(stderr, "Error: Les paramètres par défaut ne sont pas valides.\n");
        return -1;
    }

    while ((opt = getopt_long(argc, argv, "", my_opts, NULL)) != -1) {
        switch (opt) {
            case 'v':
                default_parameters[0].parameter_value.flag_param = true;
                break;

            case 'm':
                default_parameters[1].parameter_value.int_param = strtoul(optarg, NULL, 10);
                if (default_parameters[1].parameter_value.int_param < 1 ||
                    default_parameters[1].parameter_value.int_param > 4) {
                    fprintf(stderr, "Error: La valeur entrée pour l'option -m n'est pas valide.\n");
                    return -1;
                }
                break;

            case 't':
                default_parameters[2].parameter_value.flag_param = true;
                break;

            case 'p':
                default_parameters[3].parameter_value.flag_param = true;
                break;

            case 's':
                strncpy(default_parameters[4].parameter_value.str_param, optarg, sizeof(default_parameters[4].parameter_value.str_param) - 1);
                break;

            case 'd':
                strncpy(default_parameters[5].parameter_value.str_param, optarg, sizeof(default_parameters[5].parameter_value.str_param) - 1);
                break;

            default:
                fprintf(stderr, "Error: Option inconnue %c.\n", opt);
                return -1;
        }
    }

    return 0;
}
