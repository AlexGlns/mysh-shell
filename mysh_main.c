#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include "str_Parser.h"
#define READ 0      // read end for read
#define WRITE 1     // write end for pipe
#define LENGTH 1024 // max length of command
#define PERMS 0644  // set access permissions

void add_alias(char ***, int *, int *, char *);
void sig_handler_terminate();
void sig_handler_stop();

int isChild = 0;
int next = 0;


int main()
{
    pid_t pid;
    int aliases = 2;    // int array alaises with 2 rows
    int current_al = 0; // current aliases

    char myHistory[20][LENGTH]; // array to store last 20 commands

    // initialize myHistory array
    for (int i = 0; i < 20; i++)
    {
        strcpy(myHistory[i], "");
    }
    /*
    save aliases in 2 continous rows
    for Example :
         createalias myhome “cd /home/users/smith”
         alias[0] == myhome and alias[1] == cd /home/users/smith
    */

    char **alias = malloc(sizeof(char *) * 2); // init with 2 rows

    /*
        act_int is for control-c handling
        act_stp is for control-z handling
    */
    static struct sigaction act_int , act_stp;

    act_int.sa_handler = sig_handler_terminate;
    act_stp.sa_handler = sig_handler_stop;
    
    sigemptyset(&(act_int.sa_mask));
    sigemptyset(&(act_stp.sa_mask));

    sigaction(SIGINT, &act_int, NULL);      // control - c
    sigaction(SIGTSTP, &act_stp, NULL);     // control - z


    while (1)
    {
        /*
          fd = file descriptor for potentially opening a file
          p = 1 if a pipe found in command
        */
        int status, fd = -1, p = 0;
        char command[LENGTH], ch; // input can be at most 1024 chars
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
        
        int history = 0;
        char history_num[3];            //store number of command to execute from myHistory
        
        if (command[0] == '!'){         //check if you have to execute command from myHistory
            int j = 1;
            int k =0;
            while (command[j] != '\0'){
                if (strlen(command) > 3) {      //if command has more than 3 chars break and do nothing (for example !111)
                    break;
                }
                history_num[k] = command[j];
                j++;
                k++;
                if (k == 2) {               
                    break;
                }
            }
            history_num[k] = '\0';
            history = atoi(history_num);        //store num of command to execute from myHistory Array
        }

        if (history > 0 && history <= 20)       //command to execute from myHistory [1,20]
        {
            strcpy(command, myHistory[history - 1]);
        }

        // store command to myHistory
        for (int i = 0; i < 20; i++)
        {
            if (strcmp(myHistory[i], "") == 0)
            {
                strcpy(myHistory[i], command);
                break;
            }

            if (i == 19)      //if Array is full
            {
                for (int i = 1; i < 20; i++)    //go each element one posistion back
                {
                    strcpy(myHistory[i - 1], myHistory[i]);
                }
                strcpy(myHistory[19], command); //store new command to last position of array
            }
        }

        // print myHistory output
        if (strcmp(command, "myHistory") == 0)
        {
            for (int i = 0; i < 20; i++)
            {
                if (strcmp(myHistory[i], "") != 0)
                {
                    printf("%d %s \n", i + 1, myHistory[i]);
                }
                else
                {
                    break;
                }
            }
            continue;
        }

        char check_al[12] = ""; // check createalias command
        strncat(check_al, command, 11);

        if (strcmp(check_al, "createalias") == 0) // handle create alias
        {
            char delim[] = " \"";
            char *word;
            char temp[LENGTH] = "";
            word = strtok(command, delim);
            word = strtok(NULL, delim);
            add_alias(&alias, &aliases, &current_al, word);

            word = strtok(NULL, delim);
            while (word != NULL)
            {
                strcat(temp, word);
                strcat(temp, " ");
                word = strtok(NULL, delim);
            }
            add_alias(&alias, &aliases, &current_al, temp);
            continue;
        }

        char check_destalias[13] = ""; // check destroy alias command
        strncat(check_destalias, command, 12);

        if (strcmp(check_destalias, "destroyalias") == 0)
        {
            char delim[] = " \"";
            char *word;
            char temp[LENGTH] = "";
            word = strtok(command, delim);
            word = strtok(NULL, delim); // find alias command you want to destroy
            for (int i = 0; i < aliases; i += 2)
            {
                if (alias[i] != NULL)
                {
                    if (strcmp(word, alias[i]) == 0)
                    {
                        alias[i] = NULL;
                        alias[i + 1] = NULL;
                    }
                }
            }
            continue;
        }

        if (strcmp(command, "exit") == 0) // exit shell commnad
        {
            return 1;
        }

        for (int i = 0; i < aliases; i += 2) // check if command is an alias
        {
            if (alias[i] != NULL)
            {
                if (strcmp(command, alias[i]) == 0)
                {                                  // alias command found
                    strcpy(command, alias[i + 1]); // change command to the aliased one
                    break;
                }
            }
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

        if (pid == 0) // is child
        {
            isChild = 1;
            *parsedCmd = commandParser(command, parsedCmd, &fd, &p); // parse command
            if (p == 1)
            {
                printf("\n");
                exit(1);
            }
            execvp(parsedCmd[0], parsedCmd);
            perror("execvp");
            exit(1);
        }
        else // parent process is mysh shell
        { 
            if (command[strlen(command) - 1] != '&') {                  // if last char of command is '&' don't wait to end
                while (waitpid (-1 , &status , WNOHANG | WUNTRACED) == 0);
            } else {
               printf("\n");
            }
            
        }

        // free parsedStr
        for (int j = 0; j < strlen(command) + 1; j++)
        {
            free(parsedCmd[j]);
        }

        free(parsedCmd);

        // close File Descriptor
        if (fd != -1)
        {
            close(fd);
        }
    }
}

void add_alias(char ***als, int *alias_malloced, int *alias_currrent, char *word1)
{
    if (*alias_currrent >= *alias_malloced)
    {
        *als = realloc(*als, sizeof(char *) * (*alias_malloced + 2));
        *alias_malloced += 2;
    }

    char *new_add = malloc(sizeof(char) * strlen(word1) + 1);
    strcpy(new_add, word1);
    (*als)[*alias_currrent] = new_add;
    (*alias_currrent)++;
}

void sig_handler_terminate()
{
    if (isChild == 1)
    {
        isChild = 0;
        exit(2);
    }

    printf("\n");
}

void sig_handler_stop() {
    if (isChild == 1)
    {   
        next = 1;
        isChild = 0;
        kill(getpid() , SIGSTOP);
    }
    printf("\n");
}
