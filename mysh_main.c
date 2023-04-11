#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#define LENGTH 1024 // max length of command
#define PERMS 0644  // set access permissions

char *commandParser(char *, char **, int *);

int main()
{
    pid_t pid;

    while (1)
    {
        int status, fd = -1;
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

        // char *parsedCmd[strlen(command) + 1];
        char **parsedCmd = NULL;
        parsedCmd = (char **)malloc(sizeof(char *) * strlen(command) + 1);
        for (int j = 0; j < strlen(command) + 1; j++)
        {
            parsedCmd[j] = malloc(sizeof(char) * strlen(command) + 1);
        }
        //*parsedCmd = commandParser(command, parsedCmd, &fd);
        // printf("%s %s\n", parsedCmd[0], parsedCmd[1]);

        if (pid == 0)
        {                                                        // child process
            *parsedCmd = commandParser(command, parsedCmd, &fd); // parse command

            execvp(parsedCmd[0], parsedCmd);
            perror("execvp");
            exit(1);
        }
        else
        { // parent process is mysh shell
            wait(&status);
        }

        // free parsedStr
        for (int j = 0; j < strlen(command) + 1; j++)
        {
            free(parsedCmd[j]);
        }
        if (fd != -1)
        {
            close(fd);
        }
        free(parsedCmd);
    }
}

char *commandParser(char *cmd, char **cmdParsed, int *filedesc)
{
    // char cmdParsed[strlen(cmd)+1][strlen(cmd)+1];
    char delim[] = " ";
    char *word;
    // int createFile = 0; // int variable 1 or 0 to know if we need to open a file

    word = strtok(cmd, delim);
    int counter = 0;
    while (word != NULL)
    {
        if (strcmp(word, ">") == 0)
        {
            word = strtok(NULL, delim);
            while (word != NULL)
            {
                // printf("in while loop \n");
                if ((*filedesc = open(word, O_CREAT | O_RDWR, PERMS)) == -1)
                {
                    perror("creating");
                }
                dup2(*filedesc, 1);
                dup2(*filedesc, 2);
                word = strtok(NULL, delim);
            }
            //cmdParsed[counter] = NULL;
            //printf("returning \n");
            //return *cmdParsed;

            break;
        }
        //printf("1 \n");
        strcpy(cmdParsed[counter], word);
        word = strtok(NULL, delim);
        //printf("%s  \n", word);

        counter++;
    }

    cmdParsed[counter] = NULL;
    //printf("%s %s\n", cmdParsed[0], cmdParsed[1]);

    return *cmdParsed;
}
