#ifndef __TCP_PACKET_CACHE_INCLUDED
#define __TCP_PACKET_CACHE_INCLUDED
#include "list.h"

#include <stdint.h>

struct tcp_packet_cache
{
    //data pointer,actually it's whole ip packet
    char * data ;
    //data length,where it's ip total len
    uint32_t len;
    //list item
    struct list_head list;
    // the byte order in this tcp_flow
    uint32_t seq;//payload
    uint32_t seq_end;//payload
    //How many time this packet has been transmited
    uint32_t retrans_count;
};

struct tcp_payload_cache
// used by rcv_ofo to store  of out order payload.
{
    char *payload;
    uint32_t len;
    struct list_head list;
    uint32_t seq;
    uint32_t seq_end;

};
#endif // TCP_PACKET_CACHE_INCLUDED
