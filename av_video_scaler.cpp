//#include "av_video_scaler.h"
//
//#include <iostream>
//extern"C"
//{
//#include "libavcodec/avcodec.h"
//#include "libswscale/swscale.h"
//}
//#pragma comment(lib,"avcodec.lib")
//#pragma comment(lib,"swscale.lib")
//#pragma comment(lib,"avutil.lib")
//
//using namespace std;
//AVVideoScaler::AVVideoScaler()
//{
//
//}
//AVVideoScaler::~AVVideoScaler()
//{
//
//}
//
//int AVVideoScaler::InitScale()
//{
//    unique_lock<mutex> lock(mtx_);
//    if (sws_ctx_)
//    {
//        sws_freeContext(sws_ctx_);
//        sws_ctx_ = nullptr;
//    }
//    if (output_width_ < 0 || output_height_ < 0 || screen_width_ < 0 || screen_height_ < 0)
//    {
//        LOGERROR("(out_Width_ < 0 || out_Height_ < 0 || g_Width_ < 0 || g_Height_ < 0)")
//            return false;
//    }
//    plinesize_ = new int[AV_NUM_DATA_POINTERS];
//    memset(plinesize_, 0, sizeof(plinesize_));
//    pImgData_ = new unsigned char[screen_width_ * screen_height_ * 4];
//    sws_ctx_ = sws_getCachedContext(sws_ctx_, screen_width_, screen_height_, AV_PIX_FMT_BGRA,
//        out_width_, out_height_, AV_PIX_FMT_YUV420P,
//        SWS_BILINEAR,
//        0, 0, 0);
//    if (!sws_ctx_)
//    {
//        LOGERROR("sws_getCachedContext failed")
//            return false;
//    }
//    return true;
//}
//void AVVideoScaler::SetOutputSize(int width, int height)
//{
//
//}
//AVFrame* AVVideoScaler::ImgDataScale(void* data, int* linesize)
//{
//
//}