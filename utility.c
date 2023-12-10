#include <defines.h>
#include <string.h>

/*!
 * @brief concat_path concatenates suffix to prefix into result
 * It checks if prefix ends by / and adds this token if necessary
 * It also checks that result will fit into PATH_SIZE length
 * @param result the result of the concatenation
 * @param prefix the first part of the resulting path
 * @param suffix the second part of the resulting path
 * @return a pointer to the resulting path, NULL when concatenation failed
 */
char *concat_path(char *result, char *prefix, char *suffix) {
    // String final = prefix + "/" + suffixFinal
    // suffixFinal = différence entre suffix et prefix
    // On vérifie que le résultat ne dépasse pas PATH_SIZE
    if (strlen(prefix) + strlen(suffix) + 1 > PATH_SIZE) {
        return NULL;
    }
    char *cursor = prefix;
    char *suffixFinal;
    while (*cursor == *suffix) {
        cursor++;
        suffix++;
    }
    suffixFinal = suffix;
   //On concatene le prefixe et le suffixe
    strcpy(result, prefix);
    if (prefix[strlen(prefix) - 1] != '/') {
        strcat(result, "/");
    }
    strcat(result, suffixFinal);
    return result;



}
