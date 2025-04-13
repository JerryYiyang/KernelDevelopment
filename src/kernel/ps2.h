// ports
#define PS2_DATA 0x60
#define PS2_CMD 0x64
#define PS2_STATUS PS2_CMD
#define PS2_STATUS_OUTPUT 1
#define PS2_STATUS_INPUT (1 << 1)

// commands
#define PS2_READ_CONFIG 0x20
#define PS2_WRITE_CONFIG PS2_DATA
#define PS2_DISABLE_PORT1 0xAD
#define PS2_DISABLE_PORT2 0xA7

static char ps2_poll_read(void);
static char ps2_read_data(void);
static void ps2_write_data(char data);
static void ps2_write_command(char cmd)
void ps2_init(void);
static void ps2_wait_write(void);
static void ps2_wait_read(void);