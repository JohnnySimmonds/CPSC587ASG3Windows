#pragma once
#include "Mass.h"
#include <iostream>
using namespace std;
class Spring
{
	
public:

	Spring(vec3 massAPos, vec3 massBPos, bool setFixedA, bool setFixedB);
	//Spring(Mass *mA, Mass *mB, bool setFixedA, bool setFixedB);
	~Spring();
	
	void applyForce(float dt);
	Mass* getMassA();
	Mass* getMassB();
	void setMassA(Mass *mA);
	void setMassB(Mass *mB);
	void setStiffness(float stiff);
	float getStiffness();
	//void setRestLength(float restLen);
	//float getRestLength();
	
	void setDampingCo(float damp);
	float getDampingCo();
	void zeroForce();//Mass *mA, Mass *mB);
	void printVec3(vec3 toPrint);
	void unCalced();
private:

	Mass *massA;
	Mass *massB;
	float k; //stiffness of the spring
	float restLengthA, restLengthB;
	float restLength;
	float dampingCo;

};
