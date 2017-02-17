#include "line_parameters_2d_f32.h"
const float PI = 3.1415926;
/**
* <p>
* Computes the closest point along the line as a function of 't':<br>
* [x, y] = [x_0, y_0] + tÂ·[slopeX, slopeY]
* </p>
*
* @param line The line along which the closest point is being found. Not modified.
* @param p	A point. Not modified.
* @return Distance as a function of 't'
*/
float closestPointT( LineParametric2D_F32 line,	cv::Point2f p ) 
{
	float t = line.slope_.x * ( p.x - line.p_.x ) + line.slope_.y * ( p.y - line.p_.y );
	t /= line.slope_.x * line.slope_.x + line.slope_.y * line.slope_.y;

	return t;
}

/**
* <p>
* Returns the Euclidean distance squared of the closest point on the line from a point.
* </p>
*
* @param line A line segment. Not modified.
* @param p The point. Not modified.
* @return Euclidean distance squared to the closest point on the line is away from the point.
*/
float distanceSq( LineParametric2D_F32 line, cv::Point2f p ) {
	float t = closestPointT( line, p );

	float a = line.slope_.x * t + line.p_.x;
	float b = line.slope_.y * t + line.p_.y;

	float dx = p.x - a;
	float dy = p.y - b;

	return dx * dx + dy * dy;
}

/**
* <p>
* Returns the Euclidean distance of the closest point on the line from a point.
* </p>
*
* @param line A line segment. Not modified.
* @param p The point. Not modified.
* @return Distance the closest point on the line is away from the point.
*/
float distance( LineParametric2D_F32 line, cv::Point2f p ) {
	return (float)sqrt(distanceSq(line, p));
}

/**
* Finds the point of intersection between two lines and returns the point.
*
* @param a Line.
* @param b Line.
* @param ret storage for the point of intersection. If null a new point will be declared.
* @return If the two lines intersect it returns the point of intersection.  null if they don't intersect or have infinite intersections.
*/
cv::Point2i *intersection( LineParametric2D_F32 a, LineParametric2D_F32 b ) 
{
	float t_b = a.getSlopeX() * ( b.getY() - a.getY() ) - a.getSlopeY() * ( b.getX() - a.getX() );
	float bottom = a.getSlopeY() * b.getSlopeX() - b.getSlopeY() * a.getSlopeX();

	if( bottom == 0 )
		return NULL;

	t_b /= bottom;

	float x = b.getSlopeX() * t_b + b.getX();
	float y = b.getSlopeY() * t_b + b.getY();


	cv::Point2i *p = new cv::Point2i(x,y);
	return p;
}

double atanSafe( double y , double x ) {
	if( x == 0 )
		return PI/2;
	return atan(y/x);
}
/**
* Angular distance between two half circle angles.
*
* @param angA Angle between -pi/2 and pi/2.
* @param angB Angle between -pi/2 and pi/2.
* @return Acute angle between the two input angles.
*/
double distHalf( double angA , double angB ) {
	double a = abs(angA-angB);
	if( a <= PI/2 )
		return a;
	else
		return PI-a;
}
