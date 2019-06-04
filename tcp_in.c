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
	//fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);

	char cb_flags[32];
	tcp_copy_flags_to_str(cb->flags, cb_flags);
	log(DEBUG, "recived tcp packet %s", cb_flags);
	switch (tsk->state)
	{
	case TCP_CLOSED:
		tcp_sock_closed(tsk, cb, packet);
		return;
		break;
	case TCP_LISTEN:
		tcp_sock_listen(tsk, cb, packet);
		return;
		break;
	case TCP_SYN_SENT:
		tcp_sock_send(tsk, cb, packet);
		return;
		break;
	default:
		break;
	}

	if (!is_tcp_seq_valid(tsk, cb))
	{
		log(ERROR, "received tcp packet with invalid seq, drop it!");
		return;
	}

	if (cb->flags & TCP_RST)
	{
		//close this connection, and release the resources of this tcp sock
		tcp_set_state(tsk, TCP_CLOSED);
		tcp_unhash(tsk);
		return;
	}

	if (cb->flags & TCP_SYN)
	{
		//reply with TCP_RST and close this connection
		tcp_send_reset(cb);
		tcp_set_state(tsk, TCP_CLOSED);
		tcp_unhash(tsk);
		return;
	}

	if (!(cb->flags & TCP_ACK))
	{
		//drop
		log(ERROR, "received tcp packet without ack, drop it.");
		return;
	}
	//process the ack of the packet
	if (tsk->state == TCP_SYN_RECV)
	{
		tcp_s(tsk, cb, packet);
		return;
	}
	if (tsk->state == TCP_FIN_WAIT_1)
	{
		tcp_set_state(tsk, TCP_FIN_WAIT_2);
		return;
	}
	if (tsk->state == TCP_LAST_ACK)
	{
		tcp_set_state(tsk, TCP_CLOSED);
		tcp_unhash(tsk);
		return;
	}
	if (tsk->state == TCP_FIN_WAIT_2)
	{
		if (cb->flags != (TCP_FIN | TCP_ACK))
		{
			//drop
			log(ERROR, "received tcp packet without FIN|ACK, drop it.");
			return;
		}
		tsk->rcv_nxt = cb->seq_end;
		tcp_send_control_packet(tsk, TCP_ACK);
		// start a timer
		tcp_set_timewait_timer(tsk);
		tcp_set_state(tsk, TCP_TIME_WAIT);
		return;
	}
	//update rcv_wnd
	tsk->rcv_wnd -= cb->pl_len;
	//update snd_wnd
	tcp_update_window_safe(tsk, cb);
	//recive data
	if (cb->pl_len > 0)
		tcp_recv_data(tsk, cb, packet);

	if (cb->flags & TCP_FIN)
	{
		//update the TCP_STATE accordingly
		tcp_set_state(tsk, TCP_CLOSE_WAIT);
		tsk->rcv_nxt = cb->seq_end;
		tcp_send_control_packet(tsk, TCP_ACK);
		tcp_send_control_packet(tsk, TCP_FIN | TCP_ACK);
		tcp_set_state(tsk, TCP_LAST_ACK);
		return;
	}

	//reply with TCP_ACK if the connection is alive
	if (cb->flags != TCP_ACK)
	{
		tsk->rcv_nxt = cb->seq_end;
		tcp_send_control_packet(tsk, TCP_ACK);
	}
}
