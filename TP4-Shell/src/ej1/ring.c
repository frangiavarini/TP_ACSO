#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Uso correcto: %s <num_procesos> <mensaje_inicial> <proceso_inicio>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_procs = atoi(argv[1]);
    int mensaje = atoi(argv[2]);
    int inicio = atoi(argv[3]);

    if (num_procs < 3) {
        fprintf(stderr, "Error: se requieren al menos 3 procesos.\n");
        exit(EXIT_FAILURE);
    }
    if (inicio < 1 || inicio > num_procs) {
        fprintf(stderr, "Error: proceso inicial debe estar entre 1 y %d.\n", num_procs);
        exit(EXIT_FAILURE);
    }

   
    int comunicacion[num_procs][2];
    for (int i = 0; i < num_procs; i++) {
        if (pipe(comunicacion[i]) == -1) {
            perror("Error creando pipe");
            exit(EXIT_FAILURE);
        }
    }

    
    int pipe_padre[2];
    if (pipe(pipe_padre) == -1) {
        perror("Error creando pipe padre");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i <= num_procs; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error creando proceso hijo");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { 
            
            for (int j = 0; j < num_procs; j++) {
                if (j != i - 1) close(comunicacion[j][0]);
                if (j != i % num_procs) close(comunicacion[j][1]); 
            }
           
            if (i == inicio) {
                close(pipe_padre[1]); // solo lee del padre
            } else {
                close(pipe_padre[0]);
                close(pipe_padre[1]);
            }

            int valor;
            if (i == inicio) {
                read(pipe_padre[0], &valor, sizeof(int));
                close(pipe_padre[0]);
            } else {
                read(comunicacion[i - 1][0], &valor, sizeof(int));
            }

            valor++;

            write(comunicacion[i % num_procs][1], &valor, sizeof(int));

            if (i == inicio) {
                read(comunicacion[i - 1][0], &valor, sizeof(int));
                printf("Resultado final: %d\n", valor);
            }

            close(comunicacion[i - 1][0]);
            close(comunicacion[i % num_procs][1]);

            exit(EXIT_SUCCESS);
        }
    }

   
    for (int i = 0; i < num_procs; i++) {
        close(comunicacion[i][0]);
        close(comunicacion[i][1]);
    }

 
    close(pipe_padre[0]);
    write(pipe_padre[1], &mensaje, sizeof(int));
    close(pipe_padre[1]);

    
    for (int i = 0; i < num_procs; i++) {
        wait(NULL);
    }

    return 0;
}
