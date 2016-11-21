#include <stdio.h>

int compare_mac(uint8_t * mac, uint8_t * self)
{
    return memcmp(self, mac, ETH_ALEN) == 0;
}

int is_broad_cast_mac(uint8_t * mac)
{
    char mac_str[18]; /* 42:0A:6B:04:8F:B0 */
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return strcmp("FF:FF:FF:FF:FF:FF", mac_str) == 0;
}

int parse_mac( uint8_t *mac, const char *mac_str )  
{  
    char buffer[18];
    int ret = 0, i;
    unsigned int tmp[6];

    strncpy(buffer, mac_str, sizeof(buffer));
    buffer[17] = 0;

    ret = sscanf(buffer, "%2x:%2x:%2x:%2x:%2x:%2x", 
        (unsigned int *)&tmp[0], 
        (unsigned int *)&tmp[1], 
        (unsigned int *)&tmp[2], 
        (unsigned int *)&tmp[3], 
        (unsigned int *)&tmp[4], 
        (unsigned int *)&tmp[5]);

    for(i = 0 ; i < ETH_ALEN; i ++)
        mac[i] = (uint8_t)tmp[i];

    if(ret == 6) {
        return 0;
    }
    return -1;
}

void
rand_mac(uint8_t * mac, char * mac_str)
{
    int i;
    srand(time(NULL));

    mac[0] = 0x04;
    mac[1] = 0x09; /* Cratos Networks( and my B day ) */

    for(i = 2 ; i < ETH_ALEN ; i++) {
        mac[i] = rand() % 256;
    }

    snprintf(mac_str, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

