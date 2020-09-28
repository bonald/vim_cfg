/* (C) 2002 Yon Uriarte <havanna_moon@gmx.net>
 * ripped from port.c by J.Leu
 * Licensed under the GPL
 */
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "../../lib/include/glb_const.h"

int main(int argc, char **argv, char **envp) 
{
    int vrf = -1;
    char *vrf_name = NULL;
    int fd;

    if(argc < 3 ) 
    {
        printf("Usage: %s vrfnumber command <commandopts>\n", argv[0]);
        exit(1);
    }
  
    if(strcmp(argv[1], "-n") == 0)  
        vrf_name = argv[2];
    else
        vrf = atoi(argv[1]);

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        perror("socket");
        exit(-1);
    }

    if (vrf_name)
    {
        if (setsockopt(fd, SOL_SOCKET, SO_SETVRFID, vrf_name, 16) < 0) 
        {
            printf("%% No such VRF\n");
            exit(-1);
        }
    }
    else
    {
        if (vrf >= 0)
        {
            if (setsockopt(fd, SOL_SOCKET, SO_PERMVRF, (char *)&vrf, sizeof(vrf)) < 0) 
            {
                printf("%% setsocketopt SO_PERMVRF");
                exit(-1);
            }
        }
            else
                return -1;
    }

    if(vrf_name)
        execvp(argv[3],&argv[3]);
    else
        execvp(argv[2],&argv[2]);
    if( errno == EACCES )
        printf("Program %s not found.\n", argv[2]);
    else if( errno == ENOEXEC )
        printf("Program %s not executable.\n", argv[2]);

  exit(-2);
}
