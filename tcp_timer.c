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
    struct tcp_sock *tsk;
    struct tcp_timer *t, *c;
    list_for_each_entry_safe(tsk, t, &timer_list, hash_list);


}

// set the timewait timer of a tcp sock, by adding the timer into timer_list
void tcp_set_timewait_timer(struct tcp_sock *tsk)
{
	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);
}

// set the retrans timer of a tcp sock, by adding the timer into timer_list
void tcp_set_retrans_timer(struct tcp_sock *tsk)
{
	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);
}

// unset the retrans timer of a tcp sock, by removing the timer from timer_list
void tcp_unset_retrans_timer(struct tcp_sock *tsk)
{
	fprintf(stdout, "TODO: implement %s please.\n", __FUNCTION__);
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
