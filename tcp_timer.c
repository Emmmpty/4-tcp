#include "tcp_packet_cache.h"
#include "tcp.h"
#include "tcp_timer.h"
#include "tcp_sock.h"

#include <stdio.h>
#include <unistd.h>

static struct list_head timer_list;

// scan the timer_list, find the tcp sock which stays for at 2*MSL, release it
void tcp_scan_timer_list()
{
	//fprintf(stdout, "call %s .\n", __FUNCTION__);

	struct tcp_sock *tsk;
	struct tcp_timer *t, *q;
	list_for_each_entry_safe(t, q, &timer_list, list) {
        //if(t->enable == 0)
        //{
        //    continue;
        //}
        //printf("timer:type:%d,enable:%d,timeout:%d\n",t->type,t->enable,t->timeout);
		t->timeout -= TCP_TIMER_SCAN_INTERVAL;
		if (t->timeout <= 0) {



			if(t->type == 0)
			// only support time wait now
			{
                list_delete_entry(&t->list);
                tsk = timewait_to_tcp_sock(t);
                if (!tsk->parent)
                    tcp_bind_unhash(tsk);
                tcp_set_state(tsk, TCP_CLOSED);
                printf("timewait timeer.\n");
                free_tcp_sock(tsk);
			}
			if(t->type==1)
			//retransmiss timer
			//"TODO: implement %s please.\n", __FUNCTION__);
			{

                tsk = retranstimer_to_tcp_sock(t);
                if(!list_empty(&tsk->send_buf))
                {
                 //printf("retrans....\n");
                    tcp_retrans(tsk);

                    tsk->ssthresh = MSS*2 > (tsk->cwnd * MSS /2.0)? MSS*2 : (tsk->cwnd * MSS /2.0);
                    tsk->cwnd = 1;
                    congest_time(tsk);
                }
                //else
                //{
                 //   tcp_unset_retrans_timer(tsk);
                //}
			}

		}
	}
}

// set the timewait timer of a tcp sock, by adding the timer into timer_list
void tcp_set_timewait_timer(struct tcp_sock *tsk)
{
	//fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);

	struct tcp_timer *timer = &tsk->timewait;
    timer->enable = 1;
	timer->type = 0;
	timer->timeout = TCP_TIMEWAIT_TIMEOUT;
	list_add_tail(&timer->list, &timer_list);

	tcp_sock_inc_ref_cnt(tsk);
}

// set the retrans timer of a tcp sock, by adding the timer into timer_list
void tcp_set_retrans_timer(struct tcp_sock *tsk)
{
	//fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);
	static char setflag = 0;

    struct tcp_timer *retrans_timer = &tsk->retrans_timer;
    retrans_timer->type = 1;
    retrans_timer->enable =1;
    if (setflag!=0)
    {
        retrans_timer->timeout = TCP_RETRANS_INTERVAL_INITIAL;
    }
    else
    // first syn, the timeout should be very large.
    {
        retrans_timer->timeout = 5 * TCP_MSL;
        setflag = 1;
    }
    list_add_tail(&retrans_timer->list,&timer_list);
    tcp_sock_inc_ref_cnt(tsk);
}

// unset the retrans timer of a tcp sock, by removing the timer from timer_list
void tcp_unset_retrans_timer(struct tcp_sock *tsk)
{
	//fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);
    struct tcp_timer *retrans_timer = &tsk->retrans_timer;
    retrans_timer->type = 1;
    retrans_timer->timeout = TCP_RETRANS_INTERVAL_INITIAL;
    retrans_timer->enable = 0;

    list_delete_entry(&retrans_timer->list);
    tcp_sock_dec_ref_cnt(tsk);

}

// scan the timer_list periodically by calling tcp_scan_timer_list
void *tcp_timer_thread(void *arg)
{
	init_list_head(&timer_list);
	while (1) {
		usleep(TCP_TIMER_SCAN_INTERVAL);
		tcp_scan_timer_list();
	}

	return NULL;
}
