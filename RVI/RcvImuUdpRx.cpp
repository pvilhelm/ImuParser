/*
 * RcvImuUdpRx.cpp
 *
 *  Created on: 19 nov 2015
 *      Author: tomatsoppa
 */

#include "RcvRemoteCtrlUdpTx.h"
#include "RcvImuUdpRx.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void* RcvImuUdpRx::ThreadEntryFunction(void* thisPointer) {
	// A static entry function for receiveUDP
	((RcvImuUdpRx*)thisPointer)->ReceiveIMUData();
	return NULL;
}

pthread_t RcvImuUdpRx::StartReceiverThread() {
	// Starts the receiver thread; an infinite loop that continuously updates rawLidarData

	pthread_t threadID;

	//Start the receiver thread
	if(pthread_create(&threadID,NULL,ThreadEntryFunction,this)) {
		printf("Unable to create IMU rx thread");
		exit(-1);
	}
	return threadID;
}

void RcvImuUdpRx::TryToReceiveImuPacket() {
	// A blocking method that tries to receive a lidar UDP packet. Returns if successful, keeps listening on socket if not
	int receivedPacketLength=0;
	while(true) {
		receivedPacketLength = recvfrom(udpSocket,tempPacketBuffer,1800,0,(struct sockaddr *) &socketAddressOther, &socketLength);
		if (receivedPacketLength==-1) {
			printf("Unable to receive UDP packet: %s\n", strerror(errno));
			exit(-1);
		}
		if (CheckImuPkg(tempPacketBuffer)) {return;}
	}
}

void RcvImuUdpRx::ReceiveIMUData() {
	printf("IMU UDP receiver thread listening for incoming IMU packages \n");

	while(!exitThread) {
		TryToReceiveImuPacket();
		// Copy the packet data from tempPacketBuffer to rawLidarBuffer, minus the last 6 bytes that are factory bytes
		pthread_mutex_lock( &(rcvState.PresentRcvState.RcvStateLock) );
		rcvState.UpdateState(tempPacketBuffer);
		pthread_mutex_unlock( &(rcvState.PresentRcvState.RcvStateLock) );
	}
	printf("IMU UDP receiver thread exited\n");
	pthread_exit(NULL);
}

RcvImuUdpRx::RcvImuUdpRx(int udpPort) {


	// First create the UDP socket:


	tempPacketBuffer = new char[1800];//longer then it needs to be
	udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket==-1) {
		printf("Unable to create socket: %s\n", strerror(errno));
		exit(-1);
	}

	// Set up the socket adress
	socketLength = sizeof(socketAddressMe);
	memset((char *) &socketAddressMe, 0, sizeof(socketAddressMe)); // Zero out socketAddressMe
	socketAddressMe.sin_family = AF_INET;
	socketAddressMe.sin_port = htons(udpPort);
	socketAddressMe.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind the socket to socketAddress
	if(bind(udpSocket , (struct sockaddr*)&socketAddressMe, sizeof(socketAddressMe)) == -1) {
		printf("Unable to bind socket to socketAddress: %s\n", strerror(errno));
		exit(-1);
	}

}

int RcvImuUdpRx::CheckImuPkg(char * pkgBuffer){
	if(pkgBuffer[0]==101)
		return 1;
	return 0;
}

int RcvImuUdpRx::CheckHash(char * pkgBuffer){
	return 1;//TODO
}


RcvImuUdpRx::~RcvImuUdpRx() {
	close(udpSocket);
}

int main(int argc, char** argv){
	//this is a test function for sending and receiving data from the RCV
	RcvImuUdpRx rcvImuUdpRx(3000);
	rcvImuUdpRx.StartReceiverThread();
	printf("qweqwwwwwwwwwe\n");
	RcvRemoteCtrlUdpTx test((uint16_t)4444,(uint32_t)0xeeeeeeee,rcvImuUdpRx.udpSocket);
	test.StartTxThread();
	test.SetRefStrAngle(0.1f);
	test.SetRefTrq(0.223f);
	printf("\n\n\nqweqddeeeeeeeeewe");
	for(;;)
		sleep(1000);
}

