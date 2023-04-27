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

            if ((*filedesc = open(word, O_CREAT | O_RDWR | O_TRUNC, PERMS)) == -1)
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
            char nextCmd[1024];
            char** cmd1 = NULL;


            //int counter1 =0;

            // find command write of pipe
            while (word != NULL)
            {
                strcat(nextCmd, word);
                word = strtok(NULL, delim);
                if (word != NULL) {
                    strcat(nextCmd, " ");
                }
                //counter1++;
            }
            cmd1 = (char **)malloc(sizeof(char *) * strlen(nextCmd) + 1);
            for (int j = 0; j < strlen(nextCmd) + 1; j++)
            {
                cmd1[j] = malloc(sizeof(char) * strlen(nextCmd) + 1);
            }
            int newFd = -1;  //new filedescriptor
            *cmd1 = commandParser(nextCmd,cmd1,&newFd,p);
            // printf("%s  %s\n",cmd1[0],cmd1[1]);
            // printf("%s \n",nextCmd);


            int fds[2]; // file desc to open Pipe
            if (pipe(fds) < 0) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            pid_t pid = fork();
            if (pid == -1){
                perror("fork");
            }

            if (pid == 0){  //child execute command after pipe
                close(fds[WRITE]);
                dup2(fds[READ],0);
                execvp(cmd1[0],cmd1);
                perror("exec1");
                exit(EXIT_FAILURE);
            }else{
                dup2(fds[WRITE],1);
                execvp(cmdParsed[0],cmdParsed);
                perror("exec2");
                exit(EXIT_FAILURE);
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
        strcpy(cmdParsed[counter], word);
        word = strtok(NULL, delim);

        counter++;
    }

    cmdParsed[counter] = NULL;

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