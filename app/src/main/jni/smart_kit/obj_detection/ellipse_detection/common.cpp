#include "common.h"

#define CANNY_SHIFT 15
#define TG22  (int)(0.4142135623730950488016887242097*(1<<CANNY_SHIFT) + 0.5)
#define CANNY_PUSH(d)    *(d) = (uchar)2, *stack_top++ = (d)
#define CANNY_POP(d)     (d) = *--stack_top

typedef union my_32suf
{
	int i;
	unsigned u;
	float f;
}
my_32suf;

void Labeling( cv::Mat1b& image, std::vector<std::vector<cv::Point> >& segments, int iMinLength)
{
	#define RG_STACK_SIZE 2048

	// Using global stack for faster processing (even at the expense of the memory occupied)
	int stack2[RG_STACK_SIZE];
	#define RG_PUSH2(a) (stack2[sp2] = (a) , sp2++)
	#define RG_POP2(a) (sp2-- , (a) = stack2[sp2])

	// Using global stack for faster processing (even at the expense of the memory occupied)
	cv::Point stack3[RG_STACK_SIZE];
	#define RG_PUSH3(a) (stack3[sp3] = (a) , sp3++)
	#define RG_POP3(a) (sp3-- , (a) = stack3[sp3])

	int i,w,h, iDim;
	int x,y;
	int x2,y2;
	int sp2; // stack pointer
    int sp3;

	cv::Mat_<uchar> src = image.clone();
	w = src.cols;
	h = src.rows;
	iDim = w*h;

	cv::Point point;
	for (y=0; y<h; ++y)
	{
		for (x=0; x<w; ++x)
		{
			if ((src(y,x))!=0)   // point not labeled: seed found
			{
				// For each object
				sp2 = 0;
				i = x + y*w;
				RG_PUSH2(i);

				// Empty list of points
	    		sp3=0;
  		  		while (sp2>0)
				{// Rg traditional

					RG_POP2(i);
					x2=i%w;
					y2=i/w;



					point.x=x2;
					point.y=y2;

					if(src(y2,x2))
					{
						RG_PUSH3(point);
						src(y2,x2) = 0;
					}
					
					// Insert new points in the stack only if there are
					// And points to be labeled

					// 4 connected
					// sx
					if (x2>0 &&   (src(y2, x2-1)!=0))
						RG_PUSH2(i-1);
					// below
					if (y2>0 &&   (src(y2-1, x2)!=0))
						RG_PUSH2(i-w);
					// above
					if (y2<h-1 &&   (src(y2+1, x2)!=0))
						RG_PUSH2(i+w);
					// dx
					if (x2<w-1 &&   (src(y2, x2+1)!=0))
						RG_PUSH2(i+1);

					// 8 connected
					if (x2>0 && y2>0 &&   (src(y2-1,x2-1)!=0))
						RG_PUSH2(i-w-1);
					if (x2>0 && y2<h-1 &&   (src(y2+1, x2-1)!=0))
						RG_PUSH2(i+w-1);
					if (x2<w-1 && y2>0 &&   (src(y2-1, x2+1)!=0))
						RG_PUSH2(i-w+1);
					if (x2<w-1 && y2<h-1 &&   (src(y2+1, x2+1)!=0))
						RG_PUSH2(i+w+1);

				}

				if (sp3 >= iMinLength)
				{
					std::vector<cv::Point> component(stack3, stack3+sp3);
					segments.push_back(component);
				}
			}
		}
	}
};

