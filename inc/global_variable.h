#define OPTION_QUANTITY 10
#define PRINTF_WITH_TIME_MAX_SIZE 400

typedef unsigned char uchar;
typedef unsigned int uint;

extern unsigned int opt_old[OPTION_QUANTITY],opt_new[OPTION_QUANTITY];
extern unsigned char tmp[5];
extern char chartmp[PRINTF_WITH_TIME_MAX_SIZE];
extern char option_file[];
extern char options[OPTION_QUANTITY][40];
