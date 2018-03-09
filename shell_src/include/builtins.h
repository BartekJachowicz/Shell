#ifndef _BUILTINS_H_
#define _BUILTINS_H_

#define BUILTIN_ERROR 2

typedef struct {
	char* name;
	int (*fun)(char**); 
} builtin_pair;

int builtins_check(command * c, builtin_pair *builtins);
extern builtin_pair builtins_table[];

#endif /* !_BUILTINS_H_ */
