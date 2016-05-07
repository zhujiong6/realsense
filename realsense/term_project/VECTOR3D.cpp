#include "VECTOR3D.h"


VECTOR3D::VECTOR3D(dataType x /*= 0.0f*/, dataType y /*= 0.0f*/, dataType z /*= 0.0f*/) { this->x = x;  this->y = y;  this->z = z; }


void VECTOR3D::operator()(dataType x /*= 0.0f*/, dataType y /*= 0.0f*/, dataType z /*= 0.0f*/) { this->x = x;  this->y = y;  this->z = z; }


VECTOR3D::~VECTOR3D() {}


void VECTOR3D::print() { std::cout << "Vector: " << x << "\t" << y << "\t" << z << std::endl; }


VECTOR3D VECTOR3D::operator-() { return VECTOR3D(-x, -y, -z); }


VECTOR3D VECTOR3D::operator*(dataType scale) { return VECTOR3D(scale*x, scale*y, scale*z); }


VECTOR3D VECTOR3D::operator/(dataType scale) { return VECTOR3D(x / scale, y / scale, z / scale); }


void VECTOR3D::operator*=(dataType scale) { x *= scale;  y *= scale; z *= scale; }


void VECTOR3D::operator/=(dataType scale) { x /= scale;  y /= scale; z /= scale; }


dataType VECTOR3D::magnitude() { return sqrt(x*x + y*y + z*z); }


void VECTOR3D::normalize() { dataType norm = magnitude();  x /= norm;  y /= norm;  z /= norm; }


void VECTOR3D::operator=(VECTOR3D mVector) { x = mVector.x;  y = mVector.y;  z = mVector.z; }


bool VECTOR3D::operator==(VECTOR3D mVector) { return mVector.x == x && mVector.y == y && mVector.z == z ? true : false; }


VECTOR3D VECTOR3D::operator+(VECTOR3D mVector) { return VECTOR3D(x + mVector.x, y + mVector.y, z + mVector.z); }


VECTOR3D VECTOR3D::operator-(VECTOR3D mVector) { return VECTOR3D(x - mVector.x, y - mVector.y, z - mVector.z); }


void VECTOR3D::operator+=(VECTOR3D mVector) { x += mVector.x;  y += mVector.y; z += mVector.z; }


void VECTOR3D::operator-=(VECTOR3D mVector) { x -= mVector.x;  y -= mVector.y; z -= mVector.z; }


dataType VECTOR3D::dot(VECTOR3D mVector) { return x*mVector.x + y*mVector.y + z*mVector.z; }


VECTOR3D VECTOR3D::cross(VECTOR3D mVector) { return VECTOR3D(y*mVector.z - mVector.y*z, mVector.x*z - x*mVector.z, x*mVector.y - mVector.x*y); }


dataType VECTOR3D::angle(VECTOR3D mVector)
{
	dataType radian = acos(this->dot(mVector)/(this->magnitude()*mVector.magnitude()));
	return radian/(dataType)(2.0f*M_PI)*(dataType)180.0f;
}


VECTOR3D VECTOR3D::findVertex(VECTOR3D pt, dataType scale) { return VECTOR3D(pt.x + x*scale, pt.y + y*scale, pt.z + z*scale); }


dataType VECTOR3D::distance(VECTOR3D pt0, VECTOR3D pt1) { return sqrt((pt0.x - pt1.x)*(pt0.x - pt1.x) + (pt0.y - pt1.y)*(pt0.y - pt1.y) + (pt0.z - pt1.z)*(pt0.z - pt1.z)); }