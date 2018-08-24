#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "uartd_test_mode.h"

#include "debug.h"

#define SEND_PORT 8060

int sockfd = -1;
struct sockaddr_in dest_address;


static int create_broadcast_socket(char* ifname)
{
    int b_opt_val = 1;
    struct ifreq ifr;
    int ret = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sockfd)
    {
        printf("fail to create socket");
        ret = -1;
        goto RETURN;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &b_opt_val, sizeof(int)) == -1)
    {
        printf("fail to setopt broadcast");
        ret = -1;
        goto CLOSE_FD;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_ifrn.ifrn_name, ifname, strlen(ifname));
    ifr.ifr_addr.sa_family = AF_INET;

    if ((ioctl(sockfd, SIOCGIFFLAGS, &ifr)>= 0)&&(ifr.ifr_flags&IFF_UP))
    {
        if (setsockopt(sockfd,SOL_SOCKET,SO_BINDTODEVICE,ifname,strlen(ifname)+1) == -1 )
        {
            printf("setsockopt %s SO_BINDTODEVICE: %m", ifname);
            ret = -1;
            goto CLOSE_FD;
        }
    }
    else
    {
        printf("bind interface %s fail", ifname);
        ret = -1;
        goto CLOSE_FD;
    } 

    memset(&dest_address, 0, sizeof(dest_address));
    dest_address.sin_family      = AF_INET;
    dest_address.sin_addr.s_addr = htonl(INADDR_BROADCAST); 
    dest_address.sin_port        = htons(SEND_PORT);

    return ret;

CLOSE_FD:
    close(sockfd);
    sockfd = -1;

RETURN:
    return ret;
}


static int send_broadcast_to_interface(char* buffer, char* ifname)
{
	int i;
    ssize_t cnt = 0;
    int ret = 0;

    if (-1 == sockfd)
    {
        if (0 != create_broadcast_socket(ifname))
        {
            printf("create_socket %s fail", ifname);
            ret = -1;
            goto RETURN;
        }
    }

	for (i = 0; i < 10; i++)
	{
    	cnt = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&dest_address, (int)sizeof(dest_address));
		usleep(1000 * 100);
	}

    
    if (0>cnt) 
    {
        printf("sendto fail");
        ret = -1;
    } 
    else 
    {
        printf("%s send broadcast %s\n", ifname, buffer); 
    }

    close(sockfd);
    sockfd=-1;

RETURN:

    return ret;
}

int send_cmd_to_server(char cmd)
{
    int ret = -1;
    char mac_cmd[20]={0};
    char mac[20]={0};

	char *pTestMode = NULL;					

	/*pTestMode = WIFIAudio_UciConfig_SearchValueString("xzxwifiaudio.config.testmode");
	if (0 == strcmp(pTestMode, "0"))
	{
		if (pTestMode)
			free(pTestMode);

		return -1;
	}

	if (pTestMode)
		free(pTestMode);*/

    if(access("/tmp/test_mode",0))
    {
        return -1;  //no test signal
    }

	
    //if (cdb_get_str("$wanif_mac", mac, sizeof(mac), NULL) != NULL) 
    //boot_cdb_get("mac0", mac);
    cdb_get("$boot_mac0", mac);
    {
        sprintf(mac_cmd,"%s%02d",mac,cmd);
    }
    
	DEBUG_INFO("send_broadcast_to_interface:%s", mac);
    if (0==send_broadcast_to_interface(mac_cmd, "br0"))
    {
        ret=0;
    }
    if (0==send_broadcast_to_interface(mac_cmd, "br1"))
    {
        ret=0;
    }
    return ret;
}

