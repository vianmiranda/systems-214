enum {
    SUCCESS,
    FAILURE
};

int get_exit_status();
void set_exit_status(int status);

void cd(char* path);
void pwd();
void which(char* program);