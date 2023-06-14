#define OPTION_QUANTITY 13
#define TEMP_SENSOR_QUANTITY 12
#define BYPASS_CHARGE_CURRENT "500000"

typedef struct
{
    char name[40];
    int value;
} option_struct;

extern option_struct options[OPTION_QUANTITY];
extern char option_file[];
extern char bypass_charge_file[];
extern char temp_sensors[TEMP_SENSOR_QUANTITY][15];
extern volatile char ForegroundAppName[100];
