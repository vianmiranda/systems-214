enum {
    SUCCESS,
    FAILURE,
    UNDEFINED
};

enum {
    CONTINUE,
    EXIT
};

int get_exit_status();
void set_exit_status(int status);

int get_exit_flag();
void set_exit_flag(int flag);