#define OPTION_QUANTITY 11
#define PRINTF_WITH_TIME_MAX_SIZE 400

typedef struct option_struct
{
    char name[40];
    int value;
}options_array;

extern options_array options[OPTION_QUANTITY];
extern char chartmp[PRINTF_WITH_TIME_MAX_SIZE];
extern char option_file[];
extern char temp_sensors[12][15];
