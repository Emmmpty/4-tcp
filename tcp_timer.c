#include "tcp.h"
#include "tcp_timer.h"
#include "tcp_sock.h"

#include <stdio.h>
#include <unistd.h>

static struct list_head timer_list;

// scan the timer_list, find the tcp sock which stays for at 2*MSL, release it
void tcp_scan_timer_list()
{
    //fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);
    //2MSL: tcp_time.h TCP_TIMEWAIT_TIMEOUT
    //release: TCP_CLOSE
    struct tcp_timer *t;
    struct tcp_sock *tsk;
    list_for_each_entry(t, &timer_list, list_head)
    {
        if(t->timeout >= TCP_TIMEWAIT_TIMEOUT)
        {
            //tcp sock
            tsk = timewait_to_tcp_sock(t);
            tcp_set_state(tsk, TCP_CLOSED);
            free_tcp_sock(tsk);
        }
    
    }
    return;


}

// set the timewait timer of a tcp sock, by adding the timer into timer_list
void tcp_set_timewait_timer(struct tcp_sock *tsk)
{
	//fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);
    //tcp_sock struct tcp_timer timewait;

    struct tcp_timer *time_wait = &(tsk->timewait);
    /*
       
    struct tcp_timer {
        int type;   // time-wait: 0     retrans: 1
        int timeout;    // in micro second
        struct list_head list;
        int enable;
    };
    */
    time_wait->type = 0;
    time_wait->timeout = TCP_TIMEWAIT_TIMEOUT;
    list_add_tail(&(time_wait->list), &(timer_list));
    retranstimer->enable = 1;

}

// set the retrans timer of a tcp sock, by adding the timer into timer_list
void tcp_set_retrans_timer(struct tcp_sock *tsk)
{
	//fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);
    //struct tcp_timer retrans_timer;
    struct tcp_timer *retranstimer = &(tsk->retrans_timer);
    retranstimer->type = 1;
    retranstimer->timeout = TCP_TIMEWAIT_TIMEOUT;
    list_add_tail(&(retranstimer->list), &(timer_list)  );
    retranstimer->enable = 1;

}

// unset the retrans timer of a tcp sock, by removing the timer from timer_list
void tcp_unset_retrans_timer(struct tcp_sock *tsk)
{
	//fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);
    struct tcp_timer *retranstimer = &(tsk->retrans_timer);
    struct tcp_timer *t;
    list_for_each_entry_safe(t, &timer_list, list)
    {
        struct tcp_sock *tmp = timewait_to_tcp_sock(t);
        if(tmp->sk_sip == tsk->sk_sip && tmp->sk_sport == tsk->sk_sport
           tmp->sk_dip == tsk->sk_dip && tmp->sk_dport == tsk->sk_dport
        )
        {
            list_delete_entry(&t->list);
            retranstimer->enable = 0;
            return; 
        }
    }
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
