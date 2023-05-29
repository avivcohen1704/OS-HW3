#define MAJOR_NUM 235


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include "message_slot.h"

#define BUFFER_SIZE 128
int num;
MODULE_LICENSE("GPL");

// ======================== DEVICE STRUCTURES ===============================

struct cord {
    unsigned int channel;
    char message[BUFFER_SIZE];
    size_t length;
    struct cord* next;
};

struct vector {
    unsigned int minor;
    struct cord* cord;
    struct vector* next;
};

// ======================= GLOBAL VARIABLES ==================================

struct vector* head = NULL;

// ======================== HELPER FUNCTIONS ================================

int free_all_mem(void);

// ======================== DEVICE FUNCTIONS ================================

static int device_open(struct inode* inode, struct file* file) {
    struct vector* curr_vec = head;
    int minor;

    printk("invoking device_open");

    file->private_data = NULL;

    minor = iminor(inode);
    while (curr_vec != NULL) {
        if (curr_vec->minor == minor)
            break;
        curr_vec = curr_vec->next;
    }

    if (curr_vec == NULL) {
        curr_vec = (struct vector *)kmalloc(sizeof(struct vector), GFP_KERNEL);
        if (curr_vec == NULL){
            printk("couldnt allocate memory for vector");
            return -5;
        }
        curr_vec->minor = minor;
        curr_vec->cord = NULL;
        curr_vec->next = NULL;
        head = curr_vec;
    }
    printk("succesful device_open");
    return 0;
}

//--------------------------------------------------------------------------

static int device_release(struct inode* inode, struct file* file) {
    return 0;
}

//--------------------------------------------------------------------------

static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset) {
    unsigned int minor = iminor(file->f_inode);
    unsigned int channel = (unsigned int)(uintptr_t)file->private_data;
    struct cord* curr_cord;
    struct vector* curr_vec = head;
    unsigned int i;
    int channel_exists = 0;
    char curr_buffer[BUFFER_SIZE];

    printk("invoking device_write");

    if (channel == 0){
        return -EINVAL;
    }

    if(length <= 0 || length > BUFFER_SIZE){
        return -EMSGSIZE;
    }


    while (curr_vec != NULL) {
        if (curr_vec->minor == minor){
            printk("we found the minor %d", minor);
            break;}
        curr_vec = curr_vec->next;
    }
    if (curr_vec == NULL) {
        return -EINVAL;
    }

    if (curr_vec->cord == NULL){
        curr_vec->cord = (struct cord *)kmalloc(sizeof(struct cord), GFP_KERNEL);
        curr_cord = curr_vec->cord;
        if (curr_cord == NULL){
            return -EINVAL;
        }
        curr_cord->next = NULL;
        curr_cord->channel = channel;
        curr_cord->length = length;
        channel_exists = 1;
    }
    
    curr_cord = curr_vec->cord;
    while (curr_cord != NULL && (channel_exists == 0)) {
        if (curr_cord->channel == channel) {
            printk("we have found the channel %d", channel);
            curr_cord->length = length;
            channel_exists = 1;
            break;
        }
        if(curr_cord->next == NULL){
            curr_cord->next = (struct cord *)kmalloc(sizeof(struct cord), GFP_KERNEL);
            if (curr_cord->next == NULL){
                return -EINVAL;
            }
            curr_cord = curr_cord->next;
            curr_cord->next = NULL;
            curr_cord->channel = channel;
            curr_cord->length = length;
            channel_exists = 1;
            break;
        }
        curr_cord = curr_cord->next;
    }
    if (channel_exists == 0) {
        return -EINVAL;
    }
    for (i = 0; i < length; i++) {
        if (get_user(curr_buffer[i], &buffer[i]) != 0) {
            return -EINVAL;
        }
    }
    
    
    memcpy(curr_cord->message, curr_buffer, length);



    return i;
}

//--------------------------------------------------------------------------

static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset) {
    unsigned int minor = iminor(file->f_inode);
    unsigned int channel = (unsigned int)(uintptr_t)file->private_data;
    struct cord* curr_cord;
    struct vector* curr_vec = head;
    unsigned int i, j;
    char curr_buffer[BUFFER_SIZE];

    printk("invoking device_read");
    
    if (channel == 0){
        printk("================one");
        return -EINVAL;
    }

    while (curr_vec != NULL) {
        if (curr_vec->minor == minor){
            printk("we have found the minor %d", minor);
            break;
            }
        curr_vec = curr_vec->next;
    }
    if (curr_vec == NULL) {
        printk("================two");
        return -1;
    }

    curr_cord = curr_vec->cord;
    if (curr_cord == NULL) {
        printk("================three");
        return -EWOUILDBLOCK;
    }
    while (curr_cord != NULL) {
        
        if (curr_cord->channel == channel){

            break;}
        curr_cord = curr_cord->next;
    }
    
    if (curr_cord == NULL) {
        printk("================four");
        return -EWOUILDBLOCK;
    }


    if(curr_cord->message == NULL || curr_cord->length == 0){
        printk("================five");
        return -EWOUILDBLOCK;
    }
    
    if (length < curr_cord->length) {
        printk("================six");
        return -ENOSPC;
    }

    for(i=0; i< curr_cord->length; i++){
        if (get_user(curr_buffer[i], &buffer[i]) < 0) {
            for(j=0; j<i; j++){
                put_user(curr_buffer[j], &buffer[j]);
            }
            printk("================seven");
            return -1;
        }
        put_user(curr_cord->message[i], &buffer[i]);
    }
    printk("================eight");
    return i;
}

//--------------------------------------------------------------------------

static long device_ioctl(struct file* file,
                         unsigned int ioctl_command_id,
                         unsigned long ioctl_param) {

    unsigned long channel = ioctl_param;

    printk("invoking device_ioctl");
    
    if (ioctl_command_id != MSG_SLOT_CHANNEL) {
        return -EINVAL;
    }
    if (channel == 0){
        return -EINVAL;
    }

    file->private_data = (void*)(uintptr_t)ioctl_param;

    printk("succesful ioctl");

    return channel;
}

//--------------------------------------------------------------------------

static struct file_operations Fops = {
    .owner = THIS_MODULE,
    .write = device_write,
    .read = device_read,
    .open = device_open,
    .unlocked_ioctl = device_ioctl,
    .release = device_release,
};

// ======================== INIT & CLEANUP ==================================

static int __init simple_init(void) {
    int rc;
    num = 0;
    rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops);
    if (rc < 0) {
        printk(KERN_ALERT "%s registration failed for %d\n", DEVICE_FILE_NAME, MAJOR_NUM);
        return MAJOR_NUM;
    }
  printk( "Registeration is successful. ");
  printk( "If you want to talk to the device driver,\n" );
  printk( "you have to create a device file:\n" );
  printk( "mknod /dev/%s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM );
  printk( "You can echo/cat to/from the device file.\n" );
  printk( "Dont forget to rm the device file and "
          "rmmod when you're done\n" );    return 0;
}

//---------------------------------------------------------------------------

static void __exit simple_cleanup(void) {
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
    free_all_mem();
}

//---------------------------------------------------------------------------

int free_all_mem(void) {
    struct cord* curr_cord;
    struct vector* curr_vec = head;
    while (curr_vec != NULL) {
        curr_cord = curr_vec->cord;
        while (curr_cord != NULL) {
            curr_vec->cord = curr_cord->next;
            kfree(curr_cord);
            curr_cord = curr_vec->cord;
        }
        head = curr_vec->next;
        kfree(curr_vec);
        curr_vec = head;
    }
    return 1;
}

module_init(simple_init);
module_exit(simple_cleanup);
