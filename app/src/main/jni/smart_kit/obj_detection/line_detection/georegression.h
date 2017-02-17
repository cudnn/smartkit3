#ifndef GEOREGRESSION_HH
#define GEOREGRESSION_HH
#include "line_parameters_2d_f32.h"

/**
* <p>
* Returns the Euclidean distance of the closest point on the line from a point.
* </p>
*
* @param line A line segment. Not modified.
* @param p The point. Not modified.
* @return Distance the closest point on the line is away from the point.
*/
extern float distance( LineParametric2D_F32 line, cv::Point2f p ) ;
extern cv::Point2i *intersection( LineParametric2D_F32 a, LineParametric2D_F32 b );
extern double atanSafe( double y , double x );
extern double distHalf( double angA , double angB );

#endif

