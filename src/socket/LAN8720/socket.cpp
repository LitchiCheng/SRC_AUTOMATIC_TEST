#include "socket.h"
#include <string.h>
#include "NetworkAdapter.h"
#include "lwip_comm.h"
#include "lwip_sys.h"
#include "sockets.h"
#include "lwip_comm.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "ringque.h"
namespace{
}

static int validSocket(int s);
static int allocSocket();
static int freeSocket(int s);
static struct udp_pcb * socketPcb(int s);
static int setSocketRecvCallBack(int s, udp_recv_fn recv);
static int runAtSocketRecv(udp_pcb * pPcb, struct pbuf *p, uint32_t remoteip, uint16_t remoteport);
static int recvSocketMsg(int s, uint8_t* buff, uint16_t bufflen, uint32_t * remote_ip, uint16_t * remote_port);

int net::socket(int domain, int type, int protocol)
{
	return allocSocket();
}

int net::sendto(int s, const void *dataptr, size_t size, int flags,
    const struct sockaddr *to, socklen_t tolen)
{
	int ret = 0;
	ret = validSocket(s);
	if(ret < 0)
	{
		return ret;
	}
	
	struct pbuf *ptr;
	uint16_t sendsize = size;
	ptr=pbuf_alloc(PBUF_TRANSPORT, sendsize, PBUF_POOL); //申请内存
	
	udp_pcb * pcb = socketPcb(s);
	
	if(pcb)//创建成功
	{ 
//		pcb->remote_ip.addr = 0x0FC0A8C0;
		uint32_t taddr = ((sockaddr_in*)to)->sin_addr.s_addr;
		pcb->remote_ip.addr = taddr;
		pcb->remote_port = ntohs(((sockaddr_in*)to)->sin_port);
	}
	
	if(ptr)
	{
		memset(ptr->payload, 0 , ptr->len);
		memcpy(ptr->payload, dataptr, sendsize); 
//		ptr->payload = (void*)dataptr;
		udp_sendto(pcb, ptr, &pcb->remote_ip, pcb->remote_port);
		pbuf_free(ptr);//释放内存
	} 
		
	return ret;
}

