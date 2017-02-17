#ifndef _MATH_UTILS_HH
#define _MATH_UTILS_HH

inline int					isgn(int val) { return (0 < val) - (val < 0); }
inline int					isgn(double val, double tau = 1e-6) { return (abs(tau) < val) - (val<-abs(tau)); }

#endif

