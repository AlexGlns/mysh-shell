#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <glob.h>
#include "str_Parser.h"

#define READ 0      // read end for read
#define WRITE 1     // write end for pipe
#define PERMS 0644  // set access permissions

char ** wildchar(glob_t *, char* );

char *commandParser(char *cmd, char **cmdParsed, int *filedesc, int *p)
{
    glob_t gstruct;
    char **wildch;
    char delim[] = " \"";
    char *word;

    word = strtok(cmd, delim);
    int counter = 0;
    while (word != NULL)
    {
        char tmp;
        int j=0, wdchar = 0;    //wdchar == 1 if wildcharachter found
        while (word[j]!='\0'){
            if (word[j] == '*'){  //if you find a wild char call function wildchar
               wildch = wildchar(&gstruct, word);
               wdchar = 1;
               if (wildch != NULL) {
                while(*wildch) {
                    strcpy(cmdParsed[counter], *wildch);
                    wildch++;
                    counter++;
                }
               }
               break;       
            }
            j++;
        }

        if (wdchar == 1) {
            word = strtok(NULL, delim);
            continue;
        }

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

        if (strcmp(word,"&") == 0) {
            word = strtok(NULL, delim);
            continue;
        }
        // printf("1 \n");
        strcpy(cmdParsed[counter], word);
        word = strtok(NULL, delim);
        // printf("%s  \n", word);

        counter++;
    }

    cmdParsed[counter] = NULL;
    //printf("%s %s\n", cmdParsed[0], cmdParsed[1]);

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

char ** wildchar(glob_t *gstruct, char* wd) {
    int r;

    r = glob(wd, GLOB_ERR, NULL, gstruct);

    if (r==0) {
        return gstruct->gl_pathv;
    }

    return NULL;
}