/*
 * RcvImuUdpRx.h
 *
 *  Created on: 19 nov 2015
 *      Author: tomatsoppa
 */

#include "RcvState.h"
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef RCVIMUUDPRX_H_
#define RCVIMUUDPRX_H_

class RcvImuUdpRx {
public:
	~RcvImuUdpRx();
	bool exitThread = false;
	RcvState rcvState;
	RcvImuUdpRx(int udpPort);
	pthread_t StartReceiverThread(void);
	int udpSocket;

private:
	static void* ThreadEntryFunction(void* thisPointer);

	int CheckImuPkg(char* pkgBuffer);
	int CheckHash(char* pkgBuffer);
	void ReceiveIMUData();
	char* tempPacketBuffer;

	void TryToReceiveImuPacket();

	struct sockaddr_in socketAddressMe, socketAddressOther; // Socket adr
	socklen_t socketLength;

};

#endif /* RCVIMUUDPRX_H_ */
