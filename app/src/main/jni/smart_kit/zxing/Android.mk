LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
#LOCAL_ARM_MODE := arm

#LOCAL_C_INCLUDES += $(LOCAL_PATH)/zxing $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/zxing $(LOCAL_PATH) ${SMART_DIR}/libconv/iconv-android/include
#LOCAL_CFLAGS += -std=gnu++11
#LOCAL_CFLAGS += -std=c++11
#MY_CPP_LIST += $(wildcard $(LOCAL_PATH)/*.cpp) $(wildcard $(LOCAL_PATH)/zxing/*.cpp)  $(wildcard $(LOCAL_PATH)/*.c) $(wildcard $(LOCAL_PATH)/zxing/*.c)
#LOCAL_SRC_FILES := $(MY_CPP_LIST:$(LOCAL_PATH)/%=%)

LOCAL_SRC_FILES += zxing/BarcodeFormat.cpp zxing/Binarizer.cpp zxing/BinaryBitmap.cpp zxing/ChecksumException.cpp zxing/DecodeHints.cpp  \
                     zxing/Exception.cpp      zxing/FormatException.cpp zxing/ImageReaderSource.cpp zxing/InvertedLuminanceSource.cpp zxing/LuminanceSource.cpp \
                     zxing/MultiFormatReader.cpp zxing/Reader.cpp zxing/Result.cpp zxing/ResultIO.cpp zxing/ResultPoint.cpp zxing/ResultPointCallback.cpp
#qrcode 
LOCAL_SRC_FILES += zxing/qrcode/ErrorCorrectionLevel.cpp zxing/qrcode/FormatInformation.cpp zxing/qrcode/QRCodeReader.cpp zxing/qrcode/Version.cpp  \
                      zxing/qrcode/decoder/BitMatrixParser.cpp zxing/qrcode/decoder/DataBlock.cpp zxing/qrcode/decoder/DataMask.cpp  \
                     zxing/qrcode/decoder/DecodedBitStreamParser.cpp  zxing/qrcode/decoder/Decoder.cpp  zxing/qrcode/decoder/Mode.cpp  \
                      zxing/qrcode/detector/AlignmentPattern.cpp  zxing/qrcode/detector/AlignmentPatternFinder.cpp  zxing/qrcode/detector/Detector.cpp  \
                      zxing/qrcode/detector/FinderPattern.cpp  zxing/qrcode/detector/FinderPatternFinder.cpp  zxing/qrcode/detector/FinderPatternInfo.cpp 
                      
LOCAL_SRC_FILES += zxing/common/BitArray.cpp  zxing/common/BitArrayIO.cpp  zxing/common/BitMatrix.cpp   zxing/common/BitSource.cpp  \
                   zxing/common/CharacterSetECI.cpp   zxing/common/DecoderResult.cpp    zxing/common/DetectorResult.cpp   zxing/common/GlobalHistogramBinarizer.cpp  \
                   zxing/common/GreyscaleLuminanceSource.cpp zxing/common/GreyscaleRotatedLuminanceSource.cpp  zxing/common/GridSampler.cpp  \
                   zxing/common/HybridBinarizer.cpp  zxing/common/IllegalArgumentException.cpp  zxing/common/PerspectiveTransform.cpp zxing/common/Str.cpp  \
                   zxing/common/StringUtils.cpp  \
                   zxing/common/reedsolomon/GenericGF.cpp zxing/common/reedsolomon/GenericGFPoly.cpp  zxing/common/reedsolomon/ReedSolomonDecoder.cpp   \
                   zxing/common/reedsolomon/ReedSolomonException.cpp     
#oned
LOCAL_SRC_FILES += zxing/oned/CodaBarReader.cpp zxing/oned/Code128Reader.cpp zxing/oned/Code39Reader.cpp  \
                   zxing/oned/Code93Reader.cpp zxing/oned/EAN13Reader.cpp zxing/oned/EAN8Reader.cpp  \
                   zxing/oned/ITFReader.cpp  zxing/oned/MultiFormatOneDReader.cpp  zxing/oned/MultiFormatUPCEANReader.cpp  \
                   zxing/oned/OneDReader.cpp  zxing/oned/OneDResultPoint.cpp zxing/oned/UPCAReader.cpp  \
                   zxing/oned/UPCEANReader.cpp  zxing/oned/UPCEReader.cpp
#result
#LOCAL_SRC_FILES += zxing/resultParser/EmailParsedResult.cpp zxing/resultParser/URIResultParser.cpp  zxing/resultParser/strTool.cpp  \
#                   zxing/resultParser/ResultParserHandle.cpp
LOCAL_SRC_FILES +=  zxing/resultParser/strTool.cpp   zxing/resultParser/WebsiteTypeHandle.cpp zxing/resultParser/ResultParserHandle.cpp zxing/resultParser/EmailParsedResult.cpp   \
                    zxing/resultParser/URIResultParser.cpp  zxing/resultParser/TelParsedResult.cpp  \
                    zxing/resultParser/TelResultParser.cpp 

#LOCAL_SRC_FILES += zxing/resultParser/CodaBarReader.cpp zxing/oned/Code128Reader.cpp zxing/oned/Code39Reader.cpp  \
#                   zxing/oned/Code93Reader.cpp zxing/oned/EAN13Reader.cpp zxing/oned/EAN8Reader.cpp  \
#                   zxing/oned/ITFReader.cpp  zxing/oned/MultiFormatOneDReader.cpp  zxing/oned/MultiFormatUPCEANReader.cpp  \
#                   zxing/oned/OneDReader.cpp  zxing/oned/OneDResultPoint.cpp zxing/oned/UPCAReader.cpp  \
#                   zxing/oned/UPCEANReader.cpp  zxing/oned/UPCEReader.cpp
         
                   

                   
                            
LOCAL_STATIC_LIBRARIES += libiconv 
LOCAL_MODULE    := ivc_zxing

#LOCAL_WHOLE_STATIC_LIBRARIES += android_support


include $(BUILD_STATIC_LIBRARY)
#$(call import-module, android/support)
include ${SMART_DIR}/libconv/Android.mk


