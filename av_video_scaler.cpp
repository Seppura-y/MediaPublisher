#include "av_video_scaler.h"
#include "av_data_tools.h"

#include <iostream>
extern"C"
{
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avutil.h"
}
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"swscale.lib")
#pragma comment(lib,"avutil.lib")

using namespace std;

static void PrintError(int err)
{
    char buffer[1024] = { 0 };
    av_strerror(err, buffer, sizeof(buffer));
    cout << buffer << endl;
}


AVVideoScaler::AVVideoScaler()
{
    in_linesize_ = new int[AV_NUM_DATA_POINTERS];
    out_linesize_ = new int[AV_NUM_DATA_POINTERS];
}
AVVideoScaler::~AVVideoScaler()
{
    delete in_linesize_;
    delete out_linesize_;
    in_linesize_ = nullptr;
    out_linesize_ = nullptr;
}

int AVVideoScaler::InitScale(int in_fmt, int out_fmt)
{
    unique_lock<mutex> lock(mtx_);
    if (sws_ctx_)
    {
        sws_freeContext(sws_ctx_);
        sws_ctx_ = nullptr;
    }
    if (output_width_ < 0 || output_height_ < 0 || input_width_ < 0 || input_height_ < 0)
    {
        LOGERROR("(out_Width_ < 0 || out_Height_ < 0 || input_width_ < 0 || input_height_ < 0)")
            return false;
    }

    sws_ctx_ = sws_getCachedContext(sws_ctx_, input_width_, input_height_, (AVPixelFormat)in_fmt,
        output_width_, output_height_, (AVPixelFormat)out_fmt,
        SWS_BILINEAR,
        0, 0, 0);
    if (!sws_ctx_)
    {
        LOGERROR("sws_getCachedContext failed")
            return false;
    }
    return 0;
}

void AVVideoScaler::SetDimension(int input_width, int input_height, int output_width, int output_height)
{
    unique_lock<mutex> lock(mtx_);
    input_width_ = input_width;
    input_height_ = input_height;
    output_width_ = output_width;
    output_height_ = output_height;
}

int AVVideoScaler::FrameScale(AVFrame* input, AVFrame* output)
{
    unique_lock<mutex> lock(mtx_);
    if (!sws_ctx_)
    {
        LOGERROR("sws_ctx_ is null , frame scale failed")
        return -1;
    }

    if (input_width_ < 0 || input_height_ < 0 || output_width_ < 0 || output_height_ < 0)
    {
        LOGERROR("input_width_ < 0 || input_height_ < 0 || output_width_ < 0 || output_height_ < 0)")
        return -1;
    }
    int ret = sws_scale(sws_ctx_, input->data, input->linesize, 0, input_height_, output->data, output->linesize);
    if (ret < 0)
    {
        PrintError(ret);
        return -1;
    }

    return 0;
}
