#define OPTION_QUANTITY 11
#define PRINTF_WITH_TIME_MAX_SIZE 400

struct option_struct
{
    char name[];
    int value;
};

extern struct option options[OPTION_QUANTITY];
extern char chartmp[PRINTF_WITH_TIME_MAX_SIZE];
extern char option_file[];
extern char temp_sensors[12][15];
