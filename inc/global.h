#define OPTION_QUANTITY 13
#define TEMP_SENSOR_QUANTITY 12
#define BYPASS_CHARGE_CURRENT "500000"

struct ListNode
{
    char name[40];
    volatile int value;
    struct ListNode *next;
};

typedef struct ListNode ListNode;

extern char option_file[];
extern char bypass_charge_file[];
extern char temp_sensors[TEMP_SENSOR_QUANTITY][15];

extern volatile char ForegroundAppName[100];
extern ListNode *options_head;

void node_init();
