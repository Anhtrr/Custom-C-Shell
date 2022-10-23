#ifndef _NYUSHFUNCTIONS_H_
#define _NYUSHFUNCTIONS_H_

#define TRUE 1
#define FALSE 0
#define MAXCOMMANDCHARS 1001 // including terminating 0
#define MAXWORDS 500 // incuding terminating 0

// FUNCTIONS
char* getPrompt();
void printPrompt();
void parseWords(char* input);
void validifyCmd(char** input, int numberOfWords);
void executePipeProgs(char** input, int numberOfPipes, int* pipeDetectArray, 
int numberOfWords);
void executeBuiltIns(char** input, int command, int numberOfWords);
void executeSingleProg(char** input);

#endif
