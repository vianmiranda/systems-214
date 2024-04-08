#include "status.h"

static int exitStatus = UNDEFINED;

int get_exit_status() {
    return exitStatus;
}

void set_exit_status(int status) {
    exitStatus = status;
}