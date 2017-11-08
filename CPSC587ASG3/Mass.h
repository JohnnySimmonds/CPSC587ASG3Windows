#pragma once
#include <glm\glm\glm.hpp>
#include <glm\glm\gtc\matrix_transform.hpp>
using namespace glm;

class Mass 
{

public:
	Mass();
	~Mass();
	void resolveForces(float dt);
	vec3 getVelocity();
	void setVelocity(vec3 v);
	float getMass();
	void setMass(float m);
	vec3 getPosition();
	void setPosition(vec3 p);
	vec3 getForce();
	void setForce(vec3 f);
	vec3 getFixedPoint();
	void setFixedPoint(vec3 fix);
	vec3 getPrevPosition();
	void setPrevPosition(vec3 p);
	void setIsFixed(bool isFix);
	bool getIsFixed();
	void printVec3(vec3 toPrint);
	void setCalced(bool isCalced);
	bool getCalced();
	bool isDrawn();
	void setIsDrawn(bool isDrawn);
	vec3 getNewPos();
	void setNewPos(vec3 posNew);
	void setIsCube(bool cube);
	void setNewVel();
	void setIsMassA(bool isMA);
	
private:
	bool drawn = false;
	bool calced = false;
	float mass;
	vec3 newPos;
	vec3 pos;
	vec3 prevPos;
	vec3 velocity;
	vec3 vel = vec3(0.0f, 0.0f, 0.0f);
	vec3 force;
	vec3 fixedPoint;
	bool isCube = false;
	bool isFixed = false;
	bool isMassA = false;

};
