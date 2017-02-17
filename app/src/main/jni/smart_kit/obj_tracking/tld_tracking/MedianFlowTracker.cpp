#include "MedianFlowTracker.h"
//#include <cpu-features.h>
//#include <arm_neon.h>
#include <time.h>
#define MFT_ZERO           (0.000001)
typedef short deriv_type;
#ifndef CV_DESCALE
#define CV_DESCALE(x, n) (((x) + (1 << ((n)-1))) >> (n))
#endif
//#define CV_NEON 1
#ifdef __GNUC__
#  define CV_DECL_ALIGNED(x) __attribute__ ((aligned (x)))
#elif defined _MSC_VER
#  define CV_DECL_ALIGNED(x) __declspec(align(x))
#else
#  define CV_DECL_ALIGNED(x)
#endif

static void calcSharrDerivNeon(const cv::Mat& src, cv::Mat& dst)
{
	using namespace cv;
	//using cv::detail::deriv_type;
	int rows = src.rows, cols = src.cols, cn = src.channels(), colsn = cols*cn, depth = src.depth();
	CV_Assert(depth == CV_8U);
	dst.create(rows, cols, CV_MAKETYPE(DataType<deriv_type>::depth, cn * 2));



	int x, y, delta = (int)alignSize((cols + 2)*cn, 16);
	AutoBuffer<deriv_type> _tempBuf(delta * 2 + 64);
	deriv_type *trow0 = alignPtr(_tempBuf + cn, 16), *trow1 = alignPtr(trow0 + delta, 16);



	for (y = 0; y < rows; y++)
	{
		const uchar* srow0 = src.ptr<uchar>(y > 0 ? y - 1 : rows > 1 ? 1 : 0);
		const uchar* srow1 = src.ptr<uchar>(y);
		const uchar* srow2 = src.ptr<uchar>(y < rows - 1 ? y + 1 : rows > 1 ? rows - 2 : 0);
		deriv_type* drow = dst.ptr<deriv_type>(y);

		// do vertical convolution
		x = 0;


//#if CV_NEON

//		const uint16x8_t q8 = vdupq_n_u16(3);
//		const uint8x8_t d18 = vdup_n_u8(10);

//		const int16x8_t q8i = vdupq_n_s16(3);
//		const int16x8_t q9 = vdupq_n_s16(10);

//		for (; x <= colsn - 8; x += 8)
//		{
//			uint8x8_t d0 = vld1_u8((const uint8_t*)&srow0[x]);
//			uint8x8_t d1 = vld1_u8((const uint8_t*)&srow1[x]);
//			uint8x8_t d2 = vld1_u8((const uint8_t*)&srow2[x]);
//			uint16x8_t q4 = vaddl_u8(d0, d2);
//			uint16x8_t q11 = vsubl_u8(d2, d0);
//			uint16x8_t q5 = vmulq_u16(q4, q8);
//			uint16x8_t q6 = vmull_u8(d1, d18);
//			uint16x8_t q10 = vaddq_u16(q6, q5);
//			vst1q_u16((uint16_t*)&trow0[x], q10);
//			vst1q_u16((uint16_t*)&trow1[x], q11);

//		}
//#else
		for (; x < colsn; x++)
		{
			int t0 = (srow0[x] + srow2[x]) * 3 + srow1[x] * 10;
			int t1 = srow2[x] - srow0[x];
			trow0[x] = (deriv_type)t0;
			trow1[x] = (deriv_type)t1;
		}

		// make border
		int x0 = (cols > 1 ? 1 : 0)*cn, x1 = (cols > 1 ? cols - 2 : 0)*cn;
		for (int k = 0; k < cn; k++)
		{
			trow0[-cn + k] = trow0[x0 + k]; trow0[colsn + k] = trow0[x1 + k];
			trow1[-cn + k] = trow1[x0 + k]; trow1[colsn + k] = trow1[x1 + k];
		}
//#endif
		// do horizontal convolution, interleave the results and store them to dst
		x = 0;


//#if CV_NEON
//		for (; x <= colsn - 8; x += 8)
//		{

//			int16x8_t q0 = vld1q_s16((const int16_t*)&trow0[x + cn]);
//			int16x8_t q1 = vld1q_s16((const int16_t*)&trow0[x - cn]);
//			int16x8_t q2 = vld1q_s16((const int16_t*)&trow1[x + cn]);
//			int16x8_t q3 = vld1q_s16((const int16_t*)&trow1[x - cn]);
//			int16x8_t q5 = vsubq_s16(q0, q1);
//			int16x8_t q6 = vaddq_s16(q2, q3);
//			int16x8_t q4 = vld1q_s16((const int16_t*)&trow1[x]);
//			int16x8_t q7 = vmulq_s16(q6, q8i);
//			int16x8_t q10 = vmulq_s16(q4, q9);
//			int16x8_t q11 = vaddq_s16(q7, q10);
//			int16x4_t d22 = vget_low_s16(q11);
//			int16x4_t d23 = vget_high_s16(q11);
//			int16x4_t d11 = vget_high_s16(q5);
//			int16x4_t d10 = vget_low_s16(q5);
//			int16x4x2_t q5x2, q11x2;
//			q5x2.val[0] = d10; q5x2.val[1] = d22;
//			q11x2.val[0] = d11; q11x2.val[1] = d23;
//			vst2_s16((int16_t*)&drow[x * 2], q5x2);
//			vst2_s16((int16_t*)&drow[(x * 2) + 8], q11x2);

//		}

//#else

		for (; x < colsn; x++)
		{
			deriv_type t0 = (deriv_type)(trow0[x + cn] - trow0[x - cn]);
			deriv_type t1 = (deriv_type)((trow1[x + cn] + trow1[x - cn]) * 3 + trow1[x] * 10);
			drow[x * 2] = t0; drow[x * 2 + 1] = t1;
		}
//#endif
	}
}


struct LKTrackerInvoker1 : cv::ParallelLoopBody
{
	LKTrackerInvoker1(const cv::Mat& _prevImg, const cv::Mat& _prevDeriv, const cv::Mat& _nextImg,
	const cv::Point2f* _prevPts, cv::Point2f* _nextPts,
	uchar* _status, float* _err,
	cv::Size _winSize, cv::TermCriteria _criteria,
	int _level, int _maxLevel, int _flags, float _minEigThreshold);

	void operator()(const cv::Range& range) const;

	const cv::Mat* prevImg;
	const cv::Mat* nextImg;
	const cv::Mat* prevDeriv;
	const cv::Point2f* prevPts;
	cv::Point2f* nextPts;
	uchar* status;
	float* err;
	cv::Size winSize;
	cv::TermCriteria criteria;
	int level;
	int maxLevel;
	int flags;
	float minEigThreshold;
};

