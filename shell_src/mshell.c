#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include "config.h"
#include "inputparser.h"
#include "siparse.h"
#include "utils.h"
#include "pipes.h"

int main(int argc, char *argv[]) {
    line * ln;
    command *cmd;
	char *sline;
    struct sigaction sig_act;
    struct stat buffer;

	int ftat = fstat(0, &buffer);

    if(ftat == -1)
        exit(EXIT_FAILURE);

	sig_act.sa_flags = SA_RESTART;
	sigemptyset(&sig_act.sa_mask);
	sig_act.sa_handler = s_handler;
	sigaction(SIGCHLD, &sig_act, NULL);
	sig_act.sa_handler = SIG_IGN;
	sigaction(SIGINT, &sig_act, NULL);

    while(1){
        if(isatty(0) == 1){
			check_background_commands();
            write(1, PROMPT_STR, strlen(PROMPT_STR));
		}

        sline = getnextline();
        
        if(sline == NULL){
            write(2, SYNTAX_ERROR_STR, strlen(SYNTAX_ERROR_STR));
            write(2, "\n", 1);
            continue;
        }
        
        if(end_of_file() == 1)
            break;

        ln = parseline(sline);
        
        if(ln == NULL){
        	write(2, SYNTAX_ERROR_STR, strlen(SYNTAX_ERROR_STR));
            write(2, "\n", 1);
            continue;
        }
		
		check_and_run_line(ln);		
    }
    return 0;
}
