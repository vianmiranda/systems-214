#include "status.h"

static int exitStatus = UNDEFINED;
static int exit_flag = CONTINUE; 

int get_exit_status() {
    return exitStatus;
}

void set_exit_status(int status) {
    exitStatus = status;
}

int get_exit_flag() {
    return exit_flag;
}

void set_exit_flag(int flag) {
    exit_flag = flag;
}