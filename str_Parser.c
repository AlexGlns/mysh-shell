#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include "str_Parser.h"

#define READ 0      // read end for read
#define WRITE 1     // write end for pipe
#define PERMS 0644  // set access permissions

char *commandParser(char *cmd, char **cmdParsed, int *filedesc, int *p)
{
    char delim[] = " ";
    char *word;

    word = strtok(cmd, delim);
    int counter = 0;
    while (word != NULL)
    {
        if (strcmp(word, ">") == 0) // stdout redirection
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

            word = strtok(NULL, delim);
            //}
            if (word == NULL)
            {
                break;
            }
        }

        if (strcmp(word, "<") == 0)
        { // stdin redirection
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

        if (strcmp(word, ">>") == 0) // append to end of file
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

        if (strcmp(word, "|") == 0) {      //execute pipe
            *p=1;
            
            //counter++;
            cmdParsed[counter] = NULL;

            word = strtok(NULL, delim);
            if (word == NULL) {
                strcpy(cmdParsed[counter], "|");
                counter++;
                break;
            }
            //store second command
            char* cmd1[strlen(cmd)+1];
            //int counter1 =0;

            // find command write of pipe
            // while (word != NULL)
            // {
            //     strcpy(cmd1[counter], word);
            //     word = strtok(NULL, delim);
            //     counter1++;
            // }
            *cmd1 = commandParser(word,cmd1,filedesc,p);
            printf("%s  %s\n",cmd1[0],cmd1[1]);
            printf("%s  %s\n",cmdParsed[0],cmdParsed[1]);


            int fds[2]; // file desc to open Pipe
            pipe(fds);
            pid_t pid = fork();
            if (pid == -1){
                perror("fork");
            }

            if (pid == 0){  //child execute command after pipe
                close(fds[WRITE]);
                dup2(fds[READ],0);
                execvp(cmd1[0],cmd1);
                perror("exec1");
                //return -1;
            }else{
                close(fds[READ]);
                dup2(fds[WRITE],1);
                execvp(cmdParsed[0],cmdParsed);
                perror("exec2");
                //return -1;
            }

            break;
        }

        if (checkSpecialChars(word))
        {
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

int checkSpecialChars(char *wd)
{
    char *spChars[] = {">", "<", ">>"};
    for (int i = 0; i < 3; i++)
    {
        if (strcmp(wd, spChars[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}