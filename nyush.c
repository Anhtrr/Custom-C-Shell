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
#include "nyushFunctions.h"

/* 
*   1) MAIN:
*       - used to run the nyush shell
*/ 
int main(){  
    // RUNNING THE SHELL 
    while (TRUE){
        // ignore signals
        void sigIgnore();
        sigIgnore();
        // print prompt
        void printPrompt();
        printPrompt();
        // users input stored in
        char inputLine[MAXCOMMANDCHARS]; 
        char* result = fgets(inputLine, MAXCOMMANDCHARS, stdin); 
        // if fgets is an error
        if (result == NULL){
            break;
        }
        // evaluate inputLine
        void parseWords();
        parseWords(inputLine);
    }
}

/* 
*   WORKS CITED (EXTERNAL SOURCES THAT INFLUENCED MY PROJECT):
*
*       https://www.tutorialspoint.com/unix/unix-io-redirections.htm
*       https://jameshfisher.com/2017/02/24/what-is-mode_t/
*       https://www.shells.com/l/en-US/tutorial/Difference-between-%E2%80%9C%3E%E2%80%9D-and-%E2%80%9C%3E%3E%E2%80%9D-in-Linux#:~:text=So%2C%20what%20we%20learned%20is,to%20modify%20files%20in%20Linux.
*       https://stackoverflow.com/questions/15102992/what-is-the-difference-between-stdin-and-stdin-fileno
*       https://man7.org/linux/man-pages/man2/dup.2.html
*       https://www.geeksforgeeks.org/c-program-demonstrate-fork-and-pipe/
*       https://linuxhint.com/using_pipe_function_c_language/ 
*       https://man7.org/linux/man-pages/man2/pipe.2.html
*       https://www.youtube.com/watch?v=QD9YKSg3wCc
*       https://www.gnu.org/software/libc/manual/html_node/Permission-Bits.html
*       https://stackoverflow.com/questions/18415904/what-does-mode-t-0644-mean
*       https://man7.org/linux/man-pages/man3/creat.3p.html
*       https://man7.org/linux/man-pages/man2/dup.2.html
*       https://man7.org/linux/man-pages/man2/pipe.2.html
*       https://linuxhint.com/linux-pipe-command-examples/
*       https://www.cyberciti.biz/faq/kill-process-in-linux-or-terminate-a-process-in-unix-or-linux-systems/
*       https://www.ibm.com/docs/en/aix/7.2?topic=shell-signal-handling-in-c
*       https://www.gnu.org/software/libc/manual/html_node/Job-Control-Signals.html
*       https://u.osu.edu/cstutorials/2018/09/28/how-to-debug-c-program-using-gdb-in-6-simple-steps/
*       https://ftp.gnu.org/old-gnu/Manuals/gdb/html_node/gdb_25.html#:~:text=By%20default%2C%20when%20a%20program,set%20follow%2Dfork%2Dmode%20.&text=Set%20the%20debugger%20response%20to%20a%20program%20call%20of%20fork%20or%20vfork%20.
*       https://stackoverflow.com/questions/4300317/meaning-of-detaching-after-fork-from-child-process-15
*       https://linuxhint.com/signal_handlers_c_programming_language/
*       https://www.csl.mtu.edu/cs4411.ck/www/NOTES/signal/handler.html#:~:text=If%20you%20want%20to%20ignore,SIG_IGN%20for%20the%20second%20argument.&text=In%20the%20above%20two%20lines,SIG_DFL%20for%20the%20second%20argument.
*       https://linuxhint.com/resolve-bash-resource-temporarily-unavailable/
*       https://man7.org/linux/man-pages/man7/signal.7.html
*       https://linux.die.net/man/3/exit
*       https://stackoverflow.com/questions/7021725/how-to-convert-a-string-to-integer-in-c
*       https://www.computerhope.com/jargon/f/file-descriptor.htm#:~:text=On%20a%20Unix%2Dlike%20operating,and%20STDERR%20(standard%20error).&text=The%20default%20data%20stream%20for,keyboard%20input%20from%20the%20user.
*
*/
