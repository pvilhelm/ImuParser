/*
 * RcvRemoteCtrlUdpTx.cpp
 *
 *  Created on: 20 nov 2015
 *      Author: tomatsoppa
 */

#include "RcvRemoteCtrlUdpTx.h"


RcvRemoteCtrlUdpTx::RcvRemoteCtrlUdpTx(uint16_t RcvUdpPort, uint32_t RcvIp, int RcvComSocket) {
	// TODO Auto-generated constructor stub
	this->RcvUdpPort = RcvUdpPort;
	this->RcvComSocket = RcvComSocket;
	tempPacketBuffer = new char[1800];//longer then it needs to be
	pthread_mutex_init(&this->RcvRefValuesLock, NULL);
	pthread_mutex_init(&this->UdpTxLock, NULL);
	this->RcvSocketAdress.sin_family = AF_INET;
	this->RcvSocketAdress.sin_port = RcvUdpPort;
	this->RcvSocketAdress.sin_addr.s_addr = (in_addr_t)htonl(RcvIp) ;

}

RcvRemoteCtrlUdpTx::~RcvRemoteCtrlUdpTx() {
	// TODO Auto-generated destructor stub
	close(timerfd);
	pthread_mutex_destroy(&this->RcvRefValuesLock);
	pthread_mutex_destroy(&this->UdpTxLock);
}

int RcvRemoteCtrlUdpTx::SetRefStrAngle(float refStrAngle){
	pthread_mutex_lock( &(this->RcvRefValuesLock));
	this->rcvRefValues.refStrAngle = refStrAngle;
	pthread_mutex_unlock( &(this->RcvRefValuesLock));
	return 0;
}

int RcvRemoteCtrlUdpTx::SetRefTrq(float refTrq){
	refTrq = refTrq>1?1:(refTrq<-1?-1:refTrq); //check boundaries
	pthread_mutex_lock( &(this->RcvRefValuesLock));
	this->rcvRefValues.refTrq = refTrq;
	pthread_mutex_unlock( &(this->RcvRefValuesLock));
	return 0;
}

int RcvRemoteCtrlUdpTx::SetRefPars(ushort refPars) {
	pthread_mutex_lock( &(this->RcvRefValuesLock));
	this->rcvRefValues.refPars = refPars;
	pthread_mutex_unlock( &(this->RcvRefValuesLock));
	return 0;
}

pthread_t RcvRemoteCtrlUdpTx::StartTxThread() {
	pthread_t threadID;
	//Start the receiver thread
	timerfd = timerfd_create(CLOCK_MONOTONIC,0);
	struct itimerspec timspec;
	bzero(&timspec, sizeof(timspec));
	timspec.it_interval.tv_sec = 0;
	timspec.it_interval.tv_nsec = this->threadPeriodms * 1000000;
	timspec.it_value.tv_sec = 0;
	timspec.it_value.tv_nsec = 1;
	int res = timerfd_settime(timerfd, 0, &timspec, 0);
	if(res < 0){
	   std::perror("timerfd_settime:");
	}

	if(pthread_create(&threadID,NULL,ThreadEntryFunction,this)) {
		std::printf("Unable to create RCV Remote UPD TX thread");
		exit(-1);
	}
	return threadID;
}

int RcvRemoteCtrlUdpTx::SetPkgNr(uint32_t pkgNr) {
	pthread_mutex_lock( &(this->RcvRefValuesLock));
	this->rcvRefValues.pkgNr = pkgNr;
	pthread_mutex_unlock( &(this->RcvRefValuesLock));
	return 0;
}

void* RcvRemoteCtrlUdpTx::ThreadEntryFunction(void* thisPointer) {
	// A static entry function for receiveUDP
	((RcvRemoteCtrlUdpTx*)thisPointer)->SendRefToRcv();
	return NULL;
}

int RcvRemoteCtrlUdpTx::SendRefToRcvUdp() {
	//function just to send the message
	pthread_mutex_lock( &(this->UdpTxLock));
	tempPacketBuffer[0]=85;
	tempPacketBuffer[1]=1;
	float fwd = this->rcvRefValues.refTrq>= 0.0f ? this->rcvRefValues.refTrq:0.0f;
	float bwd = this->rcvRefValues.refTrq< 0.0f  ?-this->rcvRefValues.refTrq:0.0f;

	pthread_mutex_lock( &(this->RcvRefValuesLock));
	*(float*)&tempPacketBuffer[2]=htobe32(  *((uint32_t*)&(this->rcvRefValues.refStrAngle)));
	*(float*)&tempPacketBuffer[18]=htobe32( *((uint32_t*)&(fwd)));
	*(float*)&tempPacketBuffer[22]=htobe32( *((uint32_t*)&(bwd)));
	*(uint16_t*)&tempPacketBuffer[26]=(uint16_t)htobe16(this->rcvRefValues.refPars);
	*(uint32_t*)&tempPacketBuffer[28]=(uint32_t)htobe32(++this->rcvRefValues.pkgNr);
	pthread_mutex_unlock( &(this->RcvRefValuesLock));

	uint16_t hash = 0;
	for(int i=0;i<32;i++)
		hash+=tempPacketBuffer[i];

	*(uint16_t*)&tempPacketBuffer[32]=hash;

	ssize_t len = sendto(this->RcvComSocket,tempPacketBuffer,LEN_TX_DGRM,0,
			(struct sockaddr *)&this->RcvSocketAdress ,sizeof(this->RcvSocketAdress));
	if(len<0){
		std::perror("Error sending UDP to RCV:\n");
	}
	pthread_mutex_unlock( &(this->UdpTxLock));
}

void RcvRemoteCtrlUdpTx::SendRefToRcv(void) {
	//thread for timing the send the ref buffer to the RCV

	int res = 0;
	while(res = read(timerfd, &expirations, sizeof(expirations))){
		if(res<0){
			perror("Timer error UDP tx:\n");
			exit(-1);
		}
		SendRefToRcvUdp();
	}
}
