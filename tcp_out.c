#include "tcp.h"
#include "tcp_sock.h"
#include "ip.h"
#include "ether.h"

#include "log.h"
#include "list.h"
#include "tcp_packet_cache.h"

#include <stdlib.h>
#include <string.h>

//add a packet to tcp send_buf.
void tcp_add_send_buf(struct tcp_sock *tsk,char *packet,int len,u32 seq,u32 seq_end)
{
    struct tcp_packet_cache * item = (struct tcp_packet_cache*)malloc(sizeof(struct tcp_packet_cache));
    item->len = len;
    item->seq = seq;
    item->seq_end = seq_end;
    item->retrans_count = 0;
    item->data = (char *) malloc(len);
    memcpy(item->data,packet,len);
    if(list_empty(&tsk->send_buf))
    {
        list_add_tail(&item->list,&tsk->send_buf);
        tcp_set_retrans_timer(tsk);
    }
    else
    {
        list_add_tail(&item->list,&tsk->send_buf);
    }


}
// initialize tcp header according to the arguments
static void tcp_init_hdr(struct tcphdr *tcp, u16 sport, u16 dport, u32 seq, u32 ack,
		u8 flags, u16 rwnd)
{
	memset((char *)tcp, 0, TCP_BASE_HDR_SIZE);

	tcp->sport = htons(sport);
	tcp->dport = htons(dport);
	tcp->seq = htonl(seq);
	tcp->ack = htonl(ack);
	tcp->off = TCP_HDR_OFFSET;
	tcp->flags = flags;
	tcp->rwnd = htons(rwnd);
}

// send a tcp packet
//
// Given that the payload of the tcp packet has been filled, initialize the tcp
// header and ip header (remember to set the checksum in both header), and emit
// the packet by calling ip_send_packet.
void tcp_send_packet(struct tcp_sock *tsk, char *packet, int len)
{
	struct iphdr *ip = packet_to_ip_hdr(packet);
	struct tcphdr *tcp = (struct tcphdr *)((char *)ip + IP_BASE_HDR_SIZE);

	int ip_tot_len = len - ETHER_HDR_SIZE;
	int tcp_data_len = ip_tot_len - IP_BASE_HDR_SIZE - TCP_BASE_HDR_SIZE;

	u32 saddr = tsk->sk_sip;
	u32	daddr = tsk->sk_dip;
	u16 sport = tsk->sk_sport;
	u16 dport = tsk->sk_dport;

	u32 seq = tsk->snd_nxt;
	u32 ack = tsk->rcv_nxt;
	u16 rwnd = tsk->rcv_wnd;

	tcp_init_hdr(tcp, sport, dport, seq, ack, TCP_PSH|TCP_ACK, rwnd);
	ip_init_hdr(ip, saddr, daddr, ip_tot_len, IPPROTO_TCP);

	tcp->checksum = tcp_checksum(ip, tcp);

	ip->checksum = ip_checksum(ip);

    //need check!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    tsk->snd_nxt += tcp_data_len;
	//tsk->snd_wnd -= tcp_data_len;


    tcp_add_send_buf(tsk,packet,len,seq,tsk->snd_nxt);
    ip_send_packet(packet, len);


}

//retransmiss a packet.

void tcp_retrans_packet(struct tcp_sock * tsk,char *packet, int len,u32 seq,int retrans_count)
{
	struct iphdr *ip = packet_to_ip_hdr(packet);
	struct tcphdr *tcp = (struct tcphdr *)((char *)ip + IP_BASE_HDR_SIZE);

	int ip_tot_len = len - ETHER_HDR_SIZE;
	int tcp_data_len = ip_tot_len - IP_BASE_HDR_SIZE - TCP_BASE_HDR_SIZE;

	u32 saddr = tsk->sk_sip;
	u32	daddr = tsk->sk_dip;
	u16 sport = tsk->sk_sport;
	u16 dport = tsk->sk_dport;

	u32 ack = tsk->rcv_nxt;
	u16 rwnd = tsk->rcv_wnd;
    u8 flags = tcp->flags;

	tcp_init_hdr(tcp, sport, dport, seq, ack, flags, rwnd);
	ip_init_hdr(ip, saddr, daddr, ip_tot_len, IPPROTO_TCP);

	tcp->checksum = tcp_checksum(ip, tcp);

	ip->checksum = ip_checksum(ip);

	//tsk->snd_nxt += tcp_data_len;
	//tsk->snd_wnd -= tcp_data_len;
    char * _packet = (char *)malloc(len);
    memcpy(_packet,packet,len);
	ip_send_packet(_packet, len);
    char cb_flags[32];
	tcp_copy_flags_to_str(flags, cb_flags);
	printf("retrans:  seq:%d,ack:%d,count:%d,flags:%s.\n",seq,ack,retrans_count,cb_flags);
	//tsk->retrans_timer.timeout = min(64*1000*1000,tsk->RTO*(2<<retrans_count) );// set the retramsfer


}