LKTrackerInvoker1::LKTrackerInvoker1(
	const cv::Mat& _prevImg, const cv::Mat& _prevDeriv, const cv::Mat& _nextImg,
	const cv::Point2f* _prevPts, cv::Point2f* _nextPts,
	uchar* _status, float* _err,
	cv::Size _winSize, cv::TermCriteria _criteria,
	int _level, int _maxLevel, int _flags, float _minEigThreshold)
{
	prevImg = &_prevImg;
	prevDeriv = &_prevDeriv;
	nextImg = &_nextImg;
	prevPts = _prevPts;
	nextPts = _nextPts;
	status = _status;
	err = _err;
	winSize = _winSize;
	criteria = _criteria;
	level = _level;
	maxLevel = _maxLevel;
	flags = _flags;
	minEigThreshold = _minEigThreshold;
}
void LKTrackerInvoker1::operator()(const cv::Range& range) const
{
	cv::Point2f halfWin((winSize.width - 1)*0.5f, (winSize.height - 1)*0.5f);
	const cv::Mat& I = *prevImg;
	const cv::Mat& J = *nextImg;
	const cv::Mat& derivI = *prevDeriv;
    LOGD("tldt : operator");
	int j, cn = I.channels(), cn2 = cn * 2;
	cv::AutoBuffer<deriv_type> _buf(winSize.area()*(cn + cn2));
	int derivDepth = cv::DataType<deriv_type>::depth;

	cv::Mat IWinBuf(winSize, CV_MAKETYPE(derivDepth, cn), (deriv_type*)_buf);
	cv::Mat derivIWinBuf(winSize, CV_MAKETYPE(derivDepth, cn2), (deriv_type*)_buf + winSize.area()*cn);

//#ifdef CV_NEONN0
//    int* nB1 = (int*)malloc(4*sizeof(int));
//    int* nB2 = (int*)malloc(4*sizeof(int));
//    memset(nB1,0,4*sizeof(int));
//    memset(nB2,0,4*sizeof(int));
//#endif
	for (int ptidx = range.start; ptidx < range.end; ptidx++)
	{
		cv::Point2f prevPt = prevPts[ptidx] * (float)(1. / (1 << level));
		cv::Point2f nextPt;
		if (level == maxLevel)
		{
			if (flags & cv::OPTFLOW_USE_INITIAL_FLOW)
				nextPt = nextPts[ptidx] * (float)(1. / (1 << level));
			else
				nextPt = prevPt;
		}
		else
			nextPt = nextPts[ptidx] * 2.f;
		nextPts[ptidx] = nextPt;

		cv::Point2i iprevPt, inextPt;
		prevPt -= halfWin;
		iprevPt.x = cvFloor(prevPt.x);
		iprevPt.y = cvFloor(prevPt.y);

		if (iprevPt.x < -winSize.width || iprevPt.x >= derivI.cols ||
			iprevPt.y < -winSize.height || iprevPt.y >= derivI.rows)
		{
			if (level == 0)
			{
				if (status)
					status[ptidx] = false;
				if (err)
					err[ptidx] = 0;
			}
			continue;
		}

		float a = prevPt.x - iprevPt.x;
		float b = prevPt.y - iprevPt.y;
		const int W_BITS = 14, W_BITS1 = 14;
		const float FLT_SCALE = 1.f / (1 << 20);
		int iw00 = cvRound((1.f - a)*(1.f - b)*(1 << W_BITS));
		int iw01 = cvRound(a*(1.f - b)*(1 << W_BITS));
		int iw10 = cvRound((1.f - a)*b*(1 << W_BITS));
		int iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;

		int dstep = (int)(derivI.step / derivI.elemSize1());
		int stepI = (int)(I.step / I.elemSize1());
		int stepJ = (int)(J.step / J.elemSize1());
		float A11 = 0, A12 = 0, A22 = 0;


#if CV_NEON
//CV_DECL_ALIGNED(16)
		int32_t  nA11[] = { 0, 0, 0, 0 }, nA12[] = { 0, 0, 0, 0 },nA22[] = { 0, 0, 0, 0 };
		const int shifter1 = -(W_BITS - 5); //negative so it shifts right
		const int shifter2 = -(W_BITS);

		const int16x4_t d26 = vdup_n_s16((int16_t)iw00);
		const int16x4_t d27 = vdup_n_s16((int16_t)iw01);
		const int16x4_t d28 = vdup_n_s16((int16_t)iw10);
		const int16x4_t d29 = vdup_n_s16((int16_t)iw11);
		const int32x4_t q11 = vdupq_n_s32((int32_t)shifter1);
		const int32x4_t q12 = vdupq_n_s32((int32_t)shifter2);

#endif

		// extract the patch from the first image, compute covariation matrix of derivatives
		int x, y;
		for (y = 0; y < winSize.height; y++)
		{
			const uchar* src = (const uchar*)I.data + (y + iprevPt.y)*stepI + iprevPt.x*cn;
			const deriv_type* dsrc = (const deriv_type*)derivI.data + (y + iprevPt.y)*dstep + iprevPt.x*cn2;

			deriv_type* Iptr = (deriv_type*)(IWinBuf.data + y*IWinBuf.step);
			deriv_type* dIptr = (deriv_type*)(derivIWinBuf.data + y*derivIWinBuf.step);

			x = 0;

#if CV_NEON

			for (; x <= winSize.width*cn - 4; x += 4, dsrc += 4 * 2, dIptr += 4 * 2)
			{

				uint8x8_t d0 = vld1_u8(&src[x]);
				uint8x8_t d2 = vld1_u8(&src[x + cn]);
				uint16x8_t q0 = vmovl_u8(d0);
				uint16x8_t q1 = vmovl_u8(d2);

				int32x4_t q5 = vmull_s16(vget_low_s16(vreinterpretq_s16_u16(q0)), d26);
				int32x4_t q6 = vmull_s16(vget_low_s16(vreinterpretq_s16_u16(q1)), d27);

				uint8x8_t d4 = vld1_u8(&src[x + stepI]);
				uint8x8_t d6 = vld1_u8(&src[x + stepI + cn]);
				uint16x8_t q2 = vmovl_u8(d4);
				uint16x8_t q3 = vmovl_u8(d6);

				int32x4_t q7 = vmull_s16(vget_low_s16(vreinterpretq_s16_u16(q2)), d28);
				int32x4_t q8 = vmull_s16(vget_low_s16(vreinterpretq_s16_u16(q3)), d29);

				q5 = vaddq_s32(q5, q6);
				q7 = vaddq_s32(q7, q8);
				q5 = vaddq_s32(q5, q7);

				int16x4x2_t d0d1 = vld2_s16(dsrc);
				int16x4x2_t d2d3 = vld2_s16(&dsrc[cn2]);

				q5 = vqrshlq_s32(q5, q11);

				int32x4_t q4 = vmull_s16(d0d1.val[0], d26);
				q6 = vmull_s16(d0d1.val[1], d26);

				int16x4_t nd0 = vmovn_s32(q5);

				q7 = vmull_s16(d2d3.val[0], d27);
				q8 = vmull_s16(d2d3.val[1], d27);

				vst1_s16(&Iptr[x], nd0);

				int16x4x2_t d4d5 = vld2_s16(&dsrc[dstep]);
				int16x4x2_t d6d7 = vld2_s16(&dsrc[dstep + cn2]);

				q4 = vaddq_s32(q4, q7);
				q6 = vaddq_s32(q6, q8);

				q7 = vmull_s16(d4d5.val[0], d28);
				int32x4_t nq0 = vmull_s16(d4d5.val[1], d28);
				q8 = vmull_s16(d6d7.val[0], d29);
				int32x4_t q15 = vmull_s16(d6d7.val[1], d29);

				q7 = vaddq_s32(q7, q8);
				nq0 = vaddq_s32(nq0, q15);

				q4 = vaddq_s32(q4, q7);
				q6 = vaddq_s32(q6, nq0);

				int32x4_t nq1 = vld1q_s32(nA12);
				int32x4_t nq2 = vld1q_s32(nA22);
				nq0 = vld1q_s32(nA11);

				q4 = vqrshlq_s32(q4, q12);
				q6 = vqrshlq_s32(q6, q12);

				q7 = vmulq_s32(q4, q4);
				q8 = vmulq_s32(q4, q6);
				q15 = vmulq_s32(q6, q6);

				nq0 = vaddq_s32(nq0, q7);
				nq1 = vaddq_s32(nq1, q8);
				nq2 = vaddq_s32(nq2, q15);

				vst1q_s32(nA11, nq0);
				vst1q_s32(nA12, nq1);
				vst1q_s32(nA22, nq2);

				int16x4_t d8 = vmovn_s32(q4);
				int16x4_t d12 = vmovn_s32(q6);

				int16x4x2_t d8d12;
				d8d12.val[0] = d8; d8d12.val[1] = d12;
				vst2_s16(dIptr, d8d12);
			}

#else

			for (; x < winSize.width*cn; x++, dsrc += 2, dIptr += 2)
			{
				int ival = CV_DESCALE(src[x] * iw00 + src[x + cn] * iw01 +
					src[x + stepI] * iw10 + src[x + stepI + cn] * iw11, W_BITS1 - 5);
				int ixval = CV_DESCALE(dsrc[0] * iw00 + dsrc[cn2] * iw01 +
					dsrc[dstep] * iw10 + dsrc[dstep + cn2] * iw11, W_BITS1);
				int iyval = CV_DESCALE(dsrc[1] * iw00 + dsrc[cn2 + 1] * iw01 + dsrc[dstep + 1] * iw10 +
					dsrc[dstep + cn2 + 1] * iw11, W_BITS1);

				Iptr[x] = (short)ival;
				dIptr[0] = (short)ixval;
				dIptr[1] = (short)iyval;

				A11 += (float)(ixval*ixval);
				A12 += (float)(ixval*iyval);
				A22 += (float)(iyval*iyval);
			}
#endif
		}


#if CV_NEON
		A11 += (float)(nA11[0] + nA11[1] + nA11[2] + nA11[3]);
		A12 += (float)(nA12[0] + nA12[1] + nA12[2] + nA12[3]);
		A22 += (float)(nA22[0] + nA22[1] + nA22[2] + nA22[3]);
#endif

		A11 *= FLT_SCALE;
		A12 *= FLT_SCALE;
		A22 *= FLT_SCALE;

		float D = A11*A22 - A12*A12;
		float minEig = (A22 + A11 - std::sqrt((A11 - A22)*(A11 - A22) +
			4.f*A12*A12)) / (2 * winSize.width*winSize.height);

		if (err && (flags & CV_LKFLOW_GET_MIN_EIGENVALS) != 0)
			err[ptidx] = (float)minEig;

		if (minEig < minEigThreshold || D < FLT_EPSILON)
		{
			if (level == 0 && status)
				status[ptidx] = false;
			continue;
		}

		D = 1.f / D;

		nextPt -= halfWin;
		cv::Point2f prevDelta;

		for (j = 0; j < criteria.maxCount; j++)
		{
			inextPt.x = cvFloor(nextPt.x);
			inextPt.y = cvFloor(nextPt.y);

			if (inextPt.x < -winSize.width || inextPt.x >= J.cols ||
				inextPt.y < -winSize.height || inextPt.y >= J.rows)
			{
				if (level == 0 && status)
					status[ptidx] = false;
				break;
			}

			a = nextPt.x - inextPt.x;
			b = nextPt.y - inextPt.y;
			iw00 = cvRound((1.f - a)*(1.f - b)*(1 << W_BITS));
			iw01 = cvRound(a*(1.f - b)*(1 << W_BITS));
			iw10 = cvRound((1.f - a)*b*(1 << W_BITS));
			iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;
			float b1 = 0, b2 = 0;

//#if CV_NEONN0
////CV_DECL_ALIGNED(16)
//			//int CV_DECL_ALIGNED(16)  nB1[] = { 0, 0, 0, 0 }, nB2[] = { 0, 0, 0, 0 };
//          memset(nB1,0,4*sizeof(int));
//            memset(nB2,0,4*sizeof(int));-=
//           //int32x4_t nB1 = vdupq_n_s32(0);
//            //int32x4_t nB2 = vdupq_n_s32(0);
//			const int16x4_t d26_2 = vdup_n_s16((int16_t)iw00);
//			const int16x4_t d27_2 = vdup_n_s16((int16_t)iw01);
//			const int16x4_t d28_2 = vdup_n_s16((int16_t)iw10);
//			const int16x4_t d29_2 = vdup_n_s16((int16_t)iw11);

//endif

			for (y = 0; y < winSize.height; y++)
			{
				const uchar* Jptr = (const uchar*)J.data + (y + inextPt.y)*stepJ + inextPt.x*cn;
				const deriv_type* Iptr = (const deriv_type*)(IWinBuf.data + y*IWinBuf.step);
				const deriv_type* dIptr = (const deriv_type*)(derivIWinBuf.data + y*derivIWinBuf.step);

				x = 0;
//#if CV_NEONN0  //TODO

//				for (; x <= winSize.width*cn - 8; x += 8, dIptr += 8 * 2)
//				{

//					uint8x8_t d0 = vld1_u8(&Jptr[x]);
//					uint8x8_t d2 = vld1_u8(&Jptr[x + cn]);
//					uint8x8_t d4 = vld1_u8(&Jptr[x + stepJ]);
//					uint8x8_t d6 = vld1_u8(&Jptr[x + stepJ + cn]);

//					uint16x8_t q0 = vmovl_u8(d0);
//					uint16x8_t q1 = vmovl_u8(d2);
//					uint16x8_t q2 = vmovl_u8(d4);
//					uint16x8_t q3 = vmovl_u8(d6);

//					int32x4_t nq4 = vmull_s16(vget_low_s16(vreinterpretq_s16_u16(q0)), d26_2);
//					int32x4_t nq5 = vmull_s16(vget_high_s16(vreinterpretq_s16_u16(q0)), d26_2);

//					int32x4_t nq6 = vmull_s16(vget_low_s16(vreinterpretq_s16_u16(q1)), d27_2);
//					int32x4_t nq7 = vmull_s16(vget_high_s16(vreinterpretq_s16_u16(q1)), d27_2);

//					int32x4_t nq8 = vmull_s16(vget_low_s16(vreinterpretq_s16_u16(q2)), d28_2);
//					int32x4_t nq9 = vmull_s16(vget_high_s16(vreinterpretq_s16_u16(q2)), d28_2);

//					int32x4_t nq10 = vmull_s16(vget_low_s16(vreinterpretq_s16_u16(q3)), d29_2);
//					int32x4_t nq11 = vmull_s16(vget_high_s16(vreinterpretq_s16_u16(q3)), d29_2);

//					nq4 = vaddq_s32(nq4, nq6);
//					nq5 = vaddq_s32(nq5, nq7);
//					nq8 = vaddq_s32(nq8, nq10);
//					nq9 = vaddq_s32(nq9, nq11);

//					int16x8_t q6 = vld1q_s16(&Iptr[x]);

//					nq4 = vaddq_s32(nq4, nq8);
//					nq5 = vaddq_s32(nq5, nq9);

//					nq8 = vmovl_s16(vget_high_s16(q6));
//					nq6 = vmovl_s16(vget_low_s16(q6));

//					nq4 = vqrshlq_s32(nq4, q11);
//					nq5 = vqrshlq_s32(nq5, q11);

//					int16x8x2_t q0q1 = vld2q_s16(dIptr);
//					nq11 = vld1q_s32(nB1);
//					int32x4_t nq15 = vld1q_s32(nB2);

//					nq4 = vsubq_s32(nq4, nq6);
//					nq5 = vsubq_s32(nq5, nq8);

//					int32x4_t nq2 = vmovl_s16(vget_low_s16(q0q1.val[0]));
//					int32x4_t nq3 = vmovl_s16(vget_high_s16(q0q1.val[0]));

//					nq7 = vmovl_s16(vget_low_s16(q0q1.val[1]));
//					nq8 = vmovl_s16(vget_high_s16(q0q1.val[1]));

//					nq9 = vmulq_s32(nq4, nq2);
//					nq10 = vmulq_s32(nq5, nq3);

//					nq4 = vmulq_s32(nq4, nq7);
//					nq5 = vmulq_s32(nq5, nq8);

//					nq9 = vaddq_s32(nq9, nq10);
//					nq4 = vaddq_s32(nq4, nq5);

//					nq11 = vaddq_s32(nq11, nq9);
//					nq15 = vaddq_s32(nq15, nq4);

//					vst1q_s32(nB1, nq11);
//					vst1q_s32(nB2, nq15);
//				}

//#else

				for (; x < winSize.width*cn; x++, dIptr += 2)
				{
					int diff = CV_DESCALE(Jptr[x] * iw00 + Jptr[x + cn] * iw01 +
						Jptr[x + stepJ] * iw10 + Jptr[x + stepJ + cn] * iw11,
						W_BITS1 - 5) - Iptr[x];
					b1 += (float)(diff*dIptr[0]);
					b2 += (float)(diff*dIptr[1]);
				}
//#endif
			}

//#if CV_NEONN0   //TODO

//			b1 += (float)(nB1[0] + nB1[1] + nB1[2] + nB1[3]);
//			b2 += (float)(nB2[0] + nB2[1] + nB2[2] + nB2[3]);

//#endif

			b1 *= FLT_SCALE;
			b2 *= FLT_SCALE;

			cv::Point2f delta((float)((A12*b2 - A22*b1) * D),
				(float)((A12*b1 - A11*b2) * D));
			//delta = -delta;

			nextPt += delta;
			nextPts[ptidx] = nextPt + halfWin;

			if (delta.ddot(delta) <= criteria.epsilon)
				break;

			if (j > 0 && std::abs(delta.x + prevDelta.x) < 0.01 &&
				std::abs(delta.y + prevDelta.y) < 0.01)
			{
				nextPts[ptidx] -= delta*0.5f;
				break;
			}
			prevDelta = delta;
		}

		if (status[ptidx] && err && level == 0 && (flags & CV_LKFLOW_GET_MIN_EIGENVALS) == 0)
		{
			cv::Point2f nextPoint = nextPts[ptidx] - halfWin;
			cv::Point inextPoint;

			inextPoint.x = cvFloor(nextPoint.x);
			inextPoint.y = cvFloor(nextPoint.y);

			if (inextPoint.x < -winSize.width || inextPoint.x >= J.cols ||
				inextPoint.y < -winSize.height || inextPoint.y >= J.rows)
			{
				if (status)
					status[ptidx] = false;
				continue;
			}

			float aa = nextPoint.x - inextPoint.x;
			float bb = nextPoint.y - inextPoint.y;
			iw00 = cvRound((1.f - aa)*(1.f - bb)*(1 << W_BITS));
			iw01 = cvRound(aa*(1.f - bb)*(1 << W_BITS));
			iw10 = cvRound((1.f - aa)*bb*(1 << W_BITS));
			iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;
			float errval = 0.f;

			for (y = 0; y < winSize.height; y++)
			{
				const uchar* Jptr = (const uchar*)J.data + (y + inextPoint.y)*stepJ + inextPoint.x*cn;
				const deriv_type* Iptr = (const deriv_type*)(IWinBuf.data + y*IWinBuf.step);

				for (x = 0; x < winSize.width*cn; x++)
				{
					int diff = CV_DESCALE(Jptr[x] * iw00 + Jptr[x + cn] * iw01 +
						Jptr[x + stepJ] * iw10 + Jptr[x + stepJ + cn] * iw11,
						W_BITS1 - 5) - Iptr[x];
					errval += std::abs((float)diff);
				}
			}
			err[ptidx] = errval * 1.f / (32 * winSize.width*cn*winSize.height);
		}
	}
//#ifdef CV_NEONN0
//    free(nB1);
//    free(nB2);
//#endif
}
void calcOpticalFlowPyrLKNeon(cv::InputArray _prevImg, cv::InputArray _nextImg,
	cv::InputArray _prevPts, cv::InputOutputArray _nextPts,
	cv::OutputArray _status, cv::OutputArray _err,
	cv::Size winSize, int maxLevel,
	cv::TermCriteria criteria,
	int flags, double minEigThreshold)
{
	cv::Mat prevPtsMat = _prevPts.getMat();
	const int derivDepth = cv::DataType<deriv_type>::depth;

	CV_Assert(maxLevel >= 0 && winSize.width > 2 && winSize.height > 2);

	int level = 0, i, npoints;
	CV_Assert((npoints = prevPtsMat.checkVector(2, CV_32F, true)) >= 0);

	if (npoints == 0)
	{
		_nextPts.release();
		_status.release();
		_err.release();
		return;
	}

	if (!(flags & cv::OPTFLOW_USE_INITIAL_FLOW))
		_nextPts.create(prevPtsMat.size(), prevPtsMat.type(), -1, true);

	cv::Mat nextPtsMat = _nextPts.getMat();
	CV_Assert(nextPtsMat.checkVector(2, CV_32F, true) == npoints);

	const cv::Point2f* prevPts = (const cv::Point2f*)prevPtsMat.data;
	cv::Point2f* nextPts = (cv::Point2f*)nextPtsMat.data;

	_status.create((int)npoints, 1, CV_8U, -1, true);
	cv::Mat statusMat = _status.getMat(), errMat;
	CV_Assert(statusMat.isContinuous());
	uchar* status = statusMat.data;
	float* err = 0;

	for (i = 0; i < npoints; i++)
		status[i] = true;

	if (_err.needed())
	{
		_err.create((int)npoints, 1, CV_32F, -1, true);
		errMat = _err.getMat();
		CV_Assert(errMat.isContinuous());
		err = (float*)errMat.data;
	}

	std::vector<cv::Mat> prevPyr, nextPyr;
	int levels1 = -1;
	int lvlStep1 = 1;
	int levels2 = -1;
	int lvlStep2 = 1;

	if (_prevImg.kind() == cv::_InputArray::STD_VECTOR_MAT)
	{
		_prevImg.getMatVector(prevPyr);

		levels1 = int(prevPyr.size()) - 1;
		CV_Assert(levels1 >= 0);

		if (levels1 % 2 == 1 && prevPyr[0].channels() * 2 == prevPyr[1].channels() && prevPyr[1].depth() == derivDepth)
		{
			lvlStep1 = 2;
			levels1 /= 2;
		}

		// ensure that pyramid has reqired padding
		if (levels1 > 0)
		{
			cv::Size fullSize;
			cv::Point ofs;
			prevPyr[lvlStep1].locateROI(fullSize, ofs);
			CV_Assert(ofs.x >= winSize.width && ofs.y >= winSize.height
				&& ofs.x + prevPyr[lvlStep1].cols + winSize.width <= fullSize.width
				&& ofs.y + prevPyr[lvlStep1].rows + winSize.height <= fullSize.height);
		}

		if (levels1 < maxLevel)
			maxLevel = levels1;
	}

	if (_nextImg.kind() == cv::_InputArray::STD_VECTOR_MAT)
	{
		_nextImg.getMatVector(nextPyr);

		levels2 = int(nextPyr.size()) - 1;
		CV_Assert(levels2 >= 0);

		if (levels2 % 2 == 1 && nextPyr[0].channels() * 2 == nextPyr[1].channels() && nextPyr[1].depth() == derivDepth)
		{
			lvlStep2 = 2;
			levels2 /= 2;
		}

		// ensure that pyramid has reqired padding
		if (levels2 > 0)
		{
			cv::Size fullSize;
			cv::Point ofs;
			nextPyr[lvlStep2].locateROI(fullSize, ofs);
			CV_Assert(ofs.x >= winSize.width && ofs.y >= winSize.height
				&& ofs.x + nextPyr[lvlStep2].cols + winSize.width <= fullSize.width
				&& ofs.y + nextPyr[lvlStep2].rows + winSize.height <= fullSize.height);
		}

		if (levels2 < maxLevel)
			maxLevel = levels2;
	}

	if (levels1 < 0)
		maxLevel = buildOpticalFlowPyramid(_prevImg, prevPyr, winSize, maxLevel, false);

	if (levels2 < 0)
		maxLevel = buildOpticalFlowPyramid(_nextImg, nextPyr, winSize, maxLevel, false);

	if ((criteria.type & cv::TermCriteria::COUNT) == 0)
		criteria.maxCount = 30;
	else
		criteria.maxCount = std::min(std::max(criteria.maxCount, 0), 100);
	if ((criteria.type & cv::TermCriteria::EPS) == 0)
		criteria.epsilon = 0.01;
	else
		criteria.epsilon = std::min(std::max(criteria.epsilon, 0.), 10.);
	criteria.epsilon *= criteria.epsilon;

	// dI/dx ~ Ix, dI/dy ~ Iy
	cv::Mat derivIBuf;
	if (lvlStep1 == 1)
		derivIBuf.create(prevPyr[0].rows + winSize.height * 2, prevPyr[0].cols + winSize.width * 2, CV_MAKETYPE(derivDepth, prevPyr[0].channels() * 2));

	for (level = maxLevel; level >= 0; level--)
	{
		cv::Mat derivI;
		if (lvlStep1 == 1)
		{
			cv::Size imgSize = prevPyr[level * lvlStep1].size();
			cv::Mat _derivI(imgSize.height + winSize.height * 2,
				imgSize.width + winSize.width * 2, derivIBuf.type(), derivIBuf.data);
			derivI = _derivI(cv::Rect(winSize.width, winSize.height, imgSize.width, imgSize.height));
			calcSharrDerivNeon(prevPyr[level * lvlStep1], derivI);
			copyMakeBorder(derivI, _derivI, winSize.height, winSize.height, winSize.width, winSize.width, cv::BORDER_CONSTANT | cv::BORDER_ISOLATED);
		}
		else
			derivI = prevPyr[level * lvlStep1 + 1];

		CV_Assert(prevPyr[level * lvlStep1].size() == nextPyr[level * lvlStep2].size());
		CV_Assert(prevPyr[level * lvlStep1].type() == nextPyr[level * lvlStep2].type());

		typedef LKTrackerInvoker1 LKTrackerInvoker1;


		cv::parallel_for_(cv::Range(0, npoints), LKTrackerInvoker1(prevPyr[level * lvlStep1], derivI,
			nextPyr[level * lvlStep2], prevPts, nextPts,
			status, err,
			winSize, criteria, level, maxLevel,
			flags, (float)minEigThreshold));
	}
}
MedianFlowTracker::MedianFlowTracker()
{
	tracking_lost_threadhold_ = 100;
	optical_flow_pyramid_levels_ = 3;
	tracking_point_rows_ = 15;
	tracking_point_cols_ = 15;
	use_gpu_ = false;
	prev_points_.resize(tracking_point_rows_ * tracking_point_cols_);
	pred_points_.resize(tracking_point_rows_ * tracking_point_cols_);
	status_.resize(tracking_point_rows_ * tracking_point_cols_);
	scale_ratio_ = 0.0f;

	status_uniform_track = false;
	status_GFT_track = false;
}

