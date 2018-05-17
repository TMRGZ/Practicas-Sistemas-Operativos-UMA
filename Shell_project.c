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

job *registroProcesos;


void manejador(int senal) {
    printf("Verificando Cola \n");
    int pidWait;


    for (int i = 0; i < list_size(registroProcesos); ++i) {
        pidWait = waitpid(get_item_bypos(registroProcesos, i)->pgid, (int *) get_item_bypos(registroProcesos, i)->state,
                          WNOHANG);
        delete_job(registroProcesos, get_item_bypos(registroProcesos, i));
    }
}

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
    registroProcesos = new_list("Procesos");  // Lista Procesos


    signal(SIGCHLD, manejador);

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
                new_process_group(getpid());
                restore_terminal_signals();

                if (!background) {  // Segundo Plano?
                    set_terminal(getpid());
                } else {
                    job *segundoPlano = new_job(getpid(), args[0], BACKGROUND);
                    add_job(registroProcesos, segundoPlano);
                }


                execvp(inputBuffer, args);
                printf("Error, command not found: %s \n", args[0]);
                exit(-1);
            } else {// Proceso Padre
                new_process_group(pid_fork);

                if (background) {
                    printf("Background job running... pid: %d, command: %s \n", pid_fork, args[0]);
                } else {
                    set_terminal(pid_fork);
                    pid_wait = waitpid(pid_fork, &status, WUNTRACED);
                    set_terminal(getpid());
                    status_res = analyze_status(status, &info);

                    if (strcmp(status_strings[status_res], "Suspended") == 0) {
                        job *suspendido = new_job(getpid(), args[0], BACKGROUND);
                        add_job(registroProcesos, suspendido);
                    }


                    printf("Foreground pid: %d, command: %s, %s, info; %d \n", pid_wait, args[0],
                           status_strings[status_res], info);
                }
            }
        }
    } // end while
}
