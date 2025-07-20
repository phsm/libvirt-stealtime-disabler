#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

#define FLAG_TO_MODIFY "-cpu"
#define FLAG_VALUE_TO_ADD ",-kvm-steal-time"
#define CMD_TO_INJECT "qemu-system-"

typedef int (*execve_func_t)(const char *path, char *const argv[], char *const envp[]);
static execve_func_t orig_execve = NULL;

// Save a little bit of resources:
// only exec this when the library is loaded
void __attribute__((constructor)) on_load() {
    orig_execve = dlsym(RTLD_NEXT, "execve");
}

int execve(const char *path, char *const argv[], char *const envp[]) {
    int flag_val_position = -1;
    char *flag_val;
    char **newargv;
    int i; // this also counts args for malloc

    // Only inject argument to specific process
    if (argv[0] && !strstr(argv[0], CMD_TO_INJECT)) return orig_execve(path, argv, envp);
    fprintf(stderr, "[injectflag] attempt to run %s detected that matches the pattern\n", argv[0]);

    for (i = 0; argv[i]; i++) { // when the argv[i] is NULL then the end of the array is reached
        if (flag_val_position == -1 && strcmp(FLAG_TO_MODIFY, argv[i]) == 0) {
            // Found the -cpu flag, thus the next argv member will be the one to modify
            flag_val_position = i+1;
            fprintf(stderr, "[injectflag] the -cpu flag value has found on position %d\n", flag_val_position);
        }
    }

    // if the cpu flag wasn't found then run original func
    if (flag_val_position < 0) return orig_execve(path, argv, envp);


    // at this point, the size of argv is known, thus we can allocate a new argv
    newargv = malloc(sizeof(char*) * (i+1)); // +1 because NULL needs to be put as the last arg
    newargv[i] = NULL; // already terminate the array with NULL

    for (int j = 0; j < i; j++) {
        if (j == flag_val_position) {
            // allocate memory for the new value
            flag_val = (char*)malloc(
                (
                    strlen(argv[flag_val_position])
                    + strlen(FLAG_VALUE_TO_ADD)
                    + 1
                ) * sizeof(char)
            );
            strcpy(flag_val, argv[flag_val_position]);
            strcat(flag_val, FLAG_VALUE_TO_ADD);
            newargv[j] = flag_val;
            fprintf(stderr, "[injectflag] new cpu flags injected: \"%s\"\n", flag_val);
            continue;
        }
        newargv[j] = argv[j];
    }

    // reuse i for the return code
    i = orig_execve(path, newargv, envp);
    free(newargv);
    return i;
}
