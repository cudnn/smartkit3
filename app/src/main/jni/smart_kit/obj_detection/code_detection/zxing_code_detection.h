#ifndef ZXING_CODE_DETECTION_HH_
#define ZXING_CODE_DETECTION_HH_

#include <iostream>
#include <string>
class ZxingCodeDetection{

public:
	ZxingCodeDetection();
	~ZxingCodeDetection();

	bool                     detect(unsigned char *yuv420sp, int height, int width,
			                        int roix, int roiy, int roih, int roiw, bool single);
	std::string              getType();//get code information Type:URL emailaddress etc.
	std::string              getDecodeStr();//get QRCode decode string
	int                      getFormat();//get barcode format: QRCode or Barcode
	int                      getCodePositon(int *pos, int len);//get code position

private:
	void                     parseResult();

private:
    std::string              mDecodeStr; //code decoder content
    std::string              mCodeTypeStr;
    int                      mCodeFormat;//code format qrcode/ barcode
    int                      mCodePositon[41];//qrcode Coordinate points
};
#endif

