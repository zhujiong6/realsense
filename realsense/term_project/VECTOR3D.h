#pragma once
#include <math.h>
#include <iostream>


typedef float dataType;
#define M_PI 3.1415926


typedef class VECTOR3D
{
public:
	dataType x, y, z;


	VECTOR3D(dataType x = 0.0f, dataType y = 0.0f, dataType z = 0.0f);
	void operator()(dataType x = 0.0f, dataType y = 0.0f, dataType z = 0.0f);
	~VECTOR3D();

	void print();

	VECTOR3D operator-();
	VECTOR3D operator*(dataType scale);
	VECTOR3D operator/(dataType scale);
	void operator*=(dataType scale);
	void operator/=(dataType scale);
	dataType magnitude();
	void normalize();

	void operator=(VECTOR3D mVector);
	bool operator==(VECTOR3D mVector);
	VECTOR3D operator+(VECTOR3D mVector);
	VECTOR3D operator-(VECTOR3D mVector);
	void operator+=(VECTOR3D mVector);
	void operator-=(VECTOR3D mVector);
	dataType dot(VECTOR3D mVector);
	VECTOR3D cross(VECTOR3D mVector);
	dataType angle(VECTOR3D mVector);

	VECTOR3D findVertex(VECTOR3D pt, dataType scale);


	static dataType distance(VECTOR3D pt0, VECTOR3D pt1);
}VECTOR3D, POINT3D;