MedianFlowTracker::~MedianFlowTracker()
{
	// do nothing
}

void MedianFlowTracker::start_tracking(const cv::Mat &image, const cv::Rect &boundingbox)
{
	prev_image_ = image.clone();
	prev_boundingbox_ = boundingbox;
	// LOGD("boo-debug-2 : start_tracking bb = [ %f , %f, %f, %f ]\n", boundingbox.x, boundingbox.y, boundingbox.width, boundingbox.height);
}

void MedianFlowTracker::kalman_filter_init()
{
	//Intialize Kalman Filter

}

void MedianFlowTracker::setParameter(const float &scale_ratio){
	scale_ratio_ = scale_ratio;
}


bool MedianFlowTracker::generate_tracking_points_uniform(std::vector<cv::Point2f> &points, const BoundingBoxType &boundingbox)
{
	// LOGD("boo-debug-2 : boundingbox = [ %f , %f, %f, %f ]\n", boundingbox.x, boundingbox.y, boundingbox.width, boundingbox.height);
	//Protection
	if (boundingbox.height < 10 || boundingbox.width < 10){
		// LOGD("boo-debug2 : w or h < 10 \n");
		return false;
	}

	if (boundingbox.x > prev_image_.cols - 10 || boundingbox.y > prev_image_.rows - 10){
		// LOGD("boo-debug2 : x > p.cols or y > p.rows \n");
		return false;
	}


	// fill points
	float step_rows = (float)(boundingbox.height - 10) / tracking_point_rows_;
	float step_cols = (float)(boundingbox.width - 10) / tracking_point_cols_;
	int idx = 0;
	for (float r = boundingbox.y, step_r = 0; step_r < tracking_point_rows_; r += step_rows, step_r++)
	{
		for (float c = boundingbox.x, step_c = 0; step_c < tracking_point_cols_; c += step_cols, step_c++)
		{
			points[idx++] = cv::Point2f(c + 5, r + 5);
		}
	}
	if (idx < 4){
		// LOGD("boo-debug2 : idx < 4 \n");
		return false;
	}

	return true;
}

