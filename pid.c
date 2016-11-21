#include <stdio.h>
void touch_pid(const char * pid_file_name)
{
    FILE * pid_f = NULL;

    if((pid_f = fopen(pid_file_name, "wb")) != NULL) {
        fprintf(pid_f, "%d", getpid());
        fclose(pid_f);
        pid_f = NULL;
    }
}

