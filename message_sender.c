#include "message_slot.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */

int main(int argc, char* argv[]){
    int ret_val;
    if (argc != 4){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }

    unsigned int channel = atoi(argv[2]);
    char* message = argv[3];
    char* path = argv[1];

    
    int fd = open(path, O_RDWR);
    if (fd < 0){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }


    ret_val = ioctl(fd, MSG_SLOT_CHANNEL, channel);
    if (ret_val < 0){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }


    ret_val = write(fd, message, strlen(message));
    if (ret_val < 0){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }


    ret_val = close(fd);
    if (ret_val < 0){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    exit(0);

}