bool MedianFlowTracker::generate_tracking_points_GFT(std::vector<cv::Point2f> &points, const BoundingBoxType &boundingbox)
{
	//assert(points.size() == tracking_point_rows_ * tracking_point_cols_);
	//assert(boundingbox.width > tracking_point_cols_ + 10 && boundingbox.height > tracking_point_rows_ + 10);

	//Protection
	if (boundingbox.height < 10 || boundingbox.width < 10)
		return false;
	if (boundingbox.x > prev_image_.cols - 10 || boundingbox.y > prev_image_.rows - 10)
		return false;

	//Add additional reliable feature points for tracking Brian TX Bai 4/1/2016
	cv::Mat mask = cv::Mat::zeros(prev_image_.size(), CV_8U);
	BoundingBoxType tempBoundingbox = boundingbox;
	tempBoundingbox.x = MAX(boundingbox.x, 0.0f);
	tempBoundingbox.y = MAX(boundingbox.y, 0.0f);

	if(tempBoundingbox.x+tempBoundingbox.width >= prev_image_.cols)
		tempBoundingbox.width = prev_image_.cols-1-tempBoundingbox.x;
	if(tempBoundingbox.y+tempBoundingbox.height >= prev_image_.rows)
		tempBoundingbox.height = prev_image_.rows-1-tempBoundingbox.y;

	cv::Mat ROI(mask, cv::Rect(tempBoundingbox.x,tempBoundingbox.y,tempBoundingbox.width,tempBoundingbox.height));
	//cv::Mat ROI(mask, cv::Rect(0.0,0.0,100.0,100.0));
	ROI = cv::Scalar(255);

	//cv::Mat roi(mask, cv::Rect intBB);
	cv::goodFeaturesToTrack(prev_image_, points, tracking_point_rows_*tracking_point_cols_, 0.1, 3, mask, 3, 0, 0.004);
	if (points.size() < 4)
		return false;
	return true;
}

