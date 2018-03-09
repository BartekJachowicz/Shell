
#ifndef _RED_H_
#define _RED_H_

void set_myredirs(redirection **redirs);
int redirections(redirection **redirs);
#define OPEN_FLAGS S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH

#endif /* !_RED_H_ */


