#include "line_parameters_2d_f32.h"

float LineParametric2D_F32::getSlopeX()
{
	return slope_.x;
}
float LineParametric2D_F32::getSlopeY()
{
	return slope_.y;
}

int LineParametric2D_F32::getX()
{
	return p_.x;
}
int LineParametric2D_F32::getY()
{
	return p_.y;
}