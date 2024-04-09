#include <signal.h>
#include "arraylist.h"

void cd(arraylist_t* tokens);
void pwd();
char* handle_program_path(char* program);
void which(arraylist_t* tokens);
void exit_shell(arraylist_t* tokens);