//bool MedianFlowTracker::KFPredict()
//{
	//if (pred_period_ > frm_counter_)
	//	return false;
	//
	//int stateSize = 9;
	//int measureSize = 3;
	//int contrSize = 0;

	//unsigned int type = CV_32F;
	//cv::KalmanFilter kf_(stateSize, measureSize, contrSize, type);

	//cv::Mat state(stateSize, 1, type);		      // [x,y,d,v_x,v_y,v_d,a_x,a_y,a_d]
	//cv::Mat measurement(measureSize, 1, type);    // [x,y,d]

	//// Transition State Matrix A
	//// Note: set dT at each processing step!
	//// [ 1 0 0  t  0 0 1/2t^2 0       0     ]
	//// [ 0 1 0  0  t 0 0      1/2t^2  0     ]
	//// [ 0 0 1  0  0 t 0      0       1/2t^2]
	//// [ 0 0 0  1  0 0 t      0       0     ]
	//// [ 0 0 0  0  1 0 0      t       0     ]
	//// [ 0 0 0  0  0 1 0      0       t     ]
	//cv::setIdentity(kf_.transitionMatrix);

	//// Measure Matrix H
	//// [ 1 0 0 0 0 0 0 0 0]
	//// [ 0 1 0 0 0 0 0 0 0]
	//// [ 0 0 1 0 0 0 0 0 0]
	//kf.measurementMatrix = cv::Mat::zeros(measureSize, stateSize, type);
	//kf.measurementMatrix.at(0) = 1.0f;
	//kf.measurementMatrix.at(10) = 1.0f;
	//kf.measurementMatrix.at(20) = 1.0f;



	////Process Noise Covariance Matrix Q
	//cv::setIdentity(kf.processNoiseCov, cv::Scalar(1e-2));

	//// Measures Noise Covariance Matrix R
	//cv::setIdentity(kf.measurementNoiseCov, cv::Scalar(1e-1));

//}


bool MedianFlowTracker::predict(const cv::Mat &image, cv::Rect &boundingbox)
{
	this->curr_image_ = image;
	std::vector<cv::Point2f> prev_points_GFT;
	std::vector<cv::Point2f> median_filted_prev_points_uniform;
	std::vector<cv::Point2f> median_filted_pred_points_uniform;
	std::vector<cv::Point2f> median_filted_prev_points_GFT;
	std::vector<cv::Point2f> median_filted_pred_points_GFT;
	median_of_forward_backward_errors_uniform = 0.0f;
	float median_of_nccs_uniform = 0.0f;
	median_of_forward_backward_errors_GFT = 0.0f;
	float median_of_nccs_GFT = 0.0f;
	cv::Point2f offset = cv::Point2f(0, 0);
	float scale = 1.0f;
	cv::Point2f offset_uniform = cv::Point2f(0,0);
	float scale_uniform = 1.0f;
	cv::Point2f offset_GFT = cv::Point2f(0, 0);
	float scale_GFT = 1.0f;
	float weight_err_uniform = 0.0f, weight_err_GFT = 0.0f, weight_ncc_uniform = 0.0f, weight_ncc_GFT = 0.0f;

	if(prev_image_.size() != curr_image_.size()){
		prev_image_ = curr_image_;
		// LOGD("boo-debug1 : prev_image_.size() != curr_image_.size() \n");
		return false;
	}

	// generate tracking points according to bounding box
	status_uniform_track = generate_tracking_points_uniform(prev_points_, prev_boundingbox_);

//	if (prev_GFT_points_.size() < 20)
//		status_GFT_track = generate_tracking_points_GFT(prev_points_GFT, prev_boundingbox_);
//	else{
//		prev_points_GFT = prev_GFT_points_;
//		status_GFT_track = true;
//	}
	if (!status_uniform_track /*&& !status_GFT_track*/){
		// LOGD("boo-debug1 : !status_uniform_track \n");
		return false;
	}

	//printf("\nprev_box : x y width height: %d   %d   %d  %d \n",
	//	(int)prev_boundingbox_.x, (int)prev_boundingbox_.y, (int)prev_boundingbox_.width, (int)prev_boundingbox_.height);

	// do feature points tracking and filtering
	is_GFT = false;
	status_uniform_track = track_and_filter_points(prev_image_, curr_image_, prev_points_, median_filted_prev_points_uniform, median_filted_pred_points_uniform,
										median_of_forward_backward_errors_uniform, median_of_nccs_uniform);
	// LOGD("boo-debug1 : erro1 =  %f  , erro2 = %f  \n", median_of_forward_backward_errors_uniform, median_of_nccs_uniform);
	if (status_uniform_track)
		calculate_offset_and_scales(median_filted_prev_points_uniform, median_filted_pred_points_uniform, offset_uniform, scale_uniform);

	offset = offset_uniform;
	scale  = scale_uniform;
/*
	is_GFT = true;
	status_GFT_track = track_and_filter_points(prev_image_, curr_image_, prev_points_GFT, median_filted_prev_points_GFT, median_filted_pred_points_GFT,
										median_of_forward_backward_errors_GFT, median_of_nccs_GFT);
	if (status_GFT_track)
		calculate_offset_and_scales(median_filted_prev_points_GFT, median_filted_pred_points_GFT, offset_GFT, scale_GFT);

	if (!status_uniform_track && !status_GFT_track)
		return false;

	// combine the offset and scales results by Fuzzy logic
	if (status_uniform_track == true && status_GFT_track == false){
		weight_err_uniform = 1.0f;
		weight_ncc_uniform = 1.0f;
		weight_err_GFT = 0.0f;
		weight_ncc_GFT = 0.0f;
	}
	else if (status_uniform_track == false && status_GFT_track == true){
		weight_err_uniform = 0.0f;
		weight_ncc_uniform = 0.0f;
		weight_err_GFT = 1.0f;
		weight_ncc_GFT = 1.0f;
	}
	else{
		weight_err_uniform = 1 - median_of_forward_backward_errors_uniform / (median_of_forward_backward_errors_uniform + median_of_forward_backward_errors_GFT + MFT_ZERO);
		weight_err_GFT = 1 - median_of_forward_backward_errors_GFT / (median_of_forward_backward_errors_uniform + median_of_forward_backward_errors_GFT + MFT_ZERO);
		weight_ncc_uniform = median_of_nccs_uniform / (median_of_nccs_uniform + median_of_nccs_GFT + MFT_ZERO);
		weight_ncc_GFT = median_of_nccs_GFT / (median_of_nccs_uniform + median_of_nccs_GFT + MFT_ZERO);
	}

	offset = offset_uniform * (weight_err_uniform + weight_ncc_uniform) * 0.5 + offset_GFT * (weight_err_GFT + weight_ncc_GFT) * 0.5;
	scale  = scale_uniform * (weight_err_uniform + weight_ncc_uniform) * 0.5 + scale_GFT * (weight_err_GFT + weight_ncc_GFT) * 0.5;
*/

	if (scale < 1.005 && scale > 0.995)
		scale = 1;
	
	// Constraint the scale variation Brian TX Bai 3/23/2016
	if (scale_ratio_ > MFT_ZERO){
		if (scale > 1.0f)
			scale = MIN((1 + scale_ratio_), scale);
		else
			scale = MAX((1 - scale_ratio_), scale);
	}

	// update pred boundingbox

	update_boundingbox(prev_boundingbox_, pred_boundingbox_, offset, scale);
	// LOGD("boo-debug1 : prev_boundingbox_ [%f , %f , %f , %f] offset [ %f , %f ] scale = %f\n" , prev_boundingbox_.x , prev_boundingbox_.y , prev_boundingbox_.width , prev_boundingbox_.height ,offset.x , offset.y,scale);
	// update prev image and others
	prev_image_ = curr_image_.clone();
	boundingbox = pred_boundingbox_;
	prev_boundingbox_ = pred_boundingbox_;
	// LOGD("boo-debug1 : return true \n");
	return true;
}

bool MedianFlowTracker::track_and_filter_points(const cv::Mat &prev_image, const cv::Mat &curr_image,
	const std::vector<cv::Point2f> &prev_points, std::vector<cv::Point2f> &median_filted_prev_points,
	std::vector<cv::Point2f> &median_filted_pred_points, float &median_of_forward_backward_errors, float &median_of_nccs)
{
	// do optical flow
	bool status = true;
	std::vector<cv::Point2f> prev_points_backward;
	std::vector<cv::Point2f> pred_points;
	status_.resize(prev_points.size());
	status = do_forwardbackward_opticalflow(prev_image_, curr_image_, prev_points,
											pred_points, prev_points_backward, status_);
	if (status != true)
		return false;
	// filter points according to status
	std::vector<cv::Point2f> filted_prev_points;
	std::vector<cv::Point2f> filted_pred_points;
	std::vector<cv::Point2f> filted_pred_points_backward;
	filter_points_accordingto_status(prev_points, pred_points, prev_points_backward,
		filted_prev_points, filted_pred_points, filted_pred_points_backward, status_);
	//printf("filter_points_accordingto_status %lu\n", filted_prev_points.size());
	// points after median filtered is too small
	if (filted_prev_points.size() < 4)
	{
		return false;
	}

	// calculate forward backward errors
	std::vector<float> forward_backward_errors(filted_prev_points.size());
	calculate_forwardbackward_errors(filted_prev_points, filted_pred_points_backward, forward_backward_errors);

	// get median of forward backward errors
	median_of_forward_backward_errors = get_median_of_forwardbackward_errors(forward_backward_errors);
	if (is_GFT == true){
		copy_GFT_points(median_of_forward_backward_errors_GFT, filted_prev_points, forward_backward_errors, 5);
	}

	// lost !!!
	//printf("get_median_of_forwardbackward_errors %f\n", median_of_forward_backward_errors);
	// 	if (median_of_forward_backward_errors > tracking_lost_threadhold_)
	// 	{
	// 		return false;
	// 	}

	// calculate nccs
	std::vector<float> nccs(filted_prev_points.size());
	calculate_nccs(prev_image_, curr_image_, filted_prev_points, filted_pred_points, nccs);

	// get median of forward backward errors
	median_of_nccs = get_median_of_nccs(nccs);

	// filter points according to median of fb errors and median of nccs
	filter_points_accordingto_medians(filted_prev_points, filted_pred_points, median_filted_prev_points,
		median_filted_pred_points, forward_backward_errors, nccs, median_of_forward_backward_errors, median_of_nccs);
	// points after median filtered is too small
	printf("\nfilter_points_accordingto_medians %lu\n", median_filted_pred_points.size());
	if (median_filted_pred_points.size() < 4)
	{
		return false;
	}
	return true;
}

