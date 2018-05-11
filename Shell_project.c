/**
UNIX Shell Project

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell          
	(then type ^D to exit program)

**/

#include "job_control.h"   // remember to compile with module job_control.c

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------


int main(void) {
    char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
    int background;             /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE / 2];     /* command line (of 256) has max of 128 arguments */
    // probably useful variables:
    int pid_fork, pid_wait;    /* pid for created and waited process */
    int status;                /* status returned by wait */
    enum status status_res;    /* status processed by analyze_status() */
    int info;                    /* info processed by analyze_status() */

    while (1)                /* Program terminates normally inside get_command() after ^D is typed*/
    {
        printf("COMMAND->");
        fflush(stdout);
        get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */

        if (args[0] == NULL) continue;   // if empty command

        pid_fork = fork();

        if (pid_fork == 0) {
            execvp(inputBuffer, args);
            printf("Error, command not found: %s \n", args[0]);
            exit(-1);

        } else {
            if (background) {
                printf("Background job running... pid: %d, command: %s \n", getpid(), args[0]);
            } else {
                pid_wait = waitpid(pid_fork, &status, 0);
                status_res = analyze_status(status, &info);
                printf("Foreground pid: %d, command: %s, %s, info; %d \n", getpid(), args[0],
                       status_strings[status_res], info);
            }
        }
    } // end while
}
