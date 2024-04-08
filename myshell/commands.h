#include <signal.h>
#include "arraylist.h"

int cd(arraylist_t* tokens);
int pwd();
char* handle_program_path(char* program);
int which(arraylist_t* tokens);
int exit_shell(arraylist_t* tokens);