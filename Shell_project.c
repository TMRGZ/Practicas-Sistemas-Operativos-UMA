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
    int status; // Estado del proceso, no vale para mucho
    enum status status_res;     // Resultado que dara el analyze_status
    int pid;                    // pid proceso a manejar
    int info;                   // Info necesaria
    pid_t pg;                   //
    job *t;                     //

    for (int i = 1; i <= list_size(registroProcesos); ++i) {
        t = get_item_bypos(registroProcesos, i);
        pg = t->pgid;
        pid = waitpid(pg, &status, WUNTRACED | WNOHANG);
        status_res = analyze_status(status, &info);

        if (pid == pg) {
            //printf("%s \n", status_strings[status_res]);

            if (strcmp(status_strings[status_res], "Suspended") == 0) {
                t->state = STOPPED;
            } else {
                delete_job(registroProcesos, t);
            }
        }
    }
}

int selectComandoInterno(char **cmd) {
    int i;

    if (strcmp(cmd[0], "cd") == 0) {
        chdir(cmd[1]);
        return 1;
    } else if (strcmp(cmd[0], "jobs") == 0) {
        if (empty_list(registroProcesos)) {
            printf("Lista vacia \n");
        } else {
            print_job_list(registroProcesos);
        }
        return 1;
    } else if (strcmp(cmd[0], "fg") == 0) {
        int status;
        int info;

        if (cmd[1] == NULL) cmd[1] = "1";
        sscanf(cmd[1], "%d", &i);
        job *t = get_item_bypos(registroProcesos, i);
        pid_t pg = t->pgid;
        char copiaCMD[20];
        strcpy(copiaCMD, t->command);

        killpg(pg, SIGCONT);

        set_terminal(pg);
        int pid_wait = waitpid(pg, &status, WUNTRACED);
        delete_job(registroProcesos, t);
        set_terminal(getpid());

        enum status status_res = analyze_status(status, &info);

        if (strcmp(status_strings[status_res], "Suspended") == 0) {
            job *suspendido = new_job(pid_wait, copiaCMD, STOPPED);
            add_job(registroProcesos, suspendido);
        }


        return 1;
    } else if (strcmp(cmd[0], "bg") == 0) {
        if (cmd[1] == NULL) cmd[1] = "1";
        sscanf(cmd[1], "%d", &i);
        job *t = get_item_bypos(registroProcesos, i);
        pid_t pg = t->pgid;
        t->state = BACKGROUND;
        killpg(pg, SIGCONT);

        return 1;
    }

    return 0;
}


int main(void) {
    char inputBuffer[MAX_LINE];                 /* buffer to hold the command entered */
    int background;                             /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE / 2];                   /* command line (of 256) has max of 128 arguments */
    // probably useful variables:
    int pid_fork, pid_wait;                     /* pid for created and waited process */
    int status;                                 /* status returned by wait */
    enum status status_res;                     /* status processed by analyze_status() */
    int info;                                   /* info processed by analyze_status() */
    registroProcesos = new_list("Procesos");    // Lista Procesos


    signal(SIGCHLD, manejador);

    while (1)                                   /* Program terminates normally inside get_command() after ^D is typed*/
    {
        printf("COMMAND->");
        fflush(stdout);
        ignore_terminal_signals();
        get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */

        if (args[0] == NULL) continue;              // if empty command

        if (!selectComandoInterno(args)) {          // Comando interno?
            pid_fork = fork();                      // Nuevo Proceso

            if (pid_fork == 0) {                    // Proceso Hijo
                new_process_group(getpid());
                restore_terminal_signals();

                if (!background) {                  // Segundo Plano?
                    set_terminal(getpid());
                }

                execvp(inputBuffer, args);
                printf("Error, command not found: %s \n", args[0]);
                exit(-1);
            } else {// Proceso Padre
                new_process_group(pid_fork);

                if (background) {
                    printf("Background job running... pid: %d, command: %s \n", pid_fork, args[0]);
                    job *segundoPlano = new_job(pid_fork, args[0], BACKGROUND);
                    add_job(registroProcesos, segundoPlano);
                } else {
                    set_terminal(pid_fork);
                    pid_wait = waitpid(pid_fork, &status, WUNTRACED);
                    set_terminal(getpid());
                    status_res = analyze_status(status, &info);

                    if (strcmp(status_strings[status_res], "Suspended") == 0) {
                        job *suspendido = new_job(pid_wait, args[0], STOPPED);
                        add_job(registroProcesos, suspendido);
                    }


                    printf("Foreground pid: %d, command: %s, %s, info; %d \n", pid_wait, args[0],
                           status_strings[status_res], info);
                }
            }
        }
    } // end while
}
