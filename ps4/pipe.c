#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

int main(void)
{
    int filedes[2];
    pipe(filedes);

    /* Run LS. */
    pid_t pid = fork();
    if (pid == 0) {
        /* Set stdout to the input side of the pipe, and run 'ls'. */
        dup2(filedes[1], 1);
        char *argv[] = {"ls", NULL};
        execv("/bin/ls", argv);
    } else {
        /* Close the input side of the pipe, to prevent it staying open. */
        close(filedes[1]);
    }

    /* Run WC. */
    pid = fork();
    if (pid == 0) {
        dup2(filedes[0], 0);
        char *argv[] = {"wc", NULL};
        execv("/usr/bin/wc", argv);
    }

    /* Wait for WC to finish. */
    waitpid(pid);
}