void udp_recv_callback(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{
	runAtSocketRecv(upcb, p, addr->addr, port);
	return ;
}

int net::recvfrom(int s, void *mem, size_t bufflen, int flags, struct sockaddr *from, socklen_t *fromlen)
{
	int ret = 0;
	uint32_t remoteIp = 0;
	uint16_t remotePort = 0;
	setSocketRecvCallBack(s, udp_recv_callback);
	ret = recvSocketMsg(s, (uint8_t*)mem, bufflen, &remoteIp, &remotePort);
	
	if (ret > 0)
	{
		((sockaddr_in*)from)->sin_addr.s_addr = remoteIp;
		((sockaddr_in*)from)->sin_port = htons(remotePort);
	}
	return ret;
}

int net::bind(int s, const struct sockaddr *name, socklen_t namelen)
{
	int ret = 0;
	
	uint16_t port = ntohs(((sockaddr_in*)name)->sin_port);
	
	ret = udp_bind(socketPcb(s), IP_ADDR_ANY, port);//绑定本地IP地址与端口号
	
	return ret;
}

int net::close(int s)
{
	if(s>0) return freeSocket(s);
	else return -1;
}



//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

#include "sockets.h"

#define MAX_SOCK_PBUF_NUM 5
typedef struct pbuf * ppbuf;
typedef struct {
	ppbuf pp;
	uint32_t remote_ip;
	uint16_t remote_port;
} spbuf;

typedef struct sockinit_o	{
	bool allocated;
	udp_pcb *udppcb;
	bool hasRecvCallback;
	ringque<spbuf, MAX_SOCK_PBUF_NUM> splist;
public:
	sockinit_o(void)
	{
		allocated = false;
		udppcb = NULL;
		hasRecvCallback = false;
	}
} sockinit_t;
	
static sockinit_t socketPool[MEMP_NUM_UDP_PCB];

static int allocSocket()
{
	if(!NetworkAdapter::Instance()->isOpen())
		return -1;
	
	for(int i = 0; i < MEMP_NUM_UDP_PCB; i++)
	{
		if(false == socketPool[i].allocated)
		{
			socketPool[i].allocated = true;
			socketPool[i].udppcb = udp_new();
			if(socketPool[i].udppcb)
			{
				return i;
			}
			else
			{
				return -1;
			}
		}
	}
	return -2;
}

static udp_pcb * socketPcb(int s)
{
	if(validSocket(s) < 0)
	{
		return NULL;
	}
	
	return socketPool[s].udppcb;
}

static int freeSocket(int s)
{
	if(false == socketPool[s].allocated)
		return 0;
	
	socketPool[s].allocated = false;

	SYS_ARCH_PROTECT();
	while (socketPool[s].splist.elemsInQue() > 0)
	{
		ppbuf ppb = socketPool[s].splist.front().pp;
		pbuf_free(ppb);
		socketPool[s].splist.pop();
	}
	SYS_ARCH_UNPROTECT();
	// udp_disconnect(socketPool[s].udppcb); 
	udp_remove(socketPool[s].udppcb);		//断开UDP连接 
	return 0;
}

static int validSocket(int s)
{
	if(!NetworkAdapter::Instance()->isOpen()) return -1;
	
	if(s < 0) return -2;
	
	if(s >= MEMP_NUM_UDP_PCB) return -3;
	
	if(false == socketPool[s].allocated) return -4;
	
	if(NULL == socketPool[s].udppcb) return -5;
	
	return 0;
}

static int setSocketRecvCallBack(int s, udp_recv_fn recv)
{
	int ret = 0;
	
	if(socketPool[s].hasRecvCallback)
		return 0;
	
	if((ret = validSocket(s)) < 0)
	{
		return ret;
	}
	
	udp_pcb * pcb = socketPcb(s);

	udp_recv(pcb, recv, NULL);
	socketPool[s].hasRecvCallback = true;
	return ret;
}

static int runAtSocketRecv(udp_pcb * pPcb, struct pbuf *p, uint32_t remoteip, uint16_t remoteport)
{
	int so = -1;
	for(int i = 0; i < MEMP_NUM_UDP_PCB; i++)
	{
		if(pPcb == socketPool[i].udppcb)
		{
			so = i;
			break;
		}
	}
	
	if(so < 0)
		return -1;
	
	static int throwAwayCount = 0;
	
	spbuf sp;
	sp.pp = p;

	// SYS_ARCH_PROTECT();	// in interrupt function, no need to protect
	sp.remote_ip = remoteip;
	sp.remote_port = remoteport;
	bool ret = socketPool[so].splist.push(sp);
	// SYS_ARCH_UNPROTECT();// in interrupt function, no need to protect
	if (false == ret)
	{
		pbuf_free(p);
		throwAwayCount++;
		printf("throw away a frame because new frame arrived, for the %d time\r\n",throwAwayCount);
		return 1;
	}
	
	return 0;
}

static int recvSocketMsg(int s, uint8_t* buff, uint16_t bufflen, uint32_t * remote_ip, uint16_t * remote_port)
{
	
	struct pbuf *q;
	struct pbuf *p = NULL;
	uint32_t data_len = 0;
	
	SYS_ARCH_PROTECT();
	if(socketPool[s].splist.elemsInQue() > 0)
	{
		spbuf sp = socketPool[s].splist.front();
		p = sp.pp;
		*remote_ip = sp.remote_ip;
		*remote_port = sp.remote_port;
		socketPool[s].splist.pop();
	}
	SYS_ARCH_UNPROTECT();

	if(p != NULL)
	{
		memset(buff, 0, bufflen);
		for(q=p;q!=NULL;q=q->next)
		{
			if(q->len > (bufflen - data_len)) memcpy(buff + data_len, q->payload, (bufflen-data_len));//拷贝数据
			else memcpy(buff + data_len, q->payload, q->len);
			data_len += q->len;  	
			if(data_len > bufflen) break; //超出TCP客户端接收数组,跳出	
		}
		pbuf_free(p);
		if(0x00 == buff[0] 
			&& 0x00 == buff[1] 
			&& 0x00 == buff[2] 
			&& 0x00 == buff[3] )
	{
			static int a = 0;
			a++;
	}
	
	return data_len;
}
	return 0;	
}

//end of file
