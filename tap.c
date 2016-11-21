#include <arpa/inet.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_tun.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

int tap_create(char *dev, const struct sockaddr_in * ip_addr, const struct sockaddr_in * netmask, const uint8_t * mac)
{
    int fd = -1, s = -1, i;
    struct ifreq ifr;
    struct sockaddr mac_addr;

    /* create tap device */
    if ((fd = open(DEB_NET_TUN, O_RDWR)) < 0) {
        goto err;
    }
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;

    if (ioctl(fd, (TUNSETIFF), (void *)&ifr) != 0) {
        SERR("ioctl: %m\n");
        goto err;
    }
    /* save tap device name */
    strcpy(dev, ifr.ifr_name);

    /* create device socket fd */
    if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        SERR("socket: %m\n");
        goto err;
    }
    /* set mac with SIOCSIFHWADDR */
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, dev);
    memset(&mac_addr, 0, sizeof(mac_addr));
    mac_addr.sa_family = ARPHRD_ETHER;
    memcpy(mac_addr.sa_data, mac, 6);
    memcpy(&ifr.ifr_hwaddr, (char *) &mac_addr, sizeof(struct sockaddr));
    
    for( i = 0 ; i < ETH_ALEN; i ++) {
        ifr.ifr_hwaddr.sa_data[i] = mac[i];
    }
    SDBG( "set mac addr %02x:%02x:%02x:%02x:%02x:%02x on interface %s\n",
           mac[0],mac[1], mac[2], mac[3], mac[4], mac[5], ifr.ifr_name);
    if (ioctl(s, SIOCSIFHWADDR, &ifr) != 0) {
        SERR("ioctl: %m\n");
        goto err;
    }

    /* set ip addr with SIOCSIFADDR*/
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, dev);
    memcpy( (((char *)&ifr + offsetof(struct ifreq, ifr_addr) )),
                        ip_addr, sizeof(struct sockaddr));
    SDBG("set ip address %s on interface %s\n", inet_ntoa(ip_addr->sin_addr), ifr.ifr_name);
    if (ioctl(s, SIOCSIFADDR, &ifr) != 0) {
        SERR("ioctl: %m\n");
        goto err;
    }

    /* set net mask with SIOCSIFNETMASK*/
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, dev);
    memcpy( (((char *)&ifr + offsetof(struct ifreq, ifr_netmask) )),
                        netmask, sizeof(struct sockaddr));
    SDBG("set net mask %s on interface %s\n", inet_ntoa(netmask->sin_addr), ifr.ifr_name);
    if (ioctl(s, SIOCSIFNETMASK, &ifr) != 0) {
        SERR("ioctl: %m\n");
        goto err;
    }

    /* bring up interface */
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, dev);
    if(ioctl(s, SIOCGIFFLAGS, &ifr) != 0) {
        SERR("ioctl: %m\n");
        goto err;
    }
    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    if(ioctl(s, SIOCSIFFLAGS, &ifr) != 0) {
        SERR("ioctl: %m\n");
        goto err;
    }

    close(s);

    return fd;

err:
    if(fd != -1) {
        close(fd);
    }
    if(s != -1) {
        close(s);
    }
    return -1;
}

