#ifndef TCP_PACKET_CACHE_INCLUDED
#define TCP_PACKET_CACHE_INCLUDED
#include "list.h"
struct tcp_packet_cache
{
    //data pointer,actually it's whole ip packet
    char * data ;
    //data length
    int len;
    //list item
    struct list_head list;
    // the byte order in this tcp_flow
    int seq;
    //How many time this packet has been transmited
    int retrans_count;
};

#endif // TCP_PACKET_CACHE_INCLUDED
