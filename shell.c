#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>


#define maxTokens 5
#define maxHistory 100
void exitShell(char* exitPointer);
void changeDirectory(char* path);
void addToHistory(char* tokens[], int* historyCount, char* historyArray[][maxTokens]);
void printHistory(char* historyArray[][maxTokens], int historyCount);
struct Tuple findRedirection(char** tokens, int tokensSize);
void removeElement(char** array, int index, int* size);
void isBackground(char** tokens, int* tokensSize, char* isBackgroundFlag);
void reapChildren();

struct Tuple {
    char a;
    char* b;
};

int main() {
    char* history[maxHistory][maxTokens] = { NULL };
    int historyCount = 0; 
    char exit = 0;
    while(exit == 0) {
        char* cwd;
        char buff[1024];

        int tokensSizeMax = 10;
        char input[100]; 
        char* token;     
        char* tokens[tokensSizeMax]; 
        int tokensSize = 0;
        char isBackgroundFlag = 0;

        cwd = getcwd(buff, 1024);
        
        if( cwd != NULL ) {
            printf( "%s> ", cwd );
        }

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';

        token = strtok(input, " ");
        while (token != NULL) {
            tokens[tokensSize] = token;       
            tokensSize++;                   
            token = strtok(NULL, " "); 
        }
        tokens[tokensSize] = NULL; 
        
        isBackground(tokens, &tokensSize, &isBackgroundFlag);
        addToHistory(tokens, &historyCount, history);
        // signal(SIGCHLD, SIG_IGN); //cheating maybe

        signal(SIGCHLD, reapChildren);

        if (strcmp(tokens[0], "exit") == 0) {
            exitShell((&exit));
        }
        else if (strcmp(tokens[0], "cd") == 0) {
            changeDirectory(tokens[1]);
        }
        else if (strcmp(tokens[0], "history") == 0) {
            printHistory(history, historyCount);
        }
        else {
            pid_t pid;

            pid = fork();

            if (pid < 0) {
                perror("Fork failed");
                
            } else if (pid == 0) {
                // printf("child process\n");

                struct Tuple isRedirect = findRedirection(tokens, tokensSize);
      
                if (isRedirect.a == '>') {
                    freopen(isRedirect.b, "w", stdout);
                } else if (isRedirect.a == '<') {
                    freopen(isRedirect.b, "r", stdin);
                }
                // printf("Running command: %s\n", tokens[0]);
                execvp(tokens[0], tokens);
                char success = execvp(tokens[0], tokens);
                if (success == -1) {
                    printf("Error running command, command may not exist\n");
                }
                
            } else {
                // printf("parent process Child PID: %d\n", pid);
                
                if (isBackgroundFlag == 0) {
                    int status;
                    waitpid(pid, &status, 0);
                }
            
            
            }

       
    }
    }
    return 0;
}

void exitShell(char* exitPointer) {
    *exitPointer = 1;
}

void changeDirectory(char* path) {
    char success = chdir(path);
    // printf("Success: %d\n", success);
    if (success != 0) {
        printf("Error changing directory\n");
    }
}

void addToHistory(char* tokens[], int* historyCount, char* historyArray[][maxTokens]) {
    for (int i = 0; tokens[i] != NULL && i < maxTokens; i++) {
        historyArray[*historyCount][i] = strdup(tokens[i]);
    }
    (*historyCount)++;
}

void printHistory(char* historyArray[][maxTokens], int historyCount) {
    
    for (int i = 0; i < historyCount; i++) {
        
        printf("%d: ", i);
        for (int j = 0; historyArray[i][j] != NULL && j < maxTokens; j++) {
            printf("%s ", historyArray[i][j]);
        }
        printf("\n");
    }
}

struct Tuple findRedirection(char** tokens, int tokensSize){
    struct Tuple tuple;
    for (int i = 0; i < tokensSize; i++){
        if (strcmp(tokens[i], ">") == 0){
            tuple.a = '>';
            tuple.b = tokens[i + 1];
            removeElement(tokens, i, &tokensSize);
            removeElement(tokens, i, &tokensSize);
            return tuple;
        } else if (strcmp(tokens[i], "<") == 0){
            tuple.a = '<';
            tuple.b = tokens[i + 1];
            removeElement(tokens, i, &tokensSize);
            removeElement(tokens, i, &tokensSize);
            return tuple;
        }
        
    }
    
    return tuple;
}

void removeElement(char** array, int index, int* size) {
    if (index < 0 || index >= *size) {
        printf("Index out of bounds\n");
        return;
    }
    for (int i = index; i < *size - 1; i++) {
        array[i] = array[i + 1];
    }
    array[*size - 1] = NULL; 
    *size = *size - 1;
}

void isBackground(char** tokens, int* tokensSize, char* isBackgroundFlag) {
    if (tokens[*tokensSize - 1] != NULL && strcmp(tokens[*tokensSize - 1], "&") == 0) {
        *isBackgroundFlag = 1;
        removeElement(tokens, *tokensSize - 1, tokensSize);
    }
}

void reapChildren() {
    int status;
    waitpid(-1, &status, WNOHANG);
}


