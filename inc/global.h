#define OPTION_QUANTITY 13
#define TEMP_SENSOR_QUANTITY 12
#define PRINTF_WITH_TIME_MAX_SIZE 400
#define BYPASS_CHARGE_CURRENT "500000"

struct option_struct
{
    char name[40];
    int value;
};

extern struct option_struct options[OPTION_QUANTITY];
extern char chartmp[PRINTF_WITH_TIME_MAX_SIZE];
extern char option_file[];
extern char bypass_charge_file[];
extern char temp_sensors[TEMP_SENSOR_QUANTITY][15];
extern volatile char ForegroundAppName[100];
extern volatile int bypass_charge;
