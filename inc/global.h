#define OPTION_QUANTITY 12
#define PRINTF_WITH_TIME_MAX_SIZE 400

struct option_struct
{
    char name[40];
    int value;
};

extern struct option_struct options[OPTION_QUANTITY];
extern char chartmp[PRINTF_WITH_TIME_MAX_SIZE];
extern char option_file[];
extern char temp_sensors[12][15];