// send a tcp control packet
//
// The control packet is like TCP_ACK, TCP_SYN, TCP_FIN (excluding TCP_RST).
// All these packets do not have payload and the only difference among these is
// the flags.
void tcp_send_control_packet(struct tcp_sock *tsk, u8 flags)
{
	int pkt_size = ETHER_HDR_SIZE + IP_BASE_HDR_SIZE + TCP_BASE_HDR_SIZE;
	char *packet = malloc(pkt_size);
	if (!packet) {
		log(ERROR, "malloc tcp control packet failed.");
		return ;
	}

	struct iphdr *ip = packet_to_ip_hdr(packet);
	struct tcphdr *tcp = (struct tcphdr *)((char *)ip + IP_BASE_HDR_SIZE);

	u16 tot_len = IP_BASE_HDR_SIZE + TCP_BASE_HDR_SIZE;

	ip_init_hdr(ip, tsk->sk_sip, tsk->sk_dip, tot_len, IPPROTO_TCP);
	tcp_init_hdr(tcp, tsk->sk_sport, tsk->sk_dport, tsk->snd_nxt, \
			tsk->rcv_nxt, flags, tsk->rcv_wnd);

	tcp->checksum = tcp_checksum(ip, tcp);

	if (flags & (TCP_SYN|TCP_FIN))
	{
        tcp_add_send_buf(tsk,packet,pkt_size,tsk->snd_nxt,tsk->snd_nxt+1);
		tsk->snd_nxt += 1;
    }
	ip_send_packet(packet, pkt_size);
}

// send tcp reset packet
//
// Different from tcp_send_control_packet, the fields of reset packet is
// from tcp_cb instead of tcp_sock.
void tcp_send_reset(struct tcp_cb *cb)
{
	int pkt_size = ETHER_HDR_SIZE + IP_BASE_HDR_SIZE + TCP_BASE_HDR_SIZE;
	char *packet = malloc(pkt_size);
	if (!packet) {
		log(ERROR, "malloc tcp control packet failed.");
		return ;
	}

	struct iphdr *ip = packet_to_ip_hdr(packet);
	struct tcphdr *tcp = (struct tcphdr *)((char *)ip + IP_BASE_HDR_SIZE);

	u16 tot_len = IP_BASE_HDR_SIZE + TCP_BASE_HDR_SIZE;
	ip_init_hdr(ip, cb->daddr, cb->saddr, tot_len, IPPROTO_TCP);
	tcp_init_hdr(tcp, cb->dport, cb->sport, 0, cb->seq_end, TCP_RST|TCP_ACK, 0);
	tcp->checksum = tcp_checksum(ip, tcp);

	ip_send_packet(packet, pkt_size);
}

int tcp_send_data(struct tcp_sock *tsk, char *buf, int len) {
	int pkt_len = ETHER_HDR_SIZE + IP_BASE_HDR_SIZE + TCP_BASE_HDR_SIZE + len;
	char *packet = (char*)malloc(pkt_len);
	if (packet == NULL)
		return -1;
	char* tcp_data = packet + ETHER_HDR_SIZE + IP_BASE_HDR_SIZE + TCP_BASE_HDR_SIZE;
	memcpy(tcp_data, buf,len);
	tcp_send_packet(tsk, packet, pkt_len);
	return 0;
}
