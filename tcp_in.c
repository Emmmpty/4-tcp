#include "tcp.h"
#include "tcp_sock.h"
#include "tcp_timer.h"

#include "log.h"
#include "ring_buffer.h"

#include <stdlib.h>
// update the snd_wnd of tcp_sock
//
// if the snd_wnd before updating is zero, notify tcp_sock_send (wait_send)
static inline void tcp_update_window(struct tcp_sock *tsk, struct tcp_cb *cb)
{
	u16 old_snd_wnd = tsk->snd_wnd;
	tsk->snd_wnd = cb->rwnd;
	if (old_snd_wnd == 0)
		wake_up(tsk->wait_send);
}

// update the snd_wnd safely: cb->ack should be between snd_una and snd_nxt
static inline void tcp_update_window_safe(struct tcp_sock *tsk, struct tcp_cb *cb)
{
	if (less_or_equal_32b(tsk->snd_una, cb->ack) && less_or_equal_32b(cb->ack, tsk->snd_nxt))
		tcp_update_window(tsk, cb);
}

#ifndef max
#	define max(x,y) ((x)>(y) ? (x) : (y))
#endif

// check whether the sequence number of the incoming packet is in the receiving
// window
static inline int is_tcp_seq_valid(struct tcp_sock *tsk, struct tcp_cb *cb)
{
	u32 rcv_end = tsk->rcv_nxt + max(tsk->rcv_wnd, 1);
	if (less_than_32b(cb->seq, rcv_end) && less_or_equal_32b(tsk->rcv_nxt, cb->seq_end)) {
		return 1;
	}
	else {
		log(ERROR, "received packet with invalid seq, drop it.");
		return 0;
	}
}

// Process the incoming packet according to TCP state machine. 
void tcp_process(struct tcp_sock *tsk, struct tcp_cb *cb, char *packet)
{
//	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);
    
    
    
    if(!is_tcp_seq_valid(tsk, cb))
    {
        return;
    }
    
    if(cb->flags == TCP_RST)
    {
        tcp_send_control_packet(tsk, TCP_ACK); 
        tcp_sock_close(tsk);
        return;
    }

    if(cb->flags == TCP_FIN)
    {
        tcp_sock_close(tsk);
        return;
    }

    int state = tsk->state;
    
    if(state == TCP_CLOSED)
    {
        //drop + rst
        tcp_send_reset(cb);
    }
    else if(state == LISTEN)
    {
        if(cb->flags == TCP_SYN)
        {
            //listen_queue

            struct *tcp_sock tmp= alloc_tcp_sock();
            tmp->sk_sip = cb->daddr;
            tmp->sk_sport = cb->dport;
            tmp->sk_dip = cb->saddr;
            tmp->sk_dport = cb->sport;
            
            tmp->parent = tsk;


            list_add_tail(&tmp->list, &tsk->parent->listen_queue)

            tcp_send_control_packet(tmp, TCP_SYN|TCP_ACK);//send syn+ack
            
            tcp_set_state(tmp, TCP_SYN_RECV);

            tcp_hash(tmp);//establish_table
        }
        else
        {
            tcp_send_reset(cb);
        
        }
    }
    else if(state == TCP_SYN_RECV)
    {
        if(cb->flag == TCP_ACK)
        {
            //establish
            tcp_sock_accept_enqueue(tsk);
            tcp_set_state(tsk, TCP_ESTABLISHED);
            wake_up(tsk->parent->wait_accept);//sleep_on
        }
        else
        {
           tcp_send_reset(cb);
        }
    }
    else if(state == TCP_SYN_SENT)
    {
        if(cb->flag == TCP_SYN|TCP_ACK)
        {
            tcp_set_state(tsk, TCP_ESTABLISHED);
            tcp_send_control_packet(tsk, TCP_ACK);
            wake_up(tsk->wait_connect);
        }
        else
        {
            tcp_send_reset(cb);
        }

    }
}
