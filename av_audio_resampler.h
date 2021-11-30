#pragma once
#include <stdint.h>
#include <mutex>
#include <vector>


#include "av_data_tools.h"

//#include "libavutil/samplefmt.h"

typedef struct AudioResamplerParams
{
    enum AVSampleFormat src_sample_fmt;
    enum AVSampleFormat dst_sample_fmt;
    int src_sample_rate = 0;
    int dst_sample_rate = 0;
    uint64_t src_channel_layout = 0;
    uint64_t dst_channel_layout = 0;
}AudioResamplerParams;

struct AVFrame;
struct AVAudioFifo;
struct SwrContext;
class AVAudioResampler
{
public:
	AVAudioResampler() {};
	~AVAudioResampler() {};

    int AVAudioResamplerInit(const AudioResamplerParams& param);
    std::shared_ptr<AVFrame> GetOneResampledFrame(const int desired_size);
    std::shared_ptr<AVFrame> ReceiveResampledFrame(const int desired_size);
    int ReceiveResampledFrames(std::vector<std::shared_ptr<AVFrame>>& vec, int desired_size);

    int SendRawPcmData(uint8_t* raw_pcm, int size);
    int SendAudioFrame(AVFrame* frame);
protected:
    std::shared_ptr<AVFrame> OutputFrameAlloc(const int nb_samples);
private:
    std::mutex mtx_;

    AudioResamplerParams resampler_params_;

    AVAudioFifo* audio_samples_fifo_{ nullptr };
    SwrContext* swr_ctx_{ nullptr };

    int64_t start_pts_ = -1;
    int64_t current_pts_ = -1;
    int64_t total_duration_ = -1;

    int64_t src_channels_ = -1;
    int64_t src_nb_samples_ = -1;
    int64_t dst_channels_ = -1;
    int64_t dst_nb_samples_ = -1;
    int64_t max_dst_nb_samples_ = -1;
    int dst_linesize_ = -1;

    bool is_audio_fifo_only_ = false;
    bool is_flushed_ = false;
    bool is_init_ = false;

    uint8_t** resampled_buffer_ = nullptr;


    int InitResampledBuffer();
};

