#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* get_resource(char* filename) {
    json_error_t error;
    json_t *root = json_load_file(filename, 0, &error);
    if (!root) {
        fprintf(stderr, "Error loading JSON file: %s\n", error.text);
        exit(1);
    }

    json_t *local = json_object_get(json_object_get(json_object_get(root, "files"), "local"), "refs");
    const char *resource = json_string_value(json_object_get(local, "resource"));
    if (!resource) {
        fprintf(stderr, "Failed to extract value of resource field.\n");
        exit(1);
    }

    char* result = malloc(strlen(resource) + 1);
    strcpy(result, resource);
    json_decref(root);
    return result;
}

int main() {
    char* resource = get_resource("input.json");
    printf("Resource: %s\n", resource);
    free(resource);
    return 0;
}
