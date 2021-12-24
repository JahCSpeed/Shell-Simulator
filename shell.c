#define _GNU_SOURCE
#define TOKEN_DELIM " \t\r\n"
#define TOKEN_DELIM2 "&"
#define TOKEN_DELIM3 "%"
#define BUILT_IN_COMMANDS "bg cd exit fg jobs kill"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
int lastIndex(char** args);
typedef struct job{
    int pid;
    char** processName;
    int status; // 0 = running, 1 = stopped; -1 = terminated
    int is_background; //1 == background, 0 == forground;
} job;
int bgp_count = 0;
job* working_jobs;
job foreground_job;
pid_t shellid;
pid_t childid;
void exitShell(){
    for(int i = 1; i <= bgp_count; i++){
        if(working_jobs[i].pid == 1){
            kill(working_jobs[i].pid,SIGCONT);
        }
        kill(working_jobs[i].pid,SIGHUP);
        

    }
}
char* read_line(){
    char* input = NULL;
    size_t len = 0;
    if(getline(&input,&len,stdin) == EOF){
        return NULL;
    }
        
    return input;
}
char** copyStringArray(char** src){
    char** newArray = malloc( 2046);
    int i = 0;
    while(src[i] != NULL){
        newArray[i] = malloc(strlen(src[i]) * sizeof(char*));
        strcpy(newArray[i],src[i]);
        i++;
    }
    newArray[i] = NULL;
    return newArray;
}
void freeStringArray(char** src){
    if(src == NULL){
        return;
    }
    int i = 0;
    while(src[i] != NULL){
        free(src[i]);
        i++;
    }
    free(src);
}
int containsSubstring(char* input,char* lookingFor){
    char* ret = strstr(input,lookingFor);
    if(ret){
        return 0;
    }else{
        return -1;
    }
    
}
void fg(char** args){
    int p;
    if(lastIndex(args) >= 3){
        return;
    }else if(lastIndex(args) <= 1){
        return;
    }
    char* token = strtok(args[1],TOKEN_DELIM3);
    int targetiD = atoi(token);
    if(targetiD > bgp_count){
        return;

    }
    if(working_jobs[targetiD].status == -1){
        return;
    }else{
        pid_t pid = working_jobs[targetiD].pid;
        kill(pid, SIGCONT);
        childid = pid;
        working_jobs[targetiD].is_background = 0;
        foreground_job.processName = copyStringArray(working_jobs[targetiD].processName);
        foreground_job.pid = working_jobs[targetiD].pid;
        foreground_job.is_background = 0;
        for(int i = targetiD; i < bgp_count; i++){
            working_jobs[i] = working_jobs[i + 1];
            
        }
        bgp_count--;
        waitpid(pid, &p,WUNTRACED);
    }
    if(WIFEXITED(p) == 0){
       
                for(int j = 0; foreground_job.processName[j] != NULL; j++){
                    free(foreground_job.processName[j] );
                }
                free(foreground_job.processName);
        
    }
}
void bg(char** args){
    if(lastIndex(args) >= 3){
        return;
    }else if(lastIndex(args) <= 1){
        return;
    }
    char* token = strtok(args[1],TOKEN_DELIM3);
    int targetiD = atoi(token);
    if(targetiD > bgp_count){
        return;

    }
    if(working_jobs[targetiD].status == -1){
        return;
    }else{
        
        pid_t pid = working_jobs[targetiD].pid;
        kill(pid, SIGTTIN);
        kill(pid, SIGCONT);
        working_jobs[targetiD].is_background = 1;
        working_jobs[targetiD].status= 0;

    }
}
void cd(char** newcd){    
    if(newcd[1] == NULL){
        chdir(getenv("HOME"));
        return;
    }else{
        if(chdir(newcd[1]) == -1){
             fprintf(stdout," %s: no such directory\n", newcd[1]);
            return;
        }
    }
    return;
}

