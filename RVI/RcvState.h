/*
 * RcvState.h
 *
 *  Created on: 19 nov 2015
 *      Author: tomatsoppa
 */
//using namespace std;
#include <cstdint>
#include <chrono>
#include <pthread.h>

#ifndef RCVSTATE_H_
#define RCVSTATE_H_

struct Struct_RcvState{
	//dont change order or padding of this plz =) (add stuff to end)
	float strAngle;
	float trq;
	float vxWheel;
	float ax;
	float thrlPerc;
	float batU;
	float strWhlAngle;
	char errMsg[8];
	uint32_t modesAndStates;
	double latDeg;
	double longDeg;
	float vxImu;
	float headingImu;
	uint32_t pkgNr;
	std::chrono::system_clock::time_point timestamp;
	pthread_mutex_t RcvStateLock;

};

class RcvState {
public:
	RcvState();
	virtual ~RcvState();
	void UpdateState(char* udpPkg);//blocking
	Struct_RcvState GetStateCopy();//blocking
	Struct_RcvState PresentRcvState;

};

#endif /* RCVSTATE_H_ */
