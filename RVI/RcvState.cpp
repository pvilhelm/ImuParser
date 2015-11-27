/*
 * RcvState.cpp
 *
 *  Created on: 19 nov 2015
 *      Author: tomatsoppa
 */

#include "RcvState.h"
#include <cstring>
RcvState::RcvState() {
	pthread_mutex_init(&PresentRcvState.RcvStateLock, NULL);
}

RcvState::~RcvState() {
	pthread_mutex_lock(&PresentRcvState.RcvStateLock);//w8 for access
	//pthread_mutex_unlock(&PresentRcvState.RcvStateLock);
	pthread_mutex_destroy(&PresentRcvState.RcvStateLock);
}

void RcvState::UpdateState(char* udpPkg){
	//Updates the RCV state from a Udp pkg from the RCV
	pthread_mutex_lock( &PresentRcvState.RcvStateLock );
	for(int i = 0; i<69;i++){
		((char*)&PresentRcvState)[i]=udpPkg[i+1];
	}
	PresentRcvState.timestamp = std::chrono::system_clock::now();
	pthread_mutex_unlock(&PresentRcvState.RcvStateLock);
}

Struct_RcvState RcvState::GetStateCopy(){
	//returns a copy of the RCv state on the stack
	Struct_RcvState CopyOfRcvState;
	pthread_mutex_lock( &PresentRcvState.RcvStateLock );
	memcpy ( (void *) (&CopyOfRcvState), (const void *) (&PresentRcvState), sizeof(Struct_RcvState) );
	pthread_mutex_unlock(&PresentRcvState.RcvStateLock);
	return CopyOfRcvState;
}
