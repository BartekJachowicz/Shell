#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <stdlib.h>
#include "siparse.h"
#include "builtins.h"

int lexit(char*[]);
int echo(char*[]);
int lcd(char*[]);
int lkill(char*[]);
int lls(char*[]);
int undefined(char *[]);

builtin_pair builtins_table[]={
	{"exit",	&lexit},
	{"lecho",	&echo},
	{"lcd",		&lcd},
	{"lkill",	&lkill},
	{"lls",		&lls},
	{NULL,      NULL}
};

int builtins_check(command * c, builtin_pair *builtins){
	char * argv = *(c -> argv);
    if(argv == NULL)
        return -1;

    int id = 0;
    while(builtins_table[id].name != NULL){
        if(strcmp(builtins_table[id].name, argv) == 0){
			builtins->name = builtins_table[id].name;
			builtins->fun = builtins_table[id].fun;
            return id;
        }
        id++;
    }
    return -1;
}

int echo( char * argv[]) {
	int i =1;
	if (argv[i]) printf("%s", argv[i++]);
	while  (argv[i])
		printf(" %s", argv[i++]);

	printf("\n");
	fflush(stdout);
	return 0;
}

int lexit(char * argv[]) {
    exit(0);
    return 0;
}

int lcd( char * argv[]) {
    const char *arg = argv[1];
    if(arg == NULL)
        arg = getenv("HOME");
    else if(argv[2] != NULL){
        write(2, "Builtin lcd error.\n", strlen("Builtin lcd error.\n"));
	return 2;    
	}

    int ret;
    ret = chdir(arg);

    if(ret == -1){
        write(2, "Builtin lcd error.\n", strlen("Builtin lcd error.\n"));
    }
    return ret;
}

int lkill( char * argv[]) {
    int sig = SIGTERM;
    int pid;
    char *pend = NULL;

    if(argv[1] == NULL){
        write(2, "Builtin lkill error.\n", strlen("Builtin lkill error.\n"));
	return 2;
    }

    if(argv[1][0] == '-' && argv[2] != NULL){
        pid = strtol(argv[2], &pend, 10);
        sig = strtol(argv[1] + 1, &pend, 10);
    }
    else {
        pid = strtol(argv[1], &pend, 10);
    }

    int k = kill(pid, sig);

    if(k == -1)
        write(2, "Builtin lkill error.\n", strlen("Builtin lkill error.\n"));

    return 0;
}

int lls( char * argv[]) {
    DIR * mydir;
    struct dirent *myfile;

    mydir = opendir(".");

    if(mydir == NULL) {
        write(2, "Builtin lls error.\n", strlen("Builtin lls error.\n"));
        return 0;
    }

    while((myfile = readdir(mydir)) != NULL){
        if(*(myfile -> d_name) == '.')
            continue;

        write(1, myfile -> d_name, strlen(myfile -> d_name));
        write(1, "\n", 1);
    }
    closedir(mydir);
    return 0;
}

int undefined( char * argv[]) {
	fprintf(stderr, "Command %s undefined.\n", argv[0]);
	return BUILTIN_ERROR;
}
