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
#include "builtins.h"
#include "pipes.h"
#include "redirections.h"

struct proces {
	int pid[512];
	int status[512];
} background_closed_process, forground_process;

volatile int bg_size = 0, fg_size = 0;

void s_handler(int sig_num);
void check_background_commands(); 
void check_and_run_line(line * ln);
int check_pipeline(pipeline pipe_line);
int run_pipeline(pipeline pipe_line, int is_background);
void swap_file_descriptors_for_pipe(int file_desc[], int fd);
void do_exec(command *cmd);

void s_handler(int sig_num){
	volatile int child = 1; 
	int status, i, flag;
	while (child > 0){
		child = waitpid(-1, &status, WUNTRACED | WNOHANG);
		flag = 1; 
		for(i = 0; i < fg_size; i++){
			if(child == forground_process.pid[i]){
				forground_process.pid[i] = forground_process.pid[fg_size-1];
				fg_size--;
				flag = 0;
				break;
			}
		}		
		if(flag == 1 && child > 0){
			if(bg_size < 512){
				background_closed_process.pid[bg_size] = child;
				background_closed_process.status[bg_size] = status;
				bg_size++;
			}
		}
	}
}

void check_background_commands(){
	int i = 0;
	sigset_t mask_to_block, old_mask;
	sigemptyset(&mask_to_block);
	sigaddset(&mask_to_block, SIGCHLD);
	sigprocmask(SIG_BLOCK, &mask_to_block, &old_mask);
	
	for(i = 0; i < bg_size; i++){ 
		if(WIFEXITED(background_closed_process.status[i]))
			fprintf(stdout, "Background process %d terminated. (exited with status %d)\n", 
				background_closed_process.pid[i], WEXITSTATUS(background_closed_process.status[i]));
		else if(WIFSIGNALED(background_closed_process.status[i]))
			fprintf(stdout, "Background process %d terminated. (killed by signal %d)\n",
			 	background_closed_process.pid[i], WTERMSIG(background_closed_process.status[i]));
	}
	
	bg_size = 0;
	sigprocmask(SIG_SETMASK, &old_mask, NULL);
} 

void check_and_run_line(line * ln){
	pipeline *pipe;
	int is_background;
	
	for(pipe = ln->pipelines; *pipe != NULL; pipe++){
		if(check_pipeline(*pipe) == -1){
			write(2, SYNTAX_ERROR_STR, strlen(SYNTAX_ERROR_STR));
    	    write(2, "\n", 1);
			return;
		}
	}	
	for(pipe = ln->pipelines; *pipe != NULL; pipe++){	
		is_background = (ln->flags == LINBACKGROUND);
		run_pipeline(*pipe, is_background);
	}
}

int check_pipeline(pipeline pipe_line){	
	command **cmd;
	
	if(pipe_line == NULL)
       	return -1;
   	if(*pipe_line == NULL)
        return 0;
    if(*(pipe_line + 1) == NULL)
    	return 0;
    	
   	for(cmd = pipe_line; *cmd != NULL; ++cmd){
       	if((*cmd)->argv[0] == NULL)
       	    return -1;
	}
	return 0;
}

int run_pipeline(pipeline pipe_line, int is_background){
	command **cmd;
	int file_desc[2], fd = 0;
	int chpid, status, wpid;
	builtin_pair builtins;

	int builtins_table_id = builtins_check(*pipe_line, &builtins);
		
	if(builtins_table_id != -1){
		cmd = pipe_line; 
		builtins.fun((*cmd)->argv);
	}
	else {
		sigset_t mask_to_block, old_mask;
		sigemptyset(&mask_to_block);
		sigaddset(&mask_to_block, SIGCHLD);
		sigprocmask(SIG_BLOCK, &mask_to_block, &old_mask);
		
		for(cmd = pipe_line; *cmd != NULL; ++cmd){
			if((*cmd) == NULL || (*cmd)->argv[0] == NULL)
				continue;

			if(*(cmd+1) == NULL){
				file_desc[0] = 0;
				file_desc[1] = 1;
			}
			else {
				if(pipe(file_desc) == -1)
					exit(EXEC_FAILURE);
			}
			
			chpid = fork();
			
			if(chpid == -1)
				exit(EXEC_FAILURE);
			else if(chpid == 0){	
				if(is_background)
					setsid();
				
				struct sigaction sig_action;
				sigemptyset(&sig_action.sa_mask);
				sig_action.sa_handler = SIG_DFL; 
				sigaction(SIGINT, &sig_action, NULL);
    			sigaction(SIGCHLD, &sig_action, NULL);
    			sigprocmask(SIG_SETMASK, &sig_action.sa_mask, NULL);
				
				swap_file_descriptors_for_pipe(file_desc, fd);
            	
            	if(redirections((*cmd)->redirs) == -1)
            		exit(EXEC_FAILURE);
            		
                do_exec(*cmd);
			}
			else{
				if(is_background == 0){
					forground_process.pid[fg_size] = chpid;
					fg_size++;			
				}
				
				if(fd != 0)
            		close(fd);
        		if(file_desc[1] != 1)
            		close(file_desc[1]);
			}
			
			fd = file_desc[0];
		}
		
		if(fd != 0)
			close(fd);
			
		if(is_background == 0){
			sigset_t mask_int;
			sigemptyset(&mask_int);
			sigaddset(&mask_int, SIGINT);
			while(fg_size != 0){
				sigsuspend(&mask_int);
			}
		}	
		
		sigprocmask(SIG_SETMASK, &old_mask, NULL);
	}
	return 0;
}
void swap_file_descriptors_for_pipe(int file_desc[], int fd){
	if(file_desc[0] != 0)
		close(file_desc[0]);
	if(fd != 0){
		if(dup2(fd, 0) == -1)
			exit(EXEC_FAILURE);
		if(close(fd) == -1){
			close(0);
			exit(EXEC_FAILURE);
		}
	}
	if(file_desc[1] != 1){
		if(dup2(file_desc[1], 1) == -1)
			exit(EXEC_FAILURE);
		if(close(file_desc[1]) == -1){
			close(1);
			exit(EXEC_FAILURE);
		}
	}
}
void do_exec(command *cmd){
    int ret = execvp(cmd -> argv[0], cmd -> argv);

    if(ret == -1){
        int errnum = errno;

		write(2, cmd -> argv[0], strlen(cmd -> argv[0]));
        if(errnum == ENOENT)
            write(2, ": no such file or directory\n", 28);
        else if(errnum == EACCES)
            write(2, ": permission denied\n", 20);
        else 
            write(2, ": exec failure\n", 15);

        exit(EXIT_FAILURE);
    }
}
