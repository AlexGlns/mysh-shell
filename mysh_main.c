#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#define LENGTH 1024

char *commandParser(char *, char **);

int main()
{
    pid_t pid;

    while (1)
    {
        int status;
        char command[LENGTH], ch; // input can be at most 100 chars
        printf("in-mysh-now:>");

        ch = getchar(); // read input character by character
        int i = 0;
        while (ch != '\n')
        { // read until newLine
            command[i] = ch;
            i++;
            ch = getchar();
        }
        command[i] = '\0';
        if (strcmp(command, "exit") == 0)
        {
            return 1;
        }

        pid = fork();
        if (pid == -1)
        {
            perror("Failed to fork");
            continue;
        }

        //char *parsedCmd[strlen(command) + 1];
        char **parsedCmd = NULL;
        parsedCmd =(char **) malloc(sizeof(char*) * strlen(command)+1);
        for(int j=0;j<strlen(command)+1;j++){
            parsedCmd[j]=malloc(sizeof(char) * strlen(command)+1);
        }
        *parsedCmd = commandParser(command, parsedCmd);
        printf("%s %s\n", parsedCmd[0], parsedCmd[1]);

        if (pid == 0)
        { // child process

            execvp(parsedCmd[0], parsedCmd);
            perror("execvp");
            exit(1);
        }
        else
        { // parent process is mysh shell
            wait(&status);
        }
        
        //free parsedStr
        for(int j=0;j<strlen(command)+1;j++){
            free(parsedCmd[j]);
        }
        free(parsedCmd);
    }
}

char *commandParser(char *cmd, char **cmdParsed)
{
    // char cmdParsed[strlen(cmd)+1][strlen(cmd)+1];
    char delim[] = " ";
    char *word;

    word = strtok(cmd, delim);
    int counter = 0;
    while (word != NULL)
    {
        strcpy(cmdParsed[counter], word);
        word = strtok(NULL, delim);
        counter++;
    }

    cmdParsed[counter] = NULL;

    return *cmdParsed;
}
