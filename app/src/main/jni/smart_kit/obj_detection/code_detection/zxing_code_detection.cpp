#define _ANDROID_
#define DEBUG
#include <zxing/common/Counted.h>
#include <zxing/Binarizer.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/ReaderException.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/common/HybridBinarizer.h>
#include <zxing/Exception.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>

#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/ImageReaderSource.h>
#include <zxing/result.h>

#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>

#include "zxing_code_detection.h"
#include "../../include/Log.h"

//using namespace std;
using namespace zxing;
using namespace zxing;
using namespace zxing::qrcode;

#include <zxing/resultParser/URIResultParser.h>
#include <zxing/resultParser/URIParsedResult.h>
#include <zxing/resultParser/ParsedResultType.h>
#include <zxing/resultParser/ResultParserHandle.h>
#include <zxing/resultParser/WebsiteTypeHandle.h>
#include <zxing/resultParser/WebsiteType.h>


static uint8_t checkUtfString(const char* bytes, const char** errorKind);

/**
 * calculate the forth position
 * @param dstTri        (input) three coordinates of rectangle
 * @return               the rest coordinate of the rectangle
 */
cv::Point2f computeP4(cv::Point2f dstTri[3]) {
	cv::Point2f srcTri[3];

	cv::Mat rot_mat(2, 3, CV_32FC1);
	cv::Mat warp_mat(2, 3, CV_32FC1);
	cv::Mat src, warp_dst, warp_rotate_dst;

	warp_dst = cv::Mat::zeros(src.rows, src.cols, src.type());

	srcTri[0] = cv::Point2f(0, 1);
	srcTri[1] = cv::Point2f(0, 0);
	srcTri[2] = cv::Point2f(1, 0);

	warp_mat = cv::getAffineTransform(srcTri, dstTri);
	cv::Point2f outPoint;
	outPoint.x = warp_mat.at<double>(0, 0) + warp_mat.at<double>(0, 1)
			+ warp_mat.at<double>(0, 2);
	outPoint.y = warp_mat.at<double>(1, 0) + warp_mat.at<double>(1, 1)
			+ warp_mat.at<double>(1, 2);

	return outPoint;
}

vector<Ref<Result> > decode(Ref<BinaryBitmap> image, DecodeHints hints) {
	Ref<Reader> reader(new MultiFormatReader);
	return vector < Ref<Result> > (1, reader->decode(image, hints));
}

/**
 * ZxingCodeDetection Constructor
 */
ZxingCodeDetection::ZxingCodeDetection() {
	mDecodeStr = "";
}

/**
 * ZxingCodeDetection Destructor
 */
ZxingCodeDetection::~ZxingCodeDetection() {
}


/**
 * QRcode detection
 * @param yuv420sp      (input) data for code detection
 * @param height        (input) yuv420sp data's height
 * @param width         (input) yuv420sp data's width
 * @param roix          (input) x coordinate of interesting rect
 * @param roiy          (input) y coordinate of interesting rect
 * @param roih          (input) height of interesting rect
 * @param roiw          (input) width of interesting resct
 * @param single        (input) true:detect single object| false:detect multiple objects
 * @return               true:No detect|1:BarCode|2:QRCode
 */
bool ZxingCodeDetection::detect(unsigned char *yuv420sp, int height, int width,
		int roix, int roiy, int roih, int roiw, bool single)
{
	bool bRetFlg = false;
	mCodePositon[0] = 0;
	mDecodeStr = "";
	vector<Ref<Result> > results;
	Ref<LuminanceSource> source = ImageReaderSource::create(yuv420sp, height, width, 1);
	Ref<Binarizer> binarizer;

	binarizer = new GlobalHistogramBinarizer(source);

//	bool try_harder = false;
	DecodeHints hints(DecodeHints::DEFAULT_HINT);
//	hints.setTryHarder(try_harder);

	Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
	try {
		results = decode(binary, hints);
	}
	catch (ReaderException const& re) {
		(void)re;
	}

	mCodeFormat = 0;
	if(results.size() != 0 )
	{
		ArrayRef< Ref<ResultPoint> > codepoints = results[0]->getResultPoints();
		mDecodeStr = results[0]->getText()->getText();
		mCodeFormat = (int)results[0]->getBarcodeFormat();
	    std::string result = results[0]->getText()->getText();
		cv::Point2f qrpoints[4];  //qrcode Coordinate points
		mCodePositon[0] = 1;
		for (int j = 0; j < codepoints->size(); j++)
		{
			mCodePositon[j*2 + 1] = codepoints[j]->getX();
			mCodePositon[j*2 + 2] = codepoints[j]->getY();

			qrpoints[j].x = codepoints[j]->getX();
			qrpoints[j].y = codepoints[j]->getY();
		}
		qrpoints[3] = computeP4(qrpoints);
//		mCodePositon[7] = qrpoints[3].x;
//		mCodePositon[8] = qrpoints[3].y;
		bRetFlg = true;
	}

	//parseResult();
	return bRetFlg;
}
/**
 * get QRCode Position
 * @param pos           (input|output) data for code position,pos[0]:code cnt
 * @param int           (input) pos length
 * @return               -1:No detect|1:BarCode|2:QRCode
 */
