/*
 * RcvRemoteCtrlUdpTx.h
 *
 *  Created on: 20 nov 2015
 *      Author: tomatsoppa
 */

#ifndef RCVREMOTECTRLUDPTX_H_
#define RCVREMOTECTRLUDPTX_H_

#define LEN_TX_DGRM ((size_t) 34) //length of the datagram thats sent to the RCV

#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include <pthread.h>
#include <sys/timerfd.h>
#include <string.h>
#include <stdlib.h>
#include <cstdio>
#include <unistd.h>

struct RcvRefValues{
	float refTrq = 0; 		//between -1 and 1 (as share of max trq)
	float refStrAngle = 0; 	//the reference steering angle in radians
	ushort refPars = 0;		//reference paramaters (not used)
	uint32_t pkgNr = 0;		//
};

class RcvRemoteCtrlUdpTx {
public:
	RcvRemoteCtrlUdpTx(uint16_t RcvUdpPort, uint32_t RcvIp, int RcvComSocket);
	~RcvRemoteCtrlUdpTx();
	pthread_t StartTxThread();
	int SetRefStrAngle(float refStrAngle);
	int SetRefTrq(float refTrq);
	int SetRefPars(uint16_t refPars);
	int SetPkgNr(uint32_t pkgNr);
	int SendRefToRcvUdp();
	bool exitThread = false;
private:
	static void* ThreadEntryFunction(void* thisPointer);
	void SendRefToRcv(void);
	RcvRefValues rcvRefValues;
	pthread_mutex_t RcvRefValuesLock;
	pthread_mutex_t UdpTxLock;
	char * tempPacketBuffer;
	int udpSocket;
	struct sockaddr_in socketAddressMe, socketAddressOther; // Socket adr
	socklen_t socketLength;
	char RcvIp[4];
	short RcvUdpPort;
	struct sockaddr_in RcvSocketAdress;
	int RcvComSocket;
	int timerfd;
	int threadPeriodms = 100;
	uint64_t expirations = 0;
};

#endif /* RCVREMOTECTRLUDPTX_H_ */
