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
#include <string.h>

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

// -----------------------------------------------------------------------
//                            MAIN          
// -----------------------------------------------------------------------

int selectComandoInterno(char **cmd) {
    if (strcmp(cmd[0], "cd") == 0) {
        chdir(cmd[1]);
        return 1;
    }
    return 0;
}


int main(void) {
    char inputBuffer[MAX_LINE];     /* buffer to hold the command entered */
    int background;                 /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE / 2];       /* command line (of 256) has max of 128 arguments */
    // probably useful variables:
    int pid_fork, pid_wait;         /* pid for created and waited process */
    int status;                     /* status returned by wait */
    enum status status_res;         /* status processed by analyze_status() */
    int info;                       /* info processed by analyze_status() */

    while (1)                       /* Program terminates normally inside get_command() after ^D is typed*/
    {
        printf("COMMAND->");
        fflush(stdout);
        ignore_terminal_signals();
        get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */

        if (args[0] == NULL) continue;          // if empty command

        if (!selectComandoInterno(args)) {       // Comando interno?
            pid_fork = fork();                  // Nuevo Proceso

            if (pid_fork == 0) {                // Proceso Hijo
                new_process_group(pid_fork);


                if (!background) {               // Segundo Plano?
                    restore_terminal_signals();
                    set_terminal(pid_fork);

                }

                execvp(inputBuffer, args);
                printf("Error, command not found: %s \n", args[0]);
                exit(-1);

            } else {                            // Proceso Padre
                if (background) {
                    printf("Background job running... pid: %d, command: %s \n", pid_fork, args[0]);
                } else {
                    pid_wait = waitpid(pid_fork, &status, 0);
                    set_terminal(getpid());
                    status_res = analyze_status(status, &info);
                    printf("Foreground pid: %d, command: %s, %s, info; %d \n", pid_wait, args[0],
                           status_strings[status_res], info);
                }
            }
        }
    } // end while
}