int ZxingCodeDetection::getCodePositon(int *pos, int len)
{
	if(len > 9) len = 9;
	memcpy(pos, mCodePositon, len*sizeof(int));
	int res_flag = -1;
	if(mCodeFormat == zxing::BarcodeFormat::QR_CODE)
	{
		res_flag = 2;//qrcode
	}
	else if(mCodeFormat == zxing::BarcodeFormat::EAN_13)
	{
		res_flag = 1;//barcode
	}
	return pos[0]==0 ? -1 : res_flag;
}

/**
 * get QRCode Format
 * @return               code format
 */
int ZxingCodeDetection::getFormat()
{
	return (int)mCodeFormat;
}

string getURLType2String(WebsiteTypeList type)
{
	string str = "";
	switch(type)
	{
	case WebsiteType_weixin:
		str = "weixin";
		break;
	case WebsiteType_wikipedia:
		str = "wiki";
		break;
	case WebsiteType_weibo:
		str = "weibo";
		break;
	case WebsiteType_baidu:
		str = "baidu";
		break;
	case WebsiteType_baike:
		str = "baike";
		break;
	case WebsiteType_nonmatched:
		str = " ";
		break;
	default:
		str = " ";
		break;
	}
  return str;
}

/**
 * parse QRCode content
 */
void ZxingCodeDetection::parseResult()
{
	std::string ret_str = "";
	if(mDecodeStr == "")
	{
		mCodeTypeStr = ret_str;
		return ;
	}
	ResultParserHandle resultParser;
	ParsedResult *mParsedResult = resultParser.parser(mDecodeStr);
      if(mParsedResult != NULL)
	{
		ParsedResultType type =  mParsedResult->getType();
		//mDecodeStr = mParsedResult->getDisplayResult();
		WebsiteTypeList urlType= WebsiteType_nonmatched;
		std::string strurlType = "";
		switch(type)
		{
		case ParsedResultType_URI:
			urlType = ((URIParsedResult*)mParsedResult)->getWebsiteType();
			strurlType = getURLType2String(urlType);
			ret_str =  "URL: " + strurlType;
			break;
		case ParsedResultType_TEXT:
			ret_str = "text";
			break;
		case ParsedResultType_EMAIL_ADDRESS:
			ret_str = "EMAIL_ADDRESS";
			break;
		case ParsedResultType_TEL:
			ret_str = "TEL";
			break;
		 default:
		   ret_str = "TEXT_d";
		   break;
		}
	}
	else
	{
		ret_str = "TEXT_n";
	}
     mCodeTypeStr = ret_str;
}

/**
 * get QRCode decode content
 * @return               decode string
 */
string ZxingCodeDetection::getDecodeStr()
{
	const char *temp = mDecodeStr.c_str();
	const char* errorKind = NULL;
	char uChar1 = checkUtfString(temp, &errorKind);
	if(errorKind != NULL)
		mDecodeStr = "Chinese Unsupported";
	return mDecodeStr;
}

/**
 * get QRCode type
 * @return               code type string
 */
string ZxingCodeDetection::getType()
{
	return mCodeTypeStr;
}

/**
 * check if the string is UTF string
 * @param bytes         (input) the origin string to be checked
 * @param errorKind     (output) store error status
 * @return               0:is UTF string |others: is not UTF string
 */
static uint8_t checkUtfString(const char* bytes, const char** errorKind)
{
	while (*bytes != '\0') {
	         uint8_t utf8 = *(bytes++);
	         // Switch on the high four bits.
	         switch (utf8 >> 4) {
	         case 0x00:
	         case 0x01:
	         case 0x02:
	         case 0x03:
	         case 0x04:
	         case 0x05:
	         case 0x06:
	         case 0x07:
	             // Bit pattern 0xxx. No need for any extra bytes.
	             break;
	         case 0x08:
	         case 0x09:
	         case 0x0a:
	         case 0x0b:
	         case 0x0f:
	             /*
	              * Bit pattern 10xx or 1111, which are illegal start bytes.
	              * Note: 1111 is valid for normal UTF-8, but not the
	              * modified UTF-8 used here.
	              */
	             *errorKind = "start";
	             return utf8;
	         case 0x0e:
	             // Bit pattern 1110, so there are two additional bytes.
	             utf8 = *(bytes++);
	             if ((utf8 & 0xc0) != 0x80) {
	                 *errorKind = "continuation";
	                 return utf8;
	             }
	             // Fall through to take care of the final byte.
	         case 0x0c:
	         case 0x0d:
	             // Bit pattern 110x, so there is one additional byte.
	             utf8 = *(bytes++);
	             if ((utf8 & 0xc0) != 0x80) {
	                 *errorKind = "continuation";
	                 return utf8;
	             }
	             break;
	         }
	     }
	     return 0;

}







