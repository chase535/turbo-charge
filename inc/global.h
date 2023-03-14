#define OPTION_QUANTITY 10
#define PRINTF_WITH_TIME_MAX_SIZE 400

typedef unsigned char uchar;
typedef unsigned int uint;

extern uint opt_old[OPTION_QUANTITY],opt_new[OPTION_QUANTITY];
extern uchar tmp[5];
extern char chartmp[PRINTF_WITH_TIME_MAX_SIZE];
extern char option_file[];
extern char options[OPTION_QUANTITY][40];

void line_feed(char *line);
void check_read_file(char *file);
