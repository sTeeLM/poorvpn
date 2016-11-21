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


#define CACHE_HASH_SIZE          1024
#define CACHE_TTL_SEC            60

struct addr_cache_t
{
    struct  list_head link;
    uint8_t eth[ETH_ALEN];
    struct sockaddr_in dst;
    time_t  time_stamp;
};

struct list_head cache_header[CACHE_HASH_SIZE];

int addr_hash(uint8_t * dst_mac)
{
    unsigned long val = dst_mac[0] + dst_mac[1] + dst_mac[2] + dst_mac[3] + dst_mac[4] + dst_mac[5];
    return val % CACHE_HASH_SIZE;
}

struct addr_cache_t * new_cache_entry(uint8_t * dst_mac, struct sockaddr_in * dst_addr)
{
    struct addr_cache_t * ret = NULL;
    ret = malloc(sizeof(struct addr_cache_t));
    if(NULL != ret) {
        memset(ret, 0, sizeof(struct addr_cache_t));
        memcpy(ret->eth, dst_mac, sizeof(ret->eth));
        memcpy(&ret->dst, dst_addr, sizeof(ret->dst));
        ret->time_stamp = time(NULL);
        INIT_LIST_HEAD(&ret->link);
        SDBG("NEW cache entry %02x:%02x:%02x:%02x:%02x:%02x->%s:%d\n", 
            ret->eth[0], ret->eth[1],ret->eth[2],ret->eth[3],ret->eth[4],ret->eth[5],
            inet_ntoa(ret->dst.sin_addr),
            htons(ret->dst.sin_port)
        );
    }
    return ret;
}

void free_cache_entry(struct addr_cache_t * entry)
{

    if(NULL != entry) {
        SDBG("DEL cache entry %02x:%02x:%02x:%02x:%02x:%02x->%s:%d\n", 
            entry->eth[0], entry->eth[1],entry->eth[2],entry->eth[3],entry->eth[4],entry->eth[5],
            inet_ntoa(entry->dst.sin_addr),
            htons(entry->dst.sin_port)
         );
        free(entry);
    }
}

struct addr_cache_t * lookup_cache(uint8_t * dst_mac)
{
    int i;
    struct list_head *head = NULL;
    struct addr_cache_t *pos = NULL; 

    i = addr_hash(dst_mac);

    head = &cache_header[i];

    list_for_each_entry(pos, head, link) {
        if(!memcmp(dst_mac, pos->eth, sizeof(pos->eth))) {
            SDBG("lookup_cache: %02x:%02x:%02x:%02x:%02x:%02x -> %s:%d\n", 
                pos->eth[0], pos->eth[1],pos->eth[2],pos->eth[3],pos->eth[4],pos->eth[5],
                inet_ntoa(pos->dst.sin_addr),
                htons(pos->dst.sin_port));
            return pos;
        }
    }
    SDBG("lookup_cache: %02x:%02x:%02x:%02x:%02x:%02x failed\n", 
        dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
    return NULL;
}

struct addr_cache_t * update_cache(uint8_t * dst_mac, struct sockaddr_in * dst_addr)
{
    struct addr_cache_t * ret = NULL;
    int i;

    ret = lookup_cache(dst_mac);
    i = addr_hash(dst_mac);

    if(NULL == ret) {
        ret = new_cache_entry(dst_mac, dst_addr);
        if(NULL != ret) {
            list_add(&cache_header[i], &ret->link);
        }
    } else {
        memcpy(&ret->dst, dst_addr, sizeof(ret->dst));
        ret->time_stamp = time(NULL);
    }

    return ret;
}

void clear_cache(void)
{
    int i;
    time_t current = time(NULL);
    struct addr_cache_t * pos = NULL, *n = NULL; 
    struct list_head *head = NULL;

    for(i = 0 ; i < CACHE_HASH_SIZE; i ++) {
        head = &cache_header[i];
        list_for_each_entry_safe(pos, n, head, link){
            if(pos->time_stamp + CACHE_TTL_SEC < current) {
                list_del(&pos->link);
                free_cache_entry(pos);
            }
        }
    }
}

