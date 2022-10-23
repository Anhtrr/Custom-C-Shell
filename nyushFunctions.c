#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <libgen.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

/*
*   1) PUBLIC AND STATIC DEFINITIONS:
*
*       - TRUE: 1
*       - FALSE: 0
*       - MAXCOMMANDCHARS: 1001 (including mem space for terminating 0)
*       - MAXWORDS: 501 (incuding mem space for terminating 0)
*       - MAXSUSPENDEDJOBS: 101 (including mem space for terminating 0)
*       - struct Job: custom Job object used for handling jobs and fg command
*
*/
#define TRUE 1
#define FALSE 0
#define MAXCOMMANDCHARS 1001 
#define MAXWORDS 501
#define MAXSUSPENDEDJOBS 101
typedef struct Job{
    pid_t pid;
    char** command;
    int numberOfWords;
} Job;

/* 
*   2) PROMPT:
*
*       - getPrompt(): This function is responsible for fetching the current active
*                      directory. Then it constructs the prompt by building a String
*       - printPrompt(): This function actually prints out the constructed prompt from 
*                        getPrompt().
*
*/
char* getPrompt(){   
    // first part of prompt
    char* programName = "[nyush ";
    // second part of prompt
    // get current directory path - allocate 256 bytes
    char currentDir[2048];
    getcwd(currentDir, sizeof(currentDir));
    // get last directory in path
    char *lastDir;
    // if in root directory
    lastDir = basename(currentDir);
    // last part of prompt
    char* closingPrompt = "]$ ";
    // construct prompt
    char *prompt = (char*) malloc(strlen(programName) + strlen(lastDir) 
    + strlen(closingPrompt) + 1);
    strcpy(prompt, programName);
    strcat(prompt, lastDir);
    strcat(prompt, closingPrompt);
    // return prompt  
    return prompt;
}
void printPrompt(){
    // initiate pointer and function
    char* prompt;
    char* getPrompt();
    // call getPrompt
    prompt = getPrompt();
    // print the prompt
    printf("%s", prompt);
    // flush
    fflush(stdout);
}