bool MedianFlowTracker::do_forwardbackward_opticalflow(const cv::Mat &prev_image, const cv::Mat &curr_image,
	const std::vector<cv::Point2f> &prev_points, std::vector<cv::Point2f> &pred_points,
		std::vector<cv::Point2f> &prev_points_backward, std::vector<uchar> &status)
{
	//assert(status.size() == pred_points.size());
	clock_t start = 0,end = 0;
	std::vector<float> err;
	cv::TermCriteria termcrit(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
	cv::Size windows_size(4, 4);
	std::vector<uchar> forward_status(status.size());
	std::vector<uchar> backward_status(status.size());
	std::vector<cv::Point2f> back_prev_points = prev_points;

	// forward
	if (prev_points.size() == 0)	return false;

	start = clock();
	calcOpticalFlowPyrLK(prev_image, curr_image, prev_points, pred_points, forward_status,
           		err, windows_size, optical_flow_pyramid_levels_, termcrit, 0, 1e-4);
    //end = clock();
    //LOGD("tldt : lk1 %d ms\n",(end-start)/1000);
    //start = 0;
    //end = 0;
    //start = clock();
    //calcOpticalFlowPyrLKNeon(prev_image, curr_image, prev_points, pred_points, forward_status,
    //           		err, windows_size, optical_flow_pyramid_levels_, termcrit, 0, 1e-4);
	end = clock();
	// LOGD("tldt : lk2 %d ms\n",(end-start)/1000);
	// backward
	if (pred_points.size() == 0)	return false;
	//calcOpticalFlowPyrLKNeon(curr_image, prev_image, pred_points, back_prev_points, backward_status,
	//	err, windows_size, optical_flow_pyramid_levels_, termcrit, 0, 1e-4/*, cv::OPTFLOW_USE_INITIAL_FLOW*/);
    calcOpticalFlowPyrLK(curr_image, prev_image, pred_points, back_prev_points, backward_status,
      	err, windows_size, optical_flow_pyramid_levels_, termcrit, 0, 1e-4/*, cv::OPTFLOW_USE_INITIAL_FLOW*/);

	prev_points_backward = back_prev_points;
	for (int v = 0; v < status.size(); v++)
	{
		status[v] = forward_status[v] & backward_status[v];
	}
	return true;
}

void MedianFlowTracker::copy_GFT_points(const float median_of_forward_backward_errors, const std::vector<cv::Point2f> &prev_points_GFT, const std::vector<float> &forward_backward_errors, const float min_err){
	prev_GFT_points_.clear();
	//float err =
	for (int i = 0; i < prev_points_GFT.size(); i++){
		if (forward_backward_errors[i] < 2)
			prev_GFT_points_.push_back(prev_points_GFT[i]);
	}
}

void MedianFlowTracker::filter_points_accordingto_status(const std::vector<cv::Point2f> &prev_points,
	const std::vector<cv::Point2f> &pred_points, const std::vector<cv::Point2f> &prev_points_backward,
		std::vector<cv::Point2f> &filted_prev_points, std::vector<cv::Point2f> &filted_pred_points,
		std::vector<cv::Point2f> &filted_prev_points_backward, const std::vector<uchar> &status)
{
	filted_prev_points.clear();
	filted_pred_points.clear();
	for (int v = 0; v < status.size(); v++)
	{
		if (status[v] == 1)
		{
			filted_prev_points.push_back(prev_points[v]);
			filted_pred_points.push_back(pred_points[v]);
			filted_prev_points_backward.push_back(prev_points_backward[v]);
		}
	}
}

void MedianFlowTracker::calculate_forwardbackward_errors(const std::vector<cv::Point2f> &prev_points,
	const std::vector<cv::Point2f> &pred_points, std::vector<float> &forward_backward_errors)
{
	assert(forward_backward_errors.size() == prev_points.size());
	for (int v = 0; v < prev_points.size(); v++)
	{
		forward_backward_errors[v] = (prev_points[v].x - pred_points[v].x)*
			(prev_points[v].x - pred_points[v].x) +
			(prev_points[v].y - pred_points[v].y) * (prev_points[v].y - pred_points[v].y);
	}
}

float MedianFlowTracker::get_median_of_forwardbackward_errors(const std::vector<float> &forward_backward_errors)
{
	assert(forward_backward_errors.size() > 0);
	std::vector<float> tmp = forward_backward_errors;
	std::nth_element(tmp.begin(), tmp.begin() + tmp.size() / 2, tmp.end());
	return tmp[tmp.size() / 2];
}

inline float calculate_CV_TM_CCORR_NORMED(const cv::Mat &prev, const cv::Mat &pred)
{
	float sum0 = 0.0f;
	float sum1 = 0.0f;
	float sum2 = 0.0f;
	//LOGD("track xc : size %d,%d \n",prev.cols,prev.rows);
	//LOGD("track xc : size %d,%d \n",pred.cols,pred.rows);
#if CV_NEON

    float32x4_t pred_t,prev_t,vv_result,dd_result,dv_result,
        vv_temp,dv_temp,dd_temp,v_mean,d_mean,presubd_t,presubv_t;

    vv_result = vdupq_n_f32((float)0.0f);
    dd_result = vdupq_n_f32((float)0.0f);
    dv_result = vdupq_n_f32((float)0.0f);
    for (int r = 0; r < prev.rows; r++) {
        const float* pd = (float*)pred.ptr<float>(r);
        const float* pv = (float*)prev.ptr<float>(r);
        for (int c = 0; c < prev.cols; c+=4) {
            pred_t = vld1q_f32((float32_t*)(pd+c));
            prev_t = vld1q_f32((float32_t*)(pv+c));
            dd_result = vaddq_f32(dd_result,pred_t);
            vv_result = vaddq_f32(vv_result,prev_t);

            //sum0 += (float)prev.at<uchar>(r, c);
            //sum1 += (float)prev.at<uchar>(r, c);
        }
    }
    sum0 += vgetq_lane_f32(vv_result,0);
    sum0 += vgetq_lane_f32(vv_result,1);
    sum0 += vgetq_lane_f32(vv_result,2);
    sum0 += vgetq_lane_f32(vv_result,3);
    sum1 += vgetq_lane_f32(dd_result,0);
    sum1 += vgetq_lane_f32(dd_result,1);
    sum1 += vgetq_lane_f32(dd_result,2);
    sum1 += vgetq_lane_f32(dd_result,3);
    //LOGD("track xc : sum %f,%f \n",sum0,sum1);
    float prev_mean = sum0 / (prev.cols * prev.rows);
    float pred_mean = sum1 / (pred.cols * pred.rows);
    //LOGD("track xc : mean :%f,%f \n",prev_mean,pred_mean);
    sum0 = 0.0f;
    sum1 = 0.0f;
    sum2 = 0.0f;
    v_mean = vdupq_n_f32(prev_mean);
    d_mean = vdupq_n_f32(pred_mean);
    vv_result = vdupq_n_f32((float)0.0f);
    dd_result = vdupq_n_f32((float)0.0f);
    dv_result = vdupq_n_f32((float)0.0f);

    for (int r = 0; r < prev.rows; r++) {
        const float* pd = (float*)pred.ptr<float>(r);
        const float* pv = (float*)prev.ptr<float>(r);
        for (int c = 0; c < prev.cols; c+=4) {
            presubd_t = vdupq_n_f32((float)0.0f);
            presubv_t = vdupq_n_f32((float)0.0f);
            pred_t = vld1q_f32((float32_t*)(pd+c));
            prev_t = vld1q_f32((float32_t*)(pv+c));
            presubd_t = vsubq_f32(pred_t,d_mean);
            presubv_t = vsubq_f32(prev_t,v_mean);
            vv_temp = vmulq_f32(presubv_t,presubv_t);    // v[0,1,2,3]*v[0,1,2,3]
            dd_temp = vmulq_f32(presubd_t,presubd_t);
            dv_temp = vmulq_f32(presubd_t,presubv_t);
            vv_result = vaddq_f32(vv_result,vv_temp);
            dd_result = vaddq_f32(dd_result,dd_temp);
            dv_result = vaddq_f32(dv_result,dv_temp);
        }

    }
    sum0 += vgetq_lane_f32(vv_result,0);
    sum0 += vgetq_lane_f32(vv_result,1);
    sum0 += vgetq_lane_f32(vv_result,2);
    sum0 += vgetq_lane_f32(vv_result,3);
    sum1 += vgetq_lane_f32(dd_result,0);
    sum1 += vgetq_lane_f32(dd_result,1);
    sum1 += vgetq_lane_f32(dd_result,2);
    sum1 += vgetq_lane_f32(dd_result,3);
    sum2 += vgetq_lane_f32(dv_result,0);
    sum2 += vgetq_lane_f32(dv_result,1);
    sum2 += vgetq_lane_f32(dv_result,2);
    sum2 += vgetq_lane_f32(dv_result,3);
#else
	float prev_mean = 0.0f;
	float pred_mean = 0.0f;
	for (int r = 0; r < prev.rows; r++) {
		for (int c = 0; c < prev.cols; c++) {
			sum0 += (float)prev.at<uchar>(r, c);
			sum1 += (float)pred.at<uchar>(r, c);
		}
	}
	prev_mean = sum0 / (prev.cols * prev.rows);
	pred_mean = sum1 / (pred.cols * pred.rows);

	for (int r = 0; r < prev.rows; r++) {
		for (int c = 0; c < prev.cols; c++) {
			float pred_t = (float)pred.at<uchar>(r, c);
			float prev_t = (float)prev.at<uchar>(r, c);
			sum0 += (prev_t - prev_mean)*(prev_t-prev_mean);
			sum1 += (pred_t - pred_mean)*(pred_t - pred_mean);
			sum2 += (prev_t - prev_mean)*(pred_t - pred_mean);
		}
	}
#endif
	if (sum0< 0.1 && sum1< 0.1) 
		return 0.1f;
	else if (sum0 < 0.1 || sum1 < 0.1)
		return 0.0f;
	else
		return sum2 / (float)(sqrt((double)sum0)*sqrt((double)sum1));
}

void MedianFlowTracker::calculate_nccs(const cv::Mat &prev_image, const cv::Mat &curr_image, const std::vector<cv::Point2f> &prev_points,
	const std::vector<cv::Point2f> &pred_points, std::vector<float> &nccs)
{
	assert(pred_points.size() > 0);
	for (int v = 0; v < pred_points.size(); v++)
	{
		cv::Mat prev_subimage;
		cv::getRectSubPix(prev_image, cv::Size(12, 12), prev_points[v], prev_subimage);
		cv::Mat curr_subimage;
		cv::getRectSubPix(curr_image, cv::Size(12, 12), pred_points[v], curr_subimage);
// 		cv::Mat corr;
// 		cv::matchTemplate(prev_subimage, curr_subimage, corr, CV_TM_CCORR_NORMED);
// 		nccs[v] = *((float*)corr.data);
#ifdef NEON
        prev_subimage.convertTo(prev_subimage,CV_32F);
        curr_subimage.convertTo(curr_subimage,CV_32F);
#endif
		nccs[v] = calculate_CV_TM_CCORR_NORMED(prev_subimage, curr_subimage);
	}
}

float MedianFlowTracker::get_median_of_nccs(const std::vector<float> &nccs)
{
	assert(nccs.size() > 0);
	std::vector<float> tmp = nccs;
	std::nth_element(tmp.begin(), tmp.begin() + tmp.size() / 2, tmp.end());
	return tmp[tmp.size() / 2];
}

void MedianFlowTracker::filter_points_accordingto_medians(const std::vector<cv::Point2f> &prev_points,
	const std::vector<cv::Point2f> &pred_points, std::vector<cv::Point2f> &filted_prev_points,
	std::vector<cv::Point2f> &filted_pred_points, const std::vector<float> &fbs, const std::vector<float> &nccs,
	const float median_fbs, const float median_nccs)
{
	assert(fbs.size() == nccs.size());
	filted_prev_points.clear();
	filted_pred_points.clear();
	for (int v = 0; v < fbs.size(); v++)
	{
		if (fbs[v] <= median_fbs & nccs[v] >= median_nccs)
		{
			filted_prev_points.push_back(prev_points[v]);
			filted_pred_points.push_back(pred_points[v]);
		}
	}
}

static inline float distance(const cv::Point2f &p1, const cv::Point2f &p2)
{
	return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

void MedianFlowTracker::calculate_offset_and_scales(const std::vector<cv::Point2f> &prev_points,
	const std::vector<cv::Point2f> &pred_points, cv::Point2f &offset, float &scale)
{
	// get median of offset x
	std::vector<float> offset_x(prev_points.size());
	for (int v = 0; v < prev_points.size(); v++)
	{
		offset_x[v] = pred_points[v].x - prev_points[v].x;
	}

	std::nth_element(offset_x.begin(), offset_x.begin() + offset_x.size() / 2, offset_x.end());

	// get median of offset y
	std::vector<float> offset_y(prev_points.size());
	for (int v = 0; v < prev_points.size(); v++)
	{
		offset_y[v] = pred_points[v].y - prev_points[v].y;
	}

	std::nth_element(offset_y.begin(), offset_y.begin() + offset_y.size() / 2, offset_y.end());
	offset = cv::Point2f(offset_x[offset_x.size() / 2], offset_y[offset_y.size() / 2]);

	// get median of scale
	std::vector<float> candidate_scales;
	for (int v = 0; v < prev_points.size(); v++) {
		for (int u = v + 1; u < prev_points.size(); u++) {
			float dis_prev = distance(prev_points[v], prev_points[u]);
			float dis_pred = distance(pred_points[v], pred_points[u]);
			candidate_scales.push_back(dis_pred / dis_prev);
		}
	}
	std::nth_element(candidate_scales.begin(), candidate_scales.begin() + candidate_scales.size() / 2, candidate_scales.end());
	scale = candidate_scales[candidate_scales.size() / 2];
}

void MedianFlowTracker::update_boundingbox(const BoundingBoxType &prev_boundingbox, BoundingBoxType &pred_boundingbox,const cv::Point2f &offset, float scale)
{
	float scale_offset_x = 0.5*(scale - 1) * prev_boundingbox.width;
	float scale_offset_y = 0.5*(scale - 1) * prev_boundingbox.height;
	pred_boundingbox.x = prev_boundingbox.x + offset.x - scale_offset_x;
	pred_boundingbox.y = prev_boundingbox.y + offset.y - scale_offset_y;
	pred_boundingbox.width = prev_boundingbox.width * scale;
	pred_boundingbox.height = prev_boundingbox.height * scale;
}


void MedianFlowTracker::reset_boundingbox(const BoundingBoxType &reset_boundingbox){
	prev_boundingbox_ = reset_boundingbox;
}

float MedianFlowTracker::getErr(){

	if (status_uniform_track == true && status_GFT_track == false){
		LOGD("boo-debug-- : return 1 erro_uniform =  %f \n", median_of_forward_backward_errors_uniform);
		return median_of_forward_backward_errors_uniform;
	}
	else if (status_uniform_track == false && status_GFT_track == true){
		LOGD("boo-debug-- : return 2 erro_GFT =  %f \n", median_of_forward_backward_errors_GFT);
		return median_of_forward_backward_errors_GFT;
	}
	else{
		LOGD("boo-debug-- : return 3 erro_min =  %f \n", MIN(median_of_forward_backward_errors_uniform, median_of_forward_backward_errors_GFT));
		return MIN(median_of_forward_backward_errors_uniform, median_of_forward_backward_errors_GFT);
	}

}