int containsAnd(char* input){

    if(input[strlen(input) - 1] == '&'){
        return 0;
    }else{
        return -1;
    }
}
int lastIndex(char** args){
    int i = 0;
    while(args[i] != NULL){
        i++;
    }
    return i;
}

void resetFg(){
    foreground_job.pid = 0;
    foreground_job.status = 2;
    foreground_job.is_background = -1;
}

void ctrl_c(int sig){
    pid_t pid = getpid();
    if(pid != shellid){
        return;
    }
    if(childid != -1){
        kill(childid,SIGINT);
        
        fprintf(stdout,"\n[%d] %d terminated by signal %d \n",1,foreground_job.pid,SIGINT);
            
        
        
    
    }
    signal(SIGINT,ctrl_c);
}
void ctrl_z(int sig){
    pid_t pid = getpid();
    if(pid != shellid){
        return;
    }
    if(childid != -1){
        // fprintf(stdout,"\n");

        kill(childid, SIGTTIN);
        kill(childid,SIGTSTP);
        
        for(int i = 1; i <= bgp_count; i++){
            if(working_jobs[bgp_count].pid == childid){
                signal(SIGTSTP,ctrl_z);
                return;
            }
        }
        
        bgp_count++;
        working_jobs[bgp_count].pid = childid;
        working_jobs[bgp_count].is_background = 0;
        working_jobs[bgp_count].status = 1;
        working_jobs[bgp_count].processName = copyStringArray(foreground_job.processName);
        fprintf(stdout,"\n");

    }
    //fflush(stdout);
    signal(SIGTSTP,ctrl_z);
}
void killz(char** args){
    if(lastIndex(args) >= 3){
        return;
    }else if(lastIndex(args) <= 1){
        return;
    }
    char* token = strtok(args[1],TOKEN_DELIM3);
    int targetiD = atoi(token);
    if(targetiD > bgp_count){
        return;

    }
    if(working_jobs[targetiD].status == -1){
        return;
    }else{
        kill(working_jobs[targetiD].pid,SIGTERM);
        working_jobs[targetiD].status = -1;
        fprintf(stdout,"[%d] %d terminated by signal %d \n",targetiD,working_jobs[targetiD].pid,SIGTERM);
    }
}

void child_sig(int sig){
    pid_t pid;
    int p;
    pid = waitpid(WAIT_ANY, &p,WNOHANG);
    for(int i = 1; i <= bgp_count; i++){
        if(working_jobs[i].pid == pid){
            if(WIFEXITED(p) == 0){
                working_jobs[i].status = -1;
                //printf("HEYY THIS SHIT STOPPED!!\n");
            }else{
                working_jobs[i].status = -1;
                //printf("HEYY THIS SHIT STOPPED!!\n");


            }
            fflush(stdout);
            break;
        }
    }
    signal(SIGCHLD,child_sig);
    
}
char** get_arguments(char* line){
    int size = 64;
    char** tokens = malloc(size * sizeof(char*));
    char* token;
    int index = 0;

    token = strtok(line,TOKEN_DELIM);
    while(token != NULL){
        tokens[index++] = token;
        if(index >= size){
            size += size;
            tokens = realloc(tokens, size * sizeof(char*));
        }
        token = strtok(NULL,TOKEN_DELIM);
    }
    
    tokens[index] = NULL;
    if(tokens[0] == NULL){
        return tokens;
    }
    if(containsAnd(tokens[lastIndex(tokens) -1]) == 0){
        char* andToken = strtok(tokens[lastIndex(tokens) -1],TOKEN_DELIM2);
        while(andToken != NULL){
            tokens[index++] = andToken;
            if(index >= size){
                size += size;
                tokens = realloc(tokens, size * sizeof(char*));
            }
            andToken = strtok(NULL,TOKEN_DELIM2);
        }
    tokens[index -1] =  "&";
    tokens[index] = NULL;
    }
    free(token);
    return tokens;
}

