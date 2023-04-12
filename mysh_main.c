#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#define LENGTH 1024 // max length of command
#define PERMS 0644  // set access permissions

char *commandParser(char *, char **, int *);
int checkSpecialChars(char *);

int main()
{
    pid_t pid;

    while (1)
    {
        int status, fd = -1;      // fd = file descriptor for potentially opening a file
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
    char delim[] = " ";
    char *word;

    word = strtok(cmd, delim);
    int counter = 0;
    while (word != NULL)
    {
        if (strcmp(word, ">") == 0)     // stdout redirection
        {
            word = strtok(NULL, delim);
            if (word == NULL)
            {
                strcpy(cmdParsed[counter], ">");
                counter++;
                break;
            }
            // while (word != NULL)
            // {
            if ((*filedesc = open(word, O_CREAT | O_RDWR | O_TRUNC, PERMS)) == -1)
            {
                perror("creating");
            }
            dup2(*filedesc, 1);
            // dup2(*filedesc, 2);
            word = strtok(NULL, delim);
            //}
            if (word == NULL)
            {
                break;
            }
        }
        
        if(strcmp(word, "<") == 0) {   //stdin redirection
            word = strtok(NULL, delim);
            if (word == NULL)
            {
                strcpy(cmdParsed[counter], ">");
                counter++;
                break;
            }
            if ((*filedesc = open(word, O_RDONLY, PERMS)) == -1)
            {
                perror("creating");
            }
            dup2(*filedesc, 0);
            word = strtok(NULL, delim);
            if (word == NULL)
            {
                break;
            }
        }
        
        if (strcmp(word, ">>") == 0)   //append to end of file
        {
            word = strtok(NULL, delim);
            if (word == NULL)
            {
                strcpy(cmdParsed[counter], ">>");
                counter++;
                break;
            }
            if ((*filedesc = open(word, O_CREAT | O_APPEND | O_RDWR, PERMS)) == -1)
            {
                perror("creating");
            }
            dup2(*filedesc, 1);
            word = strtok(NULL, delim);
            if (word == NULL)
            {
                break;
            }
        }
        if (checkSpecialChars(word)){
            continue;
        }
        // printf("1 \n");
        strcpy(cmdParsed[counter], word);
        word = strtok(NULL, delim);
        // printf("%s  \n", word);

        counter++;
    }

    cmdParsed[counter] = NULL;
    // printf("%s %s\n", cmdParsed[0], cmdParsed[1]);

    return *cmdParsed;
}

int checkSpecialChars(char *wd){
    char* spChars[] = {">","<",">>"};
    for (int i=0;i<3;i++) {
        if(strcmp(wd,spChars[i])==0){
            return 1;
        }
    }
    return 0;
}
