#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX 1024

char **parse_command(char *str, int *is_bg)
{
    str[strcspn(str, "\n")] = 0; //removes \n from input line
    char **argv = malloc(MAX * sizeof(char *));
    char *token = strtok(str, " "); // initial tokenization

    int i;
    for (i = 0; token != NULL; i++)
    {
        //printf("@i = %d, %s", i, token);
        argv[i] = malloc(16 * sizeof(char));
        if (strcmp(token, "&") == 0) //if & is found, set background flag to 1 and break loop
        {
            *is_bg = 1;
            break;
        } else
        {
            argv[i] = token;
        }
        token = strtok(NULL, " ");
    }
    argv[i] = NULL;
    /*
    for (i = 0; argv[i] != NULL; i++)
        printf("@i = %d, %s", i, argv[i]);
    */
    return argv;
}

void handler(int s)
{
    time_t t = time(NULL); // for timestamp
    struct tm tm = *localtime(&t);

    FILE *fptr;
    fptr = fopen("log.txt", "a"); // appends line to end of log
    fprintf(fptr, "%d-%02d-%02d %02d:%02d:%02d\t\tChild process was terminated\n", tm.tm_year + 1900, tm.tm_mon + 1,
            tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    fclose(fptr);
}

int main()
{
    printf("Ubuntu™ [Version 20.04.3]\n(c) Canonical Ltd. All rights reserved.\n\n");

    while (1)
    {
        /********************READ COMMAND***********************/

        char dir[MAX], command[MAX];
        getcwd(dir, sizeof(dir));
        printf("\n%s\n>> ", dir);
        fgets(command, sizeof(command), stdin); //read command from user input

        /********************PARSE COMMAND**********************/

        int is_bg = 0;                                // flag for background process
        char **argv = parse_command(command,
                                    &is_bg); //take argument vector from parse_command function that tokenizes the input command for execvp
        /*
            printf("is_bg = %d\n", is_bg);
        */

        /********************EXECUTE COMMAND********************/

        if (strcasecmp(argv[0], "exit") == 0)
            exit(0);

        pid_t pid = fork(); // child creation
        if (pid < 0)
        {
            perror("A problem has occured with the creation of the child process");
        } else if (pid == 0)
        {
            //***CHILD PROCESS***
            if (execvp(argv[0], argv) == -1) //execution of command using argv strings
                printf("Command \'%s\' not found\n", argv[0]);
        } else
        {
            //***PARENT PROCESS***
            if (is_bg == 0) //if child process is not a background process, the parent must wait
            {
                waitpid(pid, NULL, WUNTRACED);
            }
            struct sigaction sa = {0};
            sa.sa_handler = &handler; // sa handler is set as the handler function written above
            //printf("pid = %d\n", pid);
            sa.sa_flags = SA_RESTART;
            sigaction(SIGCHLD, &sa, NULL); // the function used to handle the signal
        }
    }

    return 0;
}