void LabelingRect( cv::Mat1b& image, VVP& segments, int iMinLength, std::vector<cv::Rect>& bboxes)
{
	#define _RG_STACK_SIZE 10000

	// Using global stack for faster processing (even at the expense of the memory occupied)
	int stack2[_RG_STACK_SIZE];
	#define _RG_PUSH2(a) (stack2[sp2] = (a) , sp2++)
	#define _RG_POP2(a) (sp2-- , (a) = stack2[sp2])

	// Using global stack for faster processing (even at the expense of the memory occupied)
	cv::Point stack3[_RG_STACK_SIZE];
	#define _RG_PUSH3(a) (stack3[sp3] = (a) , sp3++)
	#define _RG_POP3(a) (sp3-- , (a) = stack3[sp3])

	int i,w,h, iDim;
	int x,y;
	int x2,y2;	
	int sp2; /* stack pointer */
    int sp3;

	cv::Mat_<uchar> src = image.clone();
	w = src.cols;
	h = src.rows;
	iDim = w*h;

	cv::Point point;
	for (y=0; y<h; y++)
	{
		for (x=0; x<w; x++)
		{
			if ((src(y,x))!=0)   // point not labeled: seed found
			{
				// For each object
				sp2 = 0;
				i = x + y*w;
				_RG_PUSH2(i);

				// Empty list of points
	    		sp3=0;
  		  		while (sp2>0) 
				{// rg traditional
		
					_RG_POP2(i);
					x2=i%w;
					y2=i/w;

					src(y2,x2) = 0;

					point.x=x2;
					point.y=y2;
					_RG_PUSH3(point);

					// Insert new points in the stack only if there are
					// And points to be labeled

					// 4 connected
					// sx
					if (x2>0 &&   (src(y2, x2-1)!=0))
						_RG_PUSH2(i-1);
					// below
					if (y2>0 &&   (src(y2-1, x2)!=0))
						_RG_PUSH2(i-w);
					// above
					if (y2<h-1 &&   (src(y2+1, x2)!=0))
						_RG_PUSH2(i+w);
					// dx
					if (x2<w-1 &&   (src(y2, x2+1)!=0))
						_RG_PUSH2(i+1);

					// 8 connected
					if (x2>0 && y2>0 &&   (src(y2-1,x2-1)!=0))
						_RG_PUSH2(i-w-1);
					if (x2>0 && y2<h-1 &&   (src(y2+1, x2-1)!=0))
						_RG_PUSH2(i+w-1);
					if (x2<w-1 && y2>0 &&   (src(y2-1, x2+1)!=0))
						_RG_PUSH2(i-w+1);
					if (x2<w-1 && y2<h-1 &&   (src(y2+1, x2+1)!=0))
						_RG_PUSH2(i+w+1);

				}

				if (sp3 >= iMinLength)
				{
					std::vector<cv::Point> component;

					int iMinx, iMaxx, iMiny,iMaxy;
					iMinx = iMaxx = stack3[0].x;
					iMiny = iMaxy = stack3[0].y;

					// labeled the point
					for (i=0; i<sp3; i++){
						point = stack3[i];
						// labeled
						component.push_back(point);

						if (iMinx > point.x)  iMinx = point.x;
						if (iMiny > point.y)  iMiny = point.y;
						if (iMaxx < point.x)  iMaxx = point.x;
						if (iMaxy < point.y)  iMaxy = point.y;
					}

					bboxes.push_back( cv::Rect(cv::Point(iMinx, iMiny), cv::Point(iMaxx+1, iMaxy+1)));
					segments.push_back(component);
				}
			}
		}	
	}
}

