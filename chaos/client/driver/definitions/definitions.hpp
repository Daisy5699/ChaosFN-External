#define ioctl_rw
#define ioctl_get_base_address
#define ioctl_get_guarded_region
#define ioctl_move_mouse
#define ioctl_get_cr3

#define securitycheck           0xDEADBEEF
#define device_name              L"chaos da goat"

typedef struct _rw_request
{
    INT32       security_check;
    INT32       process_id;
    ULONGLONG   address;
    ULONGLONG   buffer;
    ULONGLONG   size;
    BOOLEAN     is_write;
    ULONGLONG   cr3;
} rw_request;

typedef struct _base_address_request
{
    INT32       security_check;
    INT32       process_id;
    ULONGLONG   base_address;
} base_address_request;

typedef struct _guarded_region_request
{
    INT32       security_check;
    ULONGLONG   region_address;
} guarded_region_request;

typedef struct _mouse_move_request
{
    INT32       security_check;
    LONG        delta_x;
    LONG        delta_y;
    USHORT      button_flags;
} mouse_move_request;

typedef struct _cr3_request
{
    INT32       security_check;
    INT32       process_id;
    ULONGLONG   cr3;
} cr3_request;