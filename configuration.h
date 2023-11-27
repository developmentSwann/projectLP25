#pragma once

#include <stdint.h>
#include <stdbool.h>
#define STR_MAX 1024

typedef struct {
    char source[STR_MAX];
    char destination[STR_MAX];
    uint8_t processes_count;
    bool is_parallel;
    bool uses_md5;
} configuration_t;

typedef enum {
    LPARAM_SOURCE,
    LPARAM_DESTINATION,
    LPARAM_DATE_SIZE_ONLY,
    LPARAM_NO_PARALLEL,
    LPARAM_DRY_RUN,
    PARAM_CPU_MULT,
    PARAM_VERBOSE,

} opt_values;



typedef union{
    char str_param[STR_MAX];
    long long int_param;
    bool flag_param;
} type_arg;


typedef struct {
    opt_values parameter_type;
    type_arg parameter_value;
} parameter_t;

void init_configuration(configuration_t *the_config);
int set_configuration(configuration_t *the_config, int argc, char *argv[]);