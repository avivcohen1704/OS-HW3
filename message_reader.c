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
    char buffer[128];

    if (argc != 3){
        fprintf(stderr, "%s\n", strerror(errno));
        

        exit(1);
    }
    int ret_val;
    int fd = open(argv[1],O_RDONLY);
    if(fd < 0){
        fprintf(stderr, "%s\n", strerror(errno));

        exit(1);
    }   
    unsigned int channel = atoi(argv[2]);
    ret_val = ioctl(fd, MSG_SLOT_CHANNEL, channel);
    if (ret_val < 0){
        fprintf(stderr, "%s\n", strerror(errno));

        exit(1);
    }
    // here is supposed to be the reading of the msg to the buffer and then printing it to stdout using write() func
    ret_val = read(fd, buffer, 128);
    if (ret_val < 0){
        fprintf(stderr, "%s", strerror(errno));
        
        exit(1);
    }
    if(close(fd) < 0){
        fprintf(stderr, "%s\n", strerror(errno));

        exit(1);
    }
    ret_val = write(STDOUT_FILENO, buffer, ret_val) < ret_val;

    if(ret_val < 0){
        fprintf(stderr, "%s\n", strerror(errno));

        exit(1);
    }
    
    
    return 0;
}