void Thinning( cv::Mat1b& imgMask, uchar byF, uchar byB) 
{
	int r = imgMask.rows;
	int c = imgMask.cols;

	cv::Mat_<uchar> imgIT(r,c),imgM(r,c);

	for(int i=0; i<r; ++i)
	{		
		for(int j=0; j<c; ++j)
		{
			imgIT(i,j) = imgMask(i,j)==byF?1:0;
		}
	}

	bool bSomethingDone = true;
	int iCount = 0;

	while (bSomethingDone) {
		bSomethingDone = false;
		std::fill(imgM.begin(), imgM.end(), 0);

		// first iteration
		for(int y=1;y<r-2;y++) {
			for(int x=1;x<c-2;x++) {

#define c_P0 imgIT(y-1,x-1)==1
#define c_P1 imgIT(y-1,x)==1
#define c_P2 imgIT(y-1,x+1)==1
#define c_P3 imgIT(y-1,x+2)==1
#define c_P4 imgIT(y,x-1)==1
#define c_P5 imgIT(y,x)==1
#define c_P6 imgIT(y,x+1)==1
#define c_P7 imgIT(y,x+2)==1
#define c_P8 imgIT(y+1,x-1)==1
#define c_P9 imgIT(y+1,x)==1
#define c_P10 imgIT(y+1,x+1)==1
#define c_P11 imgIT(y+1,x+2)==1
#define c_P12 imgIT(y+2,x-1)==1
#define c_P13 imgIT(y+2,x)==1
#define c_P14 imgIT(y+2,x+1)==1
#define c_P15 imgIT(y+2,x+2)==1

				if (c_P5) {
					if (c_P9) {
						if (c_P6) {
							if (c_P10) {
								if (c_P4) {
									if (c_P8) {
										if (c_P1) {
											continue;
										}
										else {
											if (c_P13) {
												if (c_P2) {
													if (c_P0) {
														continue;
													}
													else {
														goto a_2;
													}
												}
												else {
													goto a_2;
												}
											}
											else {
												if (c_P14) {
													if (c_P12) {
														if (c_P2) {
															if (c_P0) {
																continue;
															}
															else {
																goto a_2;
															}
														}
														else {
															goto a_2;
														}
													}
													else {
														continue;
													}
												}
												else {
													continue;
												}
											}
										}
									}
									else {
										continue;
									}
								}
								else {
									if (c_P1) {
										if (c_P2) {
											if (c_P7) {
												if (c_P8) {
													if (c_P0) {
														continue;
													}
													else {
														goto a_2;
													}
												}
												else {
													goto a_2;
												}
											}
											else {
												if (c_P11) {
													if (c_P3) {
														if (c_P8) {
															if (c_P0) {
																continue;
															}
															else {
																goto a_2;
															}
														}
														else {
															goto a_2;
														}
													}
													else {
														continue;
													}
												}
												else {
													continue;
												}
											}
										}
										else {
											continue;
										}
									}
									else {
										if (c_P0) {
											continue;
										}
										else {
											if (c_P8) {
												goto a_2;
											}
											else {
												if (c_P2) {
													goto a_2;
												}
												else {
													if (c_P14) {
														if (c_P13) {
															if (c_P11) {
																goto a_2;
															}
															else {
																if (c_P7) {
																	goto a_2;
																}
																else {
																	if (c_P3) {
																		goto a_2;
																	}
																	else {
																		continue;
																	}
																}
															}
														}
														else {
															goto a_2;
														}
													}
													else {
														if (c_P13) {
															goto a_2;
														}
														else {
															if (c_P12) {
																goto a_2;
															}
															else {
																if (c_P11) {
																	if (c_P7) {
																		continue;
																	}
																	else {
																		goto a_2;
																	}
																}
																else {
																	if (c_P15) {
																		goto a_2;
																	}
																	else {
																		if (c_P7) {
																			goto a_2;
																		}
																		else {
																			if (c_P3) {
																				goto a_2;
																			}
																			else {
																				continue;
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
							else {
								continue;
							}
						}
						else {
							if (c_P0) {
								if (c_P8) {
									if (c_P4) {
										if (c_P2) {
											if (c_P10) {
												continue;
											}
											else {
												if (c_P1) {
													goto a_2;
												}
												else {
													continue;
												}
											}
										}
										else {
											goto a_2;
										}
									}
									else {
										continue;
									}
								}
								else {
									continue;
								}
							}
							else {
								if (c_P2) {
									continue;
								}
								else {
									if (c_P1) {
										continue;
									}
									else {
										if (c_P8) {
											goto a_2;
										}
										else {
											if (c_P10) {
												if (c_P4) {
													continue;
												}
												else {
													goto a_2;
												}
											}
											else {
												continue;
											}
										}
									}
								}
							}
						}
					}
					else {
						if (c_P6) {
							if (c_P0) {
								if (c_P2) {
									if (c_P1) {
										if (c_P8) {
											if (c_P10) {
												continue;
											}
											else {
												if (c_P4) {
													goto a_2;
												}
												else {
													continue;
												}
											}
										}
										else {
											goto a_2;
										}
									}
									else {
										continue;
									}
								}
								else {
									continue;
								}
							}
							else {
								if (c_P8) {
									continue;
								}
								else {
									if (c_P4) {
										continue;
									}
									else {
										if (c_P2) {
											goto a_2;
										}
										else {
											if (c_P10) {
												if (c_P1) {
													continue;
												}
												else {
													goto a_2;
												}
											}
											else {
												continue;
											}
										}
									}
								}
							}
						}
						else {
							if (c_P10) {
								continue;
							}
							else {
								if (c_P4) {
									if (c_P1) {
										if (c_P0) {
											goto a_2;
										}
										else {
											continue;
										}
									}
									else {
										if (c_P2) {
											continue;
										}
										else {
											if (c_P8) {
												goto a_2;
											}
											else {
												if (c_P0) {
													goto a_2;
												}
												else {
													continue;
												}
											}
										}
									}
								}
								else {
									if (c_P8) {
										continue;
									}
									else {
										if (c_P1) {
											if (c_P2) {
												goto a_2;
											}
											else {
												if (c_P0) {
													goto a_2;
												}
												else {
													continue;
												}
											}
										}
										else {
											continue;
										}
									}
								}
							}
						}
					}
				}
				else {
					continue;
				}


a_2:
				imgM(y,x) = 1;
				bSomethingDone = true;
			}
		}
		
		for (int r=0; r<imgIT.rows; ++r) {
			for (int c=0; c<imgIT.cols; ++c) {
				if (imgM(r,c) == 1)
					imgIT(r,c) = 0;
			}
		}
	}

	for(int i=0; i<r; ++i)
	{		
		for(int j=0; j<c; ++j)
		{
			imgMask(i,j) = imgIT(i,j)==1 ? byF : byB;
		}
	}
};

bool SortBottomLeft2TopRight(const cv::Point& lhs, const cv::Point& rhs)
{
	if(lhs.x == rhs.x)
	{
		return lhs.y > rhs.y;
	}
	return lhs.x < rhs.x;
};

bool SortBottomLeft2TopRight2f(const cv::Point2f& lhs, const cv::Point2f& rhs)
{
	if(lhs.x == rhs.x)
	{
		return lhs.y > rhs.y;
	}
	return lhs.x < rhs.x;
};


bool SortTopLeft2BottomRight(const cv::Point& lhs, const cv::Point& rhs)
{
	if(lhs.x == rhs.x)
	{
		return lhs.y < rhs.y;
	}
	return lhs.x < rhs.x;
};

void ivcl_canny( cv::InputArray _src, cv::OutputArray _dst,
				 cv::OutputArray _sobel_x, cv::OutputArray _sobel_y,
                 double low_thresh, double high_thresh,
                 int aperture_size, bool L2gradient )
{
    cv::Mat src = _src.getMat();
    CV_Assert( src.depth() == CV_8U );

    _dst.create(src.size(), CV_8U);
    cv::Mat dst = _dst.getMat();

    if (!L2gradient && (aperture_size & CV_CANNY_L2_GRADIENT) == CV_CANNY_L2_GRADIENT)
    {
        // backward compatibility
        aperture_size &= ~CV_CANNY_L2_GRADIENT;
        L2gradient = true;
    }

    if ((aperture_size & 1) == 0 || (aperture_size != -1 && (aperture_size < 3 || aperture_size > 7)))
        CV_Error(CV_StsBadFlag, "");

    if ( low_thresh > high_thresh )
        std::swap( low_thresh, high_thresh );

    //const int cn = src.channels();
    cv::Mat dx( src.rows, src.cols, CV_16SC1 );
    cv::Mat dy( src.rows, src.cols, CV_16SC1 );

    cv::Sobel( src, dx, CV_16S, 1, 0, aperture_size, 1, 0, cv::BORDER_REPLICATE );
    cv::Sobel( src, dy, CV_16S, 0, 1, aperture_size, 1, 0, cv::BORDER_REPLICATE );

    if ( L2gradient )
    {
        low_thresh = std::min( 32767.0, low_thresh );
        high_thresh = std::min( 32767.0, high_thresh );

        if (low_thresh > 0) low_thresh *= low_thresh;
        if (high_thresh > 0) high_thresh *= high_thresh;
    }

    int low = cvFloor(low_thresh);
    int high = cvFloor(high_thresh);

    ptrdiff_t mapstep = src.cols + 2;
    cv::AutoBuffer<uchar> buffer((src.cols+2)*(src.rows+2) + mapstep * 3 * sizeof(int));

    int* mag_buf[3];
    mag_buf[0] = (int*)(uchar*)buffer;
    mag_buf[1] = mag_buf[0] + mapstep;
    mag_buf[2] = mag_buf[1] + mapstep;
	memset(mag_buf[0], 0, /* cn* */mapstep*sizeof(int));

    uchar* map = (uchar*)( mag_buf[2] + mapstep );
    memset(map, 1, mapstep);
    memset(map + mapstep*(src.rows + 1), 1, mapstep);
	
    int maxsize = std::max( 1 << 10, src.cols * src.rows / 10 );
    std::vector<uchar*> stack(maxsize);
    uchar **stack_top = &stack[0];
    uchar **stack_bottom = &stack[0];

    /* sector numbers
       (Top-Left Origin)

        1   2   3
         *  *  *
          * * *
        0*******0
          * * *
         *  *  *
        3   2   1
    */

    //#define CANNY_PUSH(d)    *(d) = uchar(2), *stack_top++ = (d)
    //#define CANNY_POP(d)     (d) = *--stack_top

    // calculate magnitude and angle of gradient, perform non-maxima supression.
    // fill the map with one of the following values:
    //   0 - the pixel might belong to an edge
    //   1 - the pixel can not belong to an edge
    //   2 - the pixel does belong to an edge
    for (int i = 0; i <= src.rows; i++)
    {
        int* _norm = mag_buf[(i > 0) + 1] + 1;
        if (i < src.rows)
        {
            short* _dx = dx.ptr<short>(i);
            short* _dy = dy.ptr<short>(i);

            if (!L2gradient)
            {
                for ( int j = 0; j < src.cols; j++ )
                    _norm[j] = std::abs(int(_dx[j])) + std::abs(int(_dy[j]));
            }
            else
            {
                for ( int j = 0; j < src.cols; j++ )
                    _norm[j] = int(_dx[j])*_dx[j] + int(_dy[j])*_dy[j];
            }

            //if (cn > 1)
            //{
            //    for(int j = 0, jn = 0; j < src.cols; ++j, jn += cn)
            //    {
            //        int maxIdx = jn;
            //        for(int k = 1; k < cn; ++k)
            //            if(_norm[jn + k] > _norm[maxIdx]) maxIdx = jn + k;
            //        _norm[j] = _norm[maxIdx];
            //        _dx[j] = _dx[maxIdx];
            //        _dy[j] = _dy[maxIdx];
            //    }
            //}
            _norm[-1] = _norm[src.cols] = 0;
        }
        else
            memset(_norm-1, 0, /* cn* */mapstep*sizeof(int));

        // at the very beginning we do not have a complete ring
        // buffer of 3 magnitude rows for non-maxima suppression
        if (i == 0)
            continue;

        uchar* _map = map + mapstep*i + 1;
        _map[-1] = _map[src.cols] = 1;

        int* _mag = mag_buf[1] + 1; // take the central row
        ptrdiff_t magstep1 = mag_buf[2] - mag_buf[1];
        ptrdiff_t magstep2 = mag_buf[0] - mag_buf[1];

        const short* _x = dx.ptr<short>(i-1);
        const short* _y = dy.ptr<short>(i-1);

        if ((stack_top - stack_bottom) + src.cols > maxsize)
        {
            int sz = (int)(stack_top - stack_bottom);
            maxsize = maxsize * 3/2;
            stack.resize(maxsize);
            stack_bottom = &stack[0];
            stack_top = stack_bottom + sz;
        }

        int prev_flag = 0;
        for (int j = 0; j < src.cols; j++)
        {
            //#define CANNY_SHIFT 15
            //const int TG22 = (int)(0.4142135623730950488016887242097*(1<<CANNY_SHIFT) + 0.5);

            int m = _mag[j];

            if (m > low)
            {
                int xs = _x[j];
                int ys = _y[j];
                int x = std::abs(xs);
                int y = std::abs(ys) << CANNY_SHIFT;

                int tg22x = x * TG22;

                if (y < tg22x)
                {
                    if (m > _mag[j-1] && m >= _mag[j+1]) goto __ocv_canny_push;
                }
                else
                {
                    int tg67x = tg22x + (x << (CANNY_SHIFT+1));
                    if (y > tg67x)
                    {
                        if (m > _mag[j+magstep2] && m >= _mag[j+magstep1]) goto __ocv_canny_push;
                    }
                    else
                    {
                        int s = (xs ^ ys) < 0 ? -1 : 1;
                        if (m > _mag[j+magstep2-s] && m > _mag[j+magstep1+s]) goto __ocv_canny_push;
                    }
                }
            }
            prev_flag = 0;
            _map[j] = uchar(1);
            continue;
__ocv_canny_push:
            if (!prev_flag && m > high && _map[j-mapstep] != 2)
            {
                CANNY_PUSH(_map + j);
                prev_flag = 1;
            }
            else
                _map[j] = 0;
        }

        // scroll the ring buffer
        _mag = mag_buf[0];
        mag_buf[0] = mag_buf[1];
        mag_buf[1] = mag_buf[2];
        mag_buf[2] = _mag;
    }

    // now track the edges (hysteresis thresholding)
    while (stack_top > stack_bottom)
    {
        uchar* m;
        if ((stack_top - stack_bottom) + 8 > maxsize)
        {
            int sz = (int)(stack_top - stack_bottom);
            maxsize = maxsize * 3/2;
            stack.resize(maxsize);
            stack_bottom = &stack[0];
            stack_top = stack_bottom + sz;
        }

        CANNY_POP(m);

        if (!m[-1])         CANNY_PUSH(m - 1);
        if (!m[1])          CANNY_PUSH(m + 1);
        if (!m[-mapstep-1]) CANNY_PUSH(m - mapstep - 1);
        if (!m[-mapstep])   CANNY_PUSH(m - mapstep);
        if (!m[-mapstep+1]) CANNY_PUSH(m - mapstep + 1);
        if (!m[mapstep-1])  CANNY_PUSH(m + mapstep - 1);
        if (!m[mapstep])    CANNY_PUSH(m + mapstep);
        if (!m[mapstep+1])  CANNY_PUSH(m + mapstep + 1);
    }

    // the final pass, form the final image
    const uchar* pmap = map + mapstep + 1;
    uchar* pdst = dst.ptr();
    for (int i = 0; i < src.rows; i++, pmap += mapstep, pdst += dst.step)
    {
        for (int j = 0; j < src.cols; j++)
            pdst[j] = (uchar)-(pmap[j] >> 1);
    }
}

void cvCanny3(cv::Mat &srcarr, cv::Mat &dstarr,
	cv::Mat &dxarr, cv::Mat &dyarr,
                int aperture_size )
{
    cv::AutoBuffer<char> buffer;
    std::vector<uchar*> stack;
    uchar **stack_top = 0, **stack_bottom = 0;

    cv::Mat src = srcarr;
    cv::Mat dst = dstarr;

	cv::Mat dx = dxarr;
	cv::Mat dy = dyarr;


    cv::Size size;
    int flags = aperture_size;
    int low, high;
    int* mag_buf[3];
    uchar* map;
    ptrdiff_t mapstep;
    int maxsize;
    int i, j;
    cv::Mat mag_row;

    if( CV_MAT_TYPE( src.type() ) != CV_8UC1 ||
		CV_MAT_TYPE(dst.type()) != CV_8UC1 ||
		CV_MAT_TYPE(dx.type()) != CV_16SC1 ||
		CV_MAT_TYPE(dy.type()) != CV_16SC1)
        CV_Error( CV_StsUnsupportedFormat, "" );

    if( !CV_ARE_SIZES_EQ( &src, &dst ))
        CV_Error( CV_StsUnmatchedSizes, "" );
	
    aperture_size &= INT_MAX;
    if( (aperture_size & 1) == 0 || aperture_size < 3 || aperture_size > 7 )
        CV_Error( CV_StsBadFlag, "" );


	size.width = src.cols;
    size.height = src.rows;

	cv::Sobel(src, dx, CV_16S, 1, 0, aperture_size, 1, 0, cv::BORDER_REFLECT);
	cv::Sobel(src, dy, CV_16S, 0, 1, aperture_size, 1, 0, cv::BORDER_REFLECT);

	// Calculate Magnitude of Gradient
	//cv::Mat1f magGrad( size.height, size.width, 0.f );
	//magGrad = cv::abs( dx ) + cv::abs( dy );
	//double maxGrad = 0, minGrad = 0;
	//cv::minMaxIdx( magGrad, &minGrad, &maxGrad );

	//% Calculate Magnitude of Gradient
	cv::Mat1f magGrad( size.height, size.width, 0.f );
	float maxGrad = 0;
	float val = 0;
	for( i = 0; i< size.height; ++i )
	{
		float* _pmag = magGrad.ptr<float>(i);
		const short* _dx = (short*)(dx.data + dx.step*i);
		const short* _dy = (short*)(dy.data + dy.step*i);
		for( j = 0; j < size.width; ++j )
		{
			val = float( abs(_dx[j]) + abs(_dy[j]) );
			_pmag[j] = val;
			maxGrad = (val > maxGrad) ? val : maxGrad;
		}
	}
	////////////////////////////////////////////////////////////////////////////
	
	//% Normalize for threshold selection
	//normalize(magGrad, magGrad, 0.0, 1.0, NORM_MINMAX);

	//% Determine Hysteresis Thresholds
	
	// set magic numbers
	const int NUM_BINS = 64;	
	const double percent_of_pixels_not_edges = 0.9;
	const double threshold_ratio = 0.3;

	// compute histogram
	int bin_size = std::floor( maxGrad / float(NUM_BINS) + 0.5f ) + 1;
	if ( bin_size < 1 ) 
		bin_size = 1;
	int bins[ NUM_BINS ] = { 0 }; 
	for ( i = 0; i < size.height; ++i ) 
	{
		float *_pmag = magGrad.ptr<float>(i);
		for( j = 0; j < size.width; ++j )
		{
			//int hgf = int( _pmag[ j ] );
			bins[ int( _pmag[ j ] ) / bin_size ]++;
		}
	}



	//% Select the thresholds
	float total(0.f);	
	float target = float(size.height * size.width * percent_of_pixels_not_edges);
	int low_thresh, high_thresh(0);
	
	while(total < target)
	{
		total+= bins[high_thresh];
		high_thresh++;
	}
	high_thresh *= bin_size;
	low_thresh = std::floor(threshold_ratio * float(high_thresh));
	
    if( flags & CV_CANNY_L2_GRADIENT )
    {
		my_32suf ul, uh;
        ul.f = (float)low_thresh;
        uh.f = (float)high_thresh;

        low = ul.i;
        high = uh.i;
    }
    else
    {
		low = std::floor( (float)low_thresh);
        high = std::floor( (float)high_thresh );
    }

    
	buffer.allocate( ( size.width + 2 ) * ( size.height + 2 ) + ( size.width + 2 ) * 3 * sizeof(int) );
    mag_buf[0] = (int*)(char*)buffer;
    mag_buf[1] = mag_buf[0] + size.width + 2;
    mag_buf[2] = mag_buf[1] + size.width + 2;
    map = (uchar*)(mag_buf[2] + size.width + 2);
    mapstep = size.width + 2;

    maxsize = MAX( 1 << 10, size.width*size.height/10 );
    stack.resize( maxsize );
    stack_top = stack_bottom = &stack[0];

    memset( mag_buf[0], 0, (size.width+2)*sizeof(int) );
    memset( map, 1, mapstep );
    memset( map + mapstep*(size.height + 1), 1, mapstep );

    /* sector numbers
       (Top-Left Origin)

        1   2   3
         *  *  *
          * * *
        0*******0
          * * *
         *  *  *
        3   2   1
    */

    #define CANNY_PUSH(d)    *(d) = (uchar)2, *stack_top++ = (d)
    #define CANNY_POP(d)     (d) = *--stack_top

    mag_row = cv::Mat( 1, size.width, CV_32F );

    // calculate magnitude and angle of gradient, perform non-maximal suppression.
    // fill the map with one of the following values:
    //   0 - the pixel might belong to an edge
    //   1 - the pixel can not belong to an edge
    //   2 - the pixel does belong to an edge
    for( i = 0; i <= size.height; i++ )
    {
        int* _mag = mag_buf[(i > 0) + 1] + 1;
        float* _magf = (float*)_mag;
        const short* _dx = (short*)(dx.data + dx.step*i);
        const short* _dy = (short*)(dy.data + dy.step*i);
        uchar* _map;
        int x, y;
        ptrdiff_t magstep1, magstep2;
        int prev_flag = 0;

        if( i < size.height )
        {
            _mag[-1] = _mag[size.width] = 0;

            if( !(flags & CV_CANNY_L2_GRADIENT) )
                for( j = 0; j < size.width; j++ )
                    _mag[j] = abs(_dx[j]) + abs(_dy[j]);

            else
            {
                for( j = 0; j < size.width; j++ )
                {
                    x = _dx[j]; y = _dy[j];
                    _magf[j] = (float)std::sqrt((double)x*x + (double)y*y);
                }
            }
        }
        else
            memset( _mag-1, 0, (size.width + 2)*sizeof(int) );

        // at the very beginning we do not have a complete ring
        // buffer of 3 magnitude rows for non-maximal suppression
        if( i == 0 )
            continue;

        _map = map + mapstep*i + 1;
        _map[-1] = _map[size.width] = 1;

        _mag = mag_buf[1] + 1; // take the central row
        _dx = (short*)(dx.data + dx.step*(i-1));
        _dy = (short*)(dy.data + dy.step*(i-1));

        magstep1 = mag_buf[2] - mag_buf[1];
        magstep2 = mag_buf[0] - mag_buf[1];

        if( (stack_top - stack_bottom) + size.width > maxsize )
        {
            int sz = (int)(stack_top - stack_bottom);
            maxsize = MAX( maxsize * 3/2, maxsize + 8 );
            stack.resize(maxsize);
            stack_bottom = &stack[0];
            stack_top = stack_bottom + sz;
        }

        for( j = 0; j < size.width; j++ )
        {
            #define CANNY_SHIFT 15
            #define TG22  (int)(0.4142135623730950488016887242097*(1<<CANNY_SHIFT) + 0.5)

            x = _dx[j];
            y = _dy[j];
            int s = x ^ y;
            int m = _mag[j];

            x = abs(x);
            y = abs(y);
            if( m > low )
            {
                int tg22x = x * TG22;
                int tg67x = tg22x + ((x + x) << CANNY_SHIFT);

                y <<= CANNY_SHIFT;

                if( y < tg22x )
                {
                    if( m > _mag[j-1] && m >= _mag[j+1] )
                    {
                        if( m > high && !prev_flag && _map[j-mapstep] != 2 )
                        {
                            CANNY_PUSH( _map + j );
                            prev_flag = 1;
                        }
                        else
                            _map[j] = (uchar)0;
                        continue;
                    }
                }
                else if( y > tg67x )
                {
                    if( m > _mag[j+magstep2] && m >= _mag[j+magstep1] )
                    {
                        if( m > high && !prev_flag && _map[j-mapstep] != 2 )
                        {
                            CANNY_PUSH( _map + j );
                            prev_flag = 1;
                        }
                        else
                            _map[j] = (uchar)0;
                        continue;
                    }
                }
                else
                {
                    s = s < 0 ? -1 : 1;
                    if( m > _mag[j+magstep2-s] && m > _mag[j+magstep1+s] )
                    {
                        if( m > high && !prev_flag && _map[j-mapstep] != 2 )
                        {
                            CANNY_PUSH( _map + j );
                            prev_flag = 1;
                        }
                        else
                            _map[j] = (uchar)0;
                        continue;
                    }
                }
            }
            prev_flag = 0;
            _map[j] = (uchar)1;
        }

        // scroll the ring buffer
        _mag = mag_buf[0];
        mag_buf[0] = mag_buf[1];
        mag_buf[1] = mag_buf[2];
        mag_buf[2] = _mag;
    }

    // now track the edges (hysteresis thresholding)
    while( stack_top > stack_bottom )
    {
        uchar* m;
        if( (stack_top - stack_bottom) + 8 > maxsize )
        {
            int sz = (int)(stack_top - stack_bottom);
            maxsize = MAX( maxsize * 3/2, maxsize + 8 );
            stack.resize(maxsize);
            stack_bottom = &stack[0];
            stack_top = stack_bottom + sz;
        }

        CANNY_POP(m);

        if( !m[-1] )
            CANNY_PUSH( m - 1 );
        if( !m[1] )
            CANNY_PUSH( m + 1 );
        if( !m[-mapstep-1] )
            CANNY_PUSH( m - mapstep - 1 );
        if( !m[-mapstep] )
            CANNY_PUSH( m - mapstep );
        if( !m[-mapstep+1] )
            CANNY_PUSH( m - mapstep + 1 );
        if( !m[mapstep-1] )
            CANNY_PUSH( m + mapstep - 1 );
        if( !m[mapstep] )
            CANNY_PUSH( m + mapstep );
        if( !m[mapstep+1] )
            CANNY_PUSH( m + mapstep + 1 );
    }

    // the final pass, form the final image
    for( i = 0; i < size.height; i++ )
    {
        const uchar* _map = map + mapstep*(i+1) + 1;
        uchar* _dst = dst.data + dst.step*i;

        for( j = 0; j < size.width; j++ )
		{
            _dst[j] = (uchar)-(_map[j] >> 1);
		}
	}
};

void Canny3( cv::InputArray image, cv::OutputArray _edges,
			 cv::OutputArray _sobel_x, cv::OutputArray _sobel_y,
             int apertureSize, bool L2gradient )
{
    cv::Mat src = image.getMat();
    _edges.create(src.size(), CV_8U);
	_sobel_x.create(src.size(), CV_16S);
	_sobel_y.create(src.size(), CV_16S);


    cv::Mat c_src = src, c_dst = _edges.getMat();
	cv::Mat c_dx = _sobel_x.getMat();
	cv::Mat c_dy = _sobel_y.getMat();


    cvCanny3( c_src, c_dst, 
			  c_dx, c_dy,
			  apertureSize + ( L2gradient ? CV_CANNY_L2_GRADIENT : 0 ) );
};

void ivcl_canny3( cv::InputArray image, cv::OutputArray _edges,
				  cv::OutputArray _sobel_x, cv::OutputArray _sobel_y,
				  int apertureSize, bool L2gradient )
{
	cv::Mat src = image.getMat();
	_edges.create(src.size(), CV_8U);
	_sobel_x.create(src.size(), CV_16S);
	_sobel_y.create(src.size(), CV_16S);

	cv::Mat c_src = src, c_dst = _edges.getMat();
	cv::Mat c_dx = _sobel_x.getMat();
	cv::Mat c_dy = _sobel_y.getMat();

	cvCanny3( c_src, c_dst, 
			  c_dx, c_dy,
			  apertureSize + ( L2gradient ? CV_CANNY_L2_GRADIENT : 0 ) );
};



