#ifndef message_slot_h
#define message_slot_h

#include <linux/ioctl.h>


#define IOCTL_SET_ENC _IOW(MAJOR_NUM, 0, unsigned long)
#define DEVICE_RANGE_NAME "IPC"
#define DEVICE_FILE_NAME "IPC"
#define MAJOR_NUM 235
#define MSG_SLOT_CHANNEL 12
#define EINVAL 22
#define EWOUILDBLOCK 11


#endif 