/*
*   3) COMMAND READER AND INTERPRETER: 
*
*       - parseWords(): This function is responsible for firstly, taking in a full command 
*                       line input from the user as a String and reads it word by word, split
*                       by either a single space " " or new line (\n). It then inputs this 
*                       in a String array that it is also responsible for constructing: 
*                       wordsArray. It then sends this newly constructed array to 
*                       validifyCmd() to further process the user input and throw any errors.
*       - validifyCmd(): This function is responsible for understanding the grammar behind 
*                        the user's input line. It first checks if it's a Built In Command,
*                        then checks if the program contains any pipes or redirections. Some
*                        gramatical errors are also handled here and will return to the main
*                        shell if such error occurs in user input. If not, if the input calls
*                        a built in function, then it gets passed to executeBuiltIns(). If the
*                        input is not a built in and contains pipes, then it will be passed to
*                        executePipeProgs(). If the input does not contain pipes but contains
*                        redirection symbols, it will be passed to executeProgWRedir(). If none
*                        of the above specs are met, then it will be passed to executeSingleProg().
*
*/
void parseWords(char* input){
    // make a copy of inputLine
    char *inputLineCopy = strdup(input);
    // initialize delimiter - any single space occurence or new line 
    const char *delimiter = " \n";
    // keep track of number of Words in input line (or length of array)
    int numberOfWords;
    // initialize array to store Words from inputLine  
    char **wordsArray = (char**) malloc((sizeof(char*)) * MAXWORDS);
    // initialize token
    char *token;
    // initialize tmp pointer to inputLine
    char *tmp = inputLineCopy;
    // construct wordsArray that stores words split by space or new line 
    for(int i = 0; ;i++){
        // word pointed to by token pointer 
        token = strtok(tmp, delimiter);
        // base case - done constructing if no (more) space(s)/newline(s) is(are) found
        if (token == NULL){                   
            // set total numberOfWords in inputLine
            numberOfWords = i;
            break;
        }
        // constructing array (including temrinating NULL)
        wordsArray[i] = (char*) malloc (strlen(token + 1));
        strcpy(wordsArray[i], token);
        // for next strtok() iteration
        tmp = NULL;
    }
    // if user enters a blank line then reprompt 
    if (numberOfWords == 0){
        return;
    }
    else{
    // call validifyCmd() to validify command line grammar
        void validifyCmd();
        validifyCmd(wordsArray, numberOfWords);
    }
    // return
    return;
}
void validifyCmd(char** input, int numberOfWords){
    // PATH HANDLING
    // loop through each word 
    for(int i = 0; i < numberOfWords; i++){
        int length = strlen(input[i]);
        // loop through each character of each word
        for(int j = 0; j < length; j++){
            // if '/' is detected in the word
            if(input[i][j] == '/'){
                // ABSOLUTE PATH - if path word starts with "/"
                if(input[i][0] == '/'){
                    break;
                }
                // if path declares current working dir - "./".../...
                else if((input[i][0] == '.') && 
                (input[i][1] == '/')){
                    break;
                }
                // RELATIVE PATH - path starts with dir "home/..."
                else{
                    // create tmpStr to construct new word
                    int allocLength = strlen(input[i]+3);
                    char* tmpStr = (char*) malloc(allocLength);
                    // add "./" to the front of the word
                    strcpy(tmpStr, "./");
                    strcat(tmpStr, input[i]);
                    // overwrite word
                    input[i] = (char*) malloc(allocLength);
                    // copy tmp to word
                    strcpy(input[i], tmpStr);
                    break;
                }
            }
            // BASE NAME if none of the conditions above are ever met
            // do nothing to it
            else{
                continue;
            }
        }
    }
    // create array to store built in commands
    const char* builtInCommands[4];
    // cd jobs fg exit
    builtInCommands[0] = "cd";
    builtInCommands[1] = "jobs";
    builtInCommands[2] = "fg";
    builtInCommands[3] = "exit";
    // check if first word is a built in command 
    for (int i = 0; i < 4; i++){
        int cmp = strcmp(input[0], builtInCommands[i]);
        // BUILT IN COMMAND
        if (cmp == 0){
            // execute the built in command
            void executeBuiltIns();
            executeBuiltIns(input, i, numberOfWords);
            return;
        }
    }
    
    // int array
    int pipeDetectArray[numberOfWords+1];
    // total number of pipes
    int pipeCount = 0;
    // total number of input redir
    int inputRedirCount = 0;
    // total number of output redir
    int outputRedirCount = 0;
    // record the redirection index in array
    int redirIndex = 0;

    // check if there are pipes or redirs
    for (int i = 0; i < numberOfWords; i++){
        // pipe
        int cmp = strcmp(input[i], "|");
        // input redir
        int cmp1 = strcmp(input[i], "<");
        // output redir
        int cmp2 = strcmp(input[i], ">");
        int cmp3 = strcmp(input[i], ">>");
        int cmp4 = strcmp(input[i], "<<");
        // ERROR 1 - not a valid redirection
        if (cmp4 == 0){
            fprintf(stderr , "Error: invalid command\n");
            return;
        }
        // ERROR 2 - In each command, there may be at most one input 
        // redirection and one output redirection.
        else if (inputRedirCount > 1 || outputRedirCount > 1){
            fprintf(stderr , "Error: invalid command\n");
            return;
        }
        // if input redir is detected 
        else if (cmp1 == 0){
            if(!input[i+1]){
                fprintf(stderr , "Error: invalid command\n");
                return;
            }
            else{
                inputRedirCount++;
                if(redirIndex == 0){
                    redirIndex = i;
                }
                pipeDetectArray[i] = 0;
            }
        }
        // if output redir is detected 
        else if ((cmp2 == 0) || (cmp3 == 0)){
            if(!input[i+1]){
                fprintf(stderr , "Error: invalid command\n");
                return;
            }
            else{
                outputRedirCount++;
                if(redirIndex == 0){
                    redirIndex = i;
                }
                pipeDetectArray[i] = 0;
            }
        }
        // if pipe is detected at index i
        else if (cmp == 0){
            pipeCount++;
            pipeDetectArray[i] = 1;
        }
        else{
            pipeDetectArray[i] = 0;
        }
    }
    // ERROR - if redirection contains more than 1 file
    // example - cat > file.txt file2.txt
    if(redirIndex != 0){
        int redirFilee = redirIndex+2;
        char* wordd = input[redirFilee];
        if (wordd != NULL){
            int cmp = strcmp(wordd, ">");
            int cmp1 = strcmp(wordd, ">>");
            int cmp2 = strcmp(wordd, "<");
            int cmp3 = strcmp(wordd, "|");
            if(cmp != 0 && cmp1 != 0 && cmp2 != 0 && cmp3 != 0){
                fprintf(stderr, "Error: invalid command\n");
                return;
            }
        }
    }
    // if there are pipes 
    if (pipeCount > 0){
        if (inputRedirCount > 0 || outputRedirCount > 0){
            void executePipeProgs();
            executePipeProgs(input, pipeCount, pipeDetectArray, numberOfWords);
        }
        else{
            void executePipeProgs();
            executePipeProgs(input, pipeCount, pipeDetectArray, numberOfWords);
        }
        return;
    }
    // if there are redirects 
    else if (inputRedirCount > 0 || outputRedirCount > 0){
        void executeProgWRedir();
        executeProgWRedir(input, numberOfWords);
        return;
    }
    // if just a regular command w no redirections
    else{
        void executeSingleProg();
        executeSingleProg(input);
        // return
        return;
    }
}

/*
*   4) EXECUTE COMMANDS: 
*
*       - suspendedJobs[]: This array is used to store all current suspended jobs
*                          in the nyushell. 
*       - executePipeProgs(): This function is used to execute any user command lines
*                             that involve redirections with pipes.
*       - executeBuiltIns(): This function is used to execute any of the four allowed
*                            built in functions including: cd, exit, jobs, and fg.
*       - executeProgWRedir(): This function is used to execute any user command lines
*                              that involves redirection symbols but no pipes. 
*       - executeSingleProg(): This function is used to execute any function that do
*                              not match the functions above: executePipeProgs, 
*                              executeBuiltIns, executeProgWRedir. 
*
*/
Job* suspendedJobs[MAXSUSPENDEDJOBS];
void executePipeProgs(char** input, int numberOfPipes, int* pipeDetectArray, 
int numberOfWords){    
    // ERROR 1 - pipe starts first or ends the command
    if ((pipeDetectArray[0] == 1) || 
    (pipeDetectArray[numberOfWords-1] == 1)){
        fprintf(stderr , "Error: invalid command\n");
        return;
    }
    // construct array of array of input words separated by pipes
    // for null and prog after pipe
    char ***progArray = (char***) malloc(sizeof(char**) * (numberOfPipes+2)); 
    // allocate space for **progArray
    for (int i = 0; i < numberOfPipes+1; i++){
        progArray[i] = (char**) malloc(sizeof(char*)*MAXWORDS);
    }
    // index of char** array - progsArray[]
    int numberofProgs = 0;
    // index of char* word - progsArray[][]
    int k = 0;
    // length of each prog
    int lengthOfProg[numberOfPipes+1]; 
    // loop through input line words
    for (int i = 0; i < numberOfWords; i++){
        // if pipe is detected
        if(pipeDetectArray[i] == 1){ 
            // if pipe is detected 
            progArray[numberofProgs][k] = NULL;
            // TEST
            //printf("A[%i][%i]: %s\n", numberofProgs, k, progArray[numberofProgs][k]);
            //fflush(stdout);
            lengthOfProg[numberofProgs] = k; 
            numberofProgs++;
            k = 0;
        }
        // if pipe is not detected 
        else if (pipeDetectArray[i] == 0){
            progArray[numberofProgs][k] = (char*) malloc(strlen(input[i])+1);
            strcpy(progArray[numberofProgs][k],input[i]);
            // TEST
            //printf("A[%i][%i]: %s\n", numberofProgs, k, progArray[numberofProgs][k]);
            //fflush(stdout);
            k++;
        }
    }
    // last index set to null
    progArray[numberofProgs][k] = NULL;
    lengthOfProg[numberofProgs] = k; 
    numberofProgs++; 
    progArray[numberofProgs] = NULL;
    
    // check if first prog or second prog contains redirections
    int redirDetectArray[2] = {0,0};
    int redirIndex[2] = {0,0};
    // typeOf: 1 - ">" ; 2 - ">>"
    int typeOf = 0;

    // ERROR HANDLING
    // loop through progs
    for(int i = 0; i < numberofProgs; i++){
        // loop through words
        for(int j = 0; j < lengthOfProg[i]; j++){
            int cmp = strcmp(progArray[i][j], "<");
            int cmp1 = strcmp(progArray[i][j], ">");
            int cmp2 = strcmp(progArray[i][j], ">>");
            // first prog
            if(i == 0){
                // if input redir is present
                if(cmp == 0){
                    redirDetectArray[i] = 1;
                    redirIndex[0] = j;
                }
                // ERROR 1 - if output redir is present in first prog
                else if(cmp1 == 0 || cmp2 == 0){
                    fprintf(stderr , "Error: invalid command\n");
                    return;
                }
            }
            // last prog
            else if(i == (numberofProgs-1)){
                // if output redir is present
                if(cmp1 == 0){
                    typeOf = 1;
                    redirDetectArray[1] = 1;
                    redirIndex[1] = j;
                } 
                else if(cmp2 == 0){
                    typeOf = 2;
                    redirDetectArray[1] = 1;
                    redirIndex[1] = j;
                }
                // ERROR 2 - if input redir is present in last prog
                else if(cmp == 0){
                    fprintf(stderr , "Error: invalid command\n");
                    return;
                }
            }
            // middle progs
            else{
                if(cmp == 0 || cmp1 == 0 || cmp2 == 0){
                    fprintf(stderr , "Error: invalid command\n");
                    return;
                }
                else{
                    continue;
                }
            }
        }
    }
    // PARENT initial variables
    // need 2 file desc spaces per pipe
    int pip[2];
    // store fd of STDOUT of previous prog
    int prevProgOut = 0;
    // storing pids for every process forked
    pid_t pid[numberofProgs];

    // redirecting each prog input/output 
    for(int i = 0; i < numberofProgs; i++){
        // pipe fd array
        int pipeStatus = pipe(pip);
        // pipe error
        if (pipeStatus == -1){
            fprintf(stderr , "Error: invalid command\n");
            return;
        }
        // fork for every process (prog)
        pid[i] = fork();
        // fork error
        if(pid[i] == -1){
            fprintf(stderr , "Error: invalid command\n");
            return;
        }
        // CHILD process of prog 
        else if (pid[i] == 0){
            // default signals
            void sigDefault();
            sigDefault();
            // if first program
            if(i == 0){
                // if there is input redirection
                if(redirDetectArray[0] == 1){
                    // redirect input to open
                    int inputFD = open(progArray[i][redirIndex[0]+1], 
                    O_RDONLY);
                    if (inputFD == -1){
                        fprintf(stderr, "Error: invalid file\n");
                        return;
                    }
                    dup2(inputFD, STDIN_FILENO);
                }
                else{
                    // duplicate the STDIN to prevProgOut
                    dup2(prevProgOut, STDIN_FILENO);
                }
            }
            else{
                // duplicate the STDIN to prevProgOut
                dup2(prevProgOut, STDIN_FILENO);
            }
                        
            // if not last prog
            if(i != numberofProgs-1){
                // duplicate STDOUT of current CHILD prog to 
                // write end of pipe - [1]
                dup2(pip[1], STDOUT_FILENO);
            }
            // if last prog
            else if(i == numberofProgs-1){
                // if there is output redirection
                if(redirDetectArray[1] == 1){
                    // redirect output to file
                    // >
                    if(typeOf == 1){
                        int outputFD = creat(progArray[i][redirIndex[1]+1], 
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                        if (outputFD == -1){
                            fprintf(stderr, "Error: invalid file\n");
                            return;
                        }
                        dup2(outputFD, STDOUT_FILENO);
                    }
                    // redirect output to file
                    // >>
                    else if (typeOf == 2){
                        int outputFD = open(progArray[i][redirIndex[1]+1], 
                        O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR 
                        | S_IRGRP | S_IROTH);
                        if (outputFD == -1){
                            fprintf(stderr, "Error: invalid file\n");
                            return;
                        }
                        dup2(outputFD, STDOUT_FILENO);
                    }
                }
            } 
            // close the read end [0] in pipe of current CHILD prog 
            close(pip[0]);
            // if first prog has redirection
            if(i == 0 && redirDetectArray[0] == 1){
                char* execVpArray[redirIndex[0]+1];
                for(int j = 0; j < redirIndex[0]; j++){
                    execVpArray[j] = progArray[i][j];
                }
                execVpArray[redirIndex[0]] = NULL;
                // execute the current CHILD prog
                int exec = execvp(execVpArray[0], execVpArray);
                if(exec == -1){
                    fprintf(stderr , "Error: invalid program\n");
                    exit(2);
                    return;
                }
            }
            // if last prog has redirection
            else if(i == numberofProgs-1 && redirDetectArray[1] == 1){
                char* execVpArray[redirIndex[1]+1];
                for(int j = 0; j < redirIndex[1]; j++){
                    execVpArray[j] = progArray[i][j];
                }
                execVpArray[redirIndex[1]] = NULL;
                // execute the current CHILD prog
                int exec = execvp(execVpArray[0], execVpArray);
                if(exec == -1){
                    fprintf(stderr , "Error: invalid program\n");
                    exit(2);
                    return;
                }
            }
            else{
                // execute the current CHILD prog
                int exec = execvp(progArray[i][0], progArray[i]);
                if(exec == -1){
                    fprintf(stderr , "Error: invalid program\n");
                    exit(2);
                    return;
                } 
            } 
        }
        else{
            void sigIgnore();
            sigIgnore();
            // PARENT process
            // wait for CHILD process
            waitpid(pid[i], NULL, 0);
            // currently, STDOUT of CHILD process will be in read end
            // [0] of pipe in parent process
            // close write end [1] of pipe in parent process
            close(pip[1]);
            // set prevProgOut FD to read end [0] of pipe in parent process
            prevProgOut = pip[0];
        }
    }    
    return;
}
void executeBuiltIns(char** input, int command, int numberOfWords){
    void sigIgnore();
    sigIgnore();
    /* 
    *  command 0 - cd
    *  command 1 - jobs
    *  command 2 - fg
    *  command 3 - exit
    */ 
    // ERROR 1 - Built-in commands (e.g., cd) cannot be I/O redirected or piped.
    for (int j = 0; j < numberOfWords; j++){
        int cmp1 = strcmp(input[j], "|");
        int cmp2 = strcmp(input[j], ">");
        int cmp3 = strcmp(input[j], "<");
        int cmp4 = strcmp(input[j], ">>");
        if (cmp1 == 0 || cmp2 == 0 || cmp3 == 0 || cmp4 == 0){
            fprintf(stderr , "Error: invalid command\n");
            return;
        }
    }  
    // CD
    if (command == 0){
        // ERROR 1 - if cd is called with 2 arguments or no arguments
        if ((numberOfWords > 2) || (numberOfWords == 1)){
            fprintf(stderr , "Error: invalid command\n");
            return;
        }
        // call cd - SYSTEM CALL
        int validCd = chdir(input[1]);

        // ERROR 2 - if directory not found 
        if (validCd != 0){
            fprintf(stderr , "Error: invalid directory\n");
            return;
        }
    } 
    // JOBS
    else if (command == 1){
        int getIndex();
        int a = getIndex();
        // loop thru all the suspended jobs
        for(int i = 0; i < a; i++){
            int words = suspendedJobs[i]->numberOfWords;
            char* jobLine = (char*) malloc(MAXCOMMANDCHARS);
            // construct job line
            for (int j = 0; j < words; j++){
                char* word = suspendedJobs[i]->command[j];
                char* space = " ";
                // if first word
                if(j == 0){
                    strcpy(jobLine, word);
                    strcat(jobLine, space);
                }
                else{
                    strcat(jobLine, word);
                    strcat(jobLine, space);
                }
            }
            // print job line
            printf("[%i] %s\n", i+1, jobLine);
            fflush(stdout);
        }
        return;
    } 
    // FG
    else if (command == 2){
        // ERROR 1 - if fg is not called with exactly 1 argument
        if(numberOfWords != 2){
            fprintf(stderr , "Error: invalid command\n");
            return;
        }
        // convert string to integer
        int arg = atoi(input[1]);
        // get size of suspended jobs
        int getIndex();
        int a = getIndex();
        // not an existing suspended job
        if(arg < 1 || arg > a){
            fprintf(stderr , "Error: invalid job\n");
            return;
        }
        else{
            // adjust to actual index
            arg -= 1;
            // fetch pid of suspendedJob
            pid_t pid = suspendedJobs[arg]->pid;
            // create new temporary job
            Job* tmp = (Job*) malloc(sizeof(Job));
            // store current job
            tmp->pid = pid;
            tmp->command = suspendedJobs[arg]->command;
            tmp->numberOfWords = suspendedJobs[arg]->numberOfWords;

            // remove current job chosen to continue
            int argg = arg; 
            
            // keep elements
            while (suspendedJobs[argg+1]){
                suspendedJobs[argg] = suspendedJobs[argg+1];
                argg++;
            }
            // overwrite job that user chooses
            suspendedJobs[argg]= (Job*) malloc(sizeof(Job));
            suspendedJobs[argg] = NULL;

            // updates as the latest 
            kill(pid, SIGCONT);
            // status of continued process
            int status;
            // NEW PARENT
            waitpid(pid, &status, WUNTRACED);
            int a;
            // construct and add to suspended jobs list again
            if(WIFSTOPPED(status) == TRUE){
                // Add to most recent suspended list
                int getIndex();
                a = getIndex();
                suspendedJobs[a] = tmp;
            }
            return;
        }
    } 

    // execute exit 
    else if (command == 3){
        // ERROR 1 - if exit command contains any arguments
        if (numberOfWords > 1){
            fprintf(stderr , "Error: invalid command\n");
            return;
        }
        // check for suspended jobs
        int getIndex();
        int a = getIndex();
        if(a != 0){
            fprintf(stderr , "Error: there are suspended jobs\n");
            return;
        }
        else{
            exit(0);
        }
    }
    return;
}
void executeProgWRedir(char** input, int numberOfWords){
    // keep track of child status
    int status;
    // mark the index that the redirectionSymbol is at
    int redirIndex = 0;
    pid_t pid = fork();
    if (pid == 0){
        // call default sig functions back
        void sigDefault();
        sigDefault();
        // CHILD PROCESS
        char* word;
        for(int i = 0; i < numberOfWords; i++){
            word = input[i];
            if (strcmp(word, ">") == 0){
                // if input redirection has already been seen
                if(redirIndex == 0){
                    redirIndex = i;
                }
                // create file descriptor for STDOUT
                int outputFD = creat(input[i+1], S_IRUSR | S_IWUSR 
                | S_IRGRP | S_IROTH);
                // ERROR - if any error with opening file descriptor 
                if(outputFD == -1){
                    fprintf(stderr, "Error: invalid file\n");
                    exit(2);
                    return;
                }
                // redirect STDOUT
                dup2(outputFD, STDOUT_FILENO);
                close(outputFD);
            }
            else if (strcmp(word, ">>") == 0){
                // if input redirection has already been seen
                if(redirIndex == 0){
                    redirIndex = i;
                }
                // create file descriptor for STDOUT
                int outputtFD = open(input[i+1], O_RDWR | O_CREAT | O_APPEND, 
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if(outputtFD == -1){
                    fprintf(stderr, "Error: invalid file\n");
                    exit(2);
                    return;
                }
                // redirect STDOUT
                dup2(outputtFD, STDOUT_FILENO);
                close(outputtFD);
            }
            else if (strcmp(word, "<") == 0){
                // if output redirection has already been seen
                if(redirIndex == 0){
                    redirIndex = i;
                }
                // create file descriptor for STDIN
                int inputFD = open(input[i+1], O_RDONLY);
                if(inputFD == -1){
                    fprintf(stderr, "Error: invalid file\n");
                    exit(2);
                    return;
                }                
                // redirect STIDIN
                dup2(inputFD, STDIN_FILENO);
                close(inputFD);
            }
        }
        // used to copy words from input for execvp call
        char* execVpArray[numberOfWords];
        // construct execvparray
        for(int i = 0; i < redirIndex; i++){
            execVpArray[i] = input[i];
        }
        execVpArray[redirIndex] = NULL;
        int execInt = execvp(execVpArray[0], execVpArray);
        if (execInt == -1){
            fprintf(stderr, "Error: invalid program\n");
            exit(2);
            return;
        }
    }
    else if(pid == -1){
        fprintf(stderr, "Error: invalid command\n");
        return;
    }
    // PARENT
    else{
        // ignore signals
        void sigIgnore();
        sigIgnore();
        waitpid(pid, &status, WUNTRACED);
        // if child was stopped 
        if(WIFSTOPPED(status) == TRUE){
            // construct job 
            Job* sus = (Job*) malloc(sizeof(Job));
            sus->pid = pid;
            sus->command = input;
            sus->numberOfWords = numberOfWords;
            // Add to suspended list
            int getIndex();
            int a = getIndex();
            suspendedJobs[a] = sus;
            printf("\n");
            fflush(stdout);
        }
        // if child was terminated with a signal 
        if(WIFSIGNALED(status) == TRUE){
            printf("\n");
            fflush(stdout);
        }
    }
    return; 
}
void executeSingleProg(char** input){
    // fork a child
    pid_t pid = fork();
    // if forking is successful 
    if (pid == 0){
        void sigDefault();
        sigDefault();
        // execute file command and store return value in execInt
        int execInt = execvp(input[0], input);
        // if exec command fails
        if (execInt == -1){
            // try path command and store return value in execInt1
            int execInt1 = execvp(input[0], input);
            if (execInt1 == -1){
                fprintf(stderr , "Error: invalid program\n");
                exit(2);
                return;
            }
        }
    }
    else if (pid == -1){
        fprintf(stderr , "Error: invalid command\n");
        return;
    }
    // fork was successful - parent process executes here
    else{
        // keep track of child status
        int status;
        // Ignore signal
        void sigIgnore();
        sigIgnore();
        // wait for child
        waitpid(pid, &status, WUNTRACED);
        // if child was stopped 
        if(WIFSTOPPED(status) == TRUE){
            // construct job 
            Job* sus = (Job*) malloc(sizeof(Job));
            sus->pid = pid;
            sus->command = input;
            sus->numberOfWords = 1;
            // Add to suspended list
            int getIndex();
            int a = getIndex();
            suspendedJobs[a] = sus;
        }
    }
    return;
}

/*
*   5) HANDLING SIGNALS:
*
*       - sigDefault(): This function sets all the signals that were ignored
*                       back to default functions.
*       - sigIgnore(): This function sets all the signals that were ignored 
*                      to ignore again.
*
*/
void sigDefault(){
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGSTOP, SIG_DFL);
}
void sigIgnore(){
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGSTOP, SIG_IGN);
}

/*
*   6) HELPERS:
*
*       - getIndex(): This function gets the index of the next available 
*                     slot in the public suspendedJobs list. This index 
*                     can also be used as the size/ number of suspended
*                     jobs. 
*
*/
int getIndex(){
    int a = 0;
    for(int i = 0; i < MAXSUSPENDEDJOBS-1;i++){
        // if 100th spot
        if(i == MAXSUSPENDEDJOBS-2){
            // if last index is NULL
            if(!suspendedJobs[i]){
                return i;
            }
            // if last index is already taken 
            else{
                return -1;
            }
        }
        // if current index has not been taken
        else if(!suspendedJobs[i]){
            a = i;
            break;
        }
        // if this spot is already taken
        else{
            continue;
        }
    }
    return a;
}
