#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "inputparser.h"

struct input_buffer{
    char s_line[MAX_LINE_LENGTH + 1];
    char *it;
    char *end;
} in_buf, s_buf;

int end_of_file();
int read_and_set();
char * iterate(char * i);
int bufend_check(char * argv, char * position);
void too_long_line();
char * getnextline();

int end_of_file(){
    return in_buf.end == in_buf.s_line;
}
int read_and_set(){
	int b = read(0, in_buf.s_line, MAX_LINE_LENGTH);
	
	if(b < 0)
		return b;

	in_buf.it = in_buf.s_line;
    in_buf.end = in_buf.s_line + b;
	return b;
}
char * iterate(char * i){
	char * iterator = i;
	while(iterator != in_buf.end && *(iterator) != '\n'){
        iterator++;
    }
	return iterator;
}
int bufend_check(char * argv, char * position){
	if(position != in_buf.end){
        *argv = '\0';
        in_buf.it = position + 1;
    	return 1;
    }
	return 0;
}
void too_long_line(){
	char *y = in_buf.it; int i;
    if(in_buf.it != NULL){
		y = iterate(in_buf.it);             
		if(y != in_buf.end){
	    	in_buf.it = y + 1;
	    	return;
		}
   	}

    while(1){
    	i = read_and_set();
    	y = iterate(in_buf.it);
        if(y != in_buf.end){
        	in_buf.it = y + 1;
            return;
       	}
    }
}

char * getnextline(){
    int bytes;
    char *position;

    if(in_buf.it == NULL || in_buf.it == in_buf.end){
        bytes = read_and_set();
		
        if(bytes < 0)
            return NULL;
        else if(bytes == 0){
            in_buf.s_line[0] = '\0';
            return in_buf.s_line;
        }
    }

    position = iterate(in_buf.it);

	char * r = in_buf.it;
	if(bufend_check(position, position))
		return r;

    s_buf.end = s_buf.s_line;
    memcpy(s_buf.end, in_buf.it, (position - in_buf.it)* sizeof(char));
    s_buf.end += (position - in_buf.it);

    while(1){
        bytes = read_and_set();

        if(bytes < 0)
            return NULL;
        else if(bytes == 0){
            *s_buf.end = '\0';
            return s_buf.s_line;
        }

        position = iterate(in_buf.s_line);

        if((s_buf.end - s_buf.s_line) + (position - in_buf.it) > MAX_LINE_LENGTH)
            s_buf.end = s_buf.s_line;
        else{
            memcpy(s_buf.end, in_buf.it, (position - in_buf.it)* sizeof(char));
            s_buf.end += (position - in_buf.it);
        }

        if(s_buf.end == s_buf.s_line){
            too_long_line();
            return NULL;
        }

		if(bufend_check(s_buf.end, position))
			return s_buf.s_line;
    }
}