void background_process(char** args){
    int pid = fork();
    childid = pid;
    args[lastIndex(args) -1] = NULL;
    if(pid == 0){
        setpgid(0,0);
        if(execvp(args[0],args) == -1){
            perror(args[0]);
            return;
        }

    }else if(pid < 0){
        perror("Error: forking");
        return;
    }else{       
        bgp_count++;
        fprintf(stdout,"[%d] %d\n",bgp_count,pid);
        working_jobs[bgp_count].processName = copyStringArray(args);
        working_jobs[bgp_count].pid = pid;
        working_jobs[bgp_count].status = 0;
        working_jobs[bgp_count].is_background = 1;
    }
    
    
    return;
}
void print_jobs(){
    if(bgp_count <= 0){
        return;
    }
    for(int i = 1; i <= bgp_count; i++){
           if(working_jobs[i].status == -1){
               continue;
           }

            fprintf(stdout,"[%d] %d ",i,working_jobs[i].pid);
            if(working_jobs[i].status == 0){
                 fprintf(stdout,"Running");
            }else if(working_jobs[i].status == 1){
                 fprintf(stdout,"Stopped");
            }else if(working_jobs[i].status == -1){
                 fprintf(stdout,"Terminated ;p");
            }
            
            int p = 0;
            
            while(working_jobs[i].processName[p] != NULL){
                 fprintf(stdout," %s",working_jobs[i].processName[p]);
                p++;
            }
            
            if((working_jobs[i].status == 1 || working_jobs[i].status == -1)  && working_jobs[i].is_background == 0)
                 fprintf(stdout,"\n");
            else if(working_jobs[i].status == 0 && working_jobs[i].is_background == 1)
                 fprintf(stdout," &\n");
            else
                 fprintf(stdout,"\n");
    }
    
    
}
int forground(char** args){
    int pid = fork();
    int p;
    if(pid == 0){
        //child
        
        if(execvp(args[0],args) == -1){
            perror(args[0]);
            return 1;
        }

    }else if(pid < 0){
        return 1;
    }else{ 
        childid = pid;
        foreground_job.processName = copyStringArray(args);
        foreground_job.pid = pid;
        foreground_job.is_background = 0;   
        waitpid(-1, &p,WUNTRACED);
        
    }

    return 1;
}
void shell_loop(){
    char* command_line = NULL;
    char** command_args = NULL;
    //int isRunning;
    signal(SIGINT,ctrl_c);
    signal(SIGTSTP,ctrl_z);
    signal(SIGCHLD,SIG_IGN);
    signal(SIGCHLD,child_sig);
    do{
        childid = -1;
        fflush(stdout);
        fprintf(stdout,"> ");
        command_line = read_line();
        if(feof(stdin)){
            exitShell();
            for(int i = 1; i <= bgp_count; i++){
                for(int j = 0; working_jobs[i].processName[j] != NULL; j++){
                    free(working_jobs[i].processName[j] );
                }
                free(working_jobs[i].processName);
            }
            free(working_jobs);
            return;
        }
        command_args = get_arguments(command_line);
        
        if(command_args[0] == NULL){
           continue;
        }
        if(strcmp(command_args[0], "bg") == 0){
            bg(command_args);
        }else if(strcmp(command_args[0], "cd") == 0){
            cd(command_args);
        }else if(strcmp(command_args[0], "fg") == 0){
            fg(command_args);
        }else if(strcmp(command_args[0], "jobs") == 0){
            print_jobs();
        }else if(strcmp(command_args[0], "kill") == 0){
            killz(command_args);
        }else if(strcmp(command_args[0], "exit") == 0 ){
            exitShell();
            free(command_line);
            free(command_args);
            return;
        }else{
            if(containsAnd(command_args[lastIndex(command_args) - 1]) == 0){
                background_process(command_args);
            }else{
                forground(command_args);  
            }
        }
    free(command_line);
    free(command_args);
    }while(1);
    

}
int main(int argc, char** argv){
    bgp_count = 0;
    shellid = getpid();
    working_jobs = malloc(1000 *sizeof(job));
    shell_loop();
    return EXIT_SUCCESS;
}
