#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "config.h"
#include "siparse.h"
#include "redirections.h"

struct my_redirs{
	const char * file;
	int flags;
	int file_desc;
} myredirs;

void set_myredirs(redirection **redirs);
int redirections(redirection **redirs);

int redirections(redirection **redirs){
	if(redirs == NULL)
		return 0;

	redirection **r;
	for(r = redirs; *r != NULL; r++){
		set_myredirs(r);
	
		int fd = open(myredirs.file, myredirs.flags, OPEN_FLAGS);
		
		if(fd == -1){
			int err = errno;
					
			write(2, myredirs.file, strlen(myredirs.file));
			if(err == ENOENT)
    	        write(2, ": no such file or directory\n", 28);
    	    else if(err == EACCES)
    	        write(2, ": permission denied\n", 20);
		
			return -1;
		}
		if(fd != myredirs.file_desc){
			if(dup2(fd, myredirs.file_desc) == -1)
				exit(EXEC_FAILURE);
			if(close(fd) == -1)
				close(myredirs.file_desc);
		}
	} 
	return 0;
}
void set_myredirs(redirection **r){
	myredirs.file = (*r) -> filename;
	if(IS_RIN((*r)->flags)){
		myredirs.flags = O_RDONLY;
		myredirs.file_desc = 0;
	}
	else if(IS_ROUT((*r)->flags)){
		myredirs.flags = O_WRONLY | O_CREAT | O_TRUNC;
		myredirs.file_desc = 1;
	}
	else if(IS_RAPPEND((*r)->flags)){
		myredirs.flags = O_WRONLY | O_CREAT | O_APPEND;
		myredirs.file_desc = 1;
	}
}

