#ifndef AV_DECODE_HANDLER_H
#define AV_DECODE_HANDLER_H


#include "iav_base_handler.h"
#include "av_decoder.h"
#include "av_video_scaler.h"

struct AVFrame;
struct AVCodecParameters;
class AVDecodeHandler : public IAVBaseHandler
{
public:
	~AVDecodeHandler();
	int Open(AVCodecParameters* param);
	virtual void Handle(AVHandlerPackage* pkg) override;
	//void GetPlayFrame(AVFrame* frame);
	AVFrame* GetPlayFrame();
	int CopyVCodecExtraData(uint8_t* buffer, int& size);

	void SetScaler(int width, int height, int fmt);

	void SetNeedPlay(bool status);
	void SetNeedScale(bool status);
	void SetNeedResample(bool status);
	void SetStreamCopy(bool status);

	/////////////////////////////////////////////////
	////video
	uint8_t* GetSpsData();
	uint8_t* GetPpsData();
	int GetSpsSize();
	int GetPpsSize();

	void set_audio_info_callback(std::function<void(int, int)> func) { std::lock_guard<std::mutex>lock(mtx_); audio_info_callback_ = func; };
	void set_video_info_callback(std::function<void(AVRational)> func) { std::lock_guard<std::mutex>lock(mtx_); video_info_callback_ = func; };
protected:
	virtual void Loop() override;
	void CreateFrame();
	std::function<void(int, int)> audio_info_callback_ = nullptr;
	std::function<void(AVRational)> video_info_callback_ = nullptr;
protected:

private:
	bool is_need_play_ = false;

	bool is_need_resample_ = false;
	bool is_stream_copy_ = false;
	AVFrame* play_frame_ = nullptr;
	AVFrame* decoded_frame_ = nullptr;

	AVFrame* scaled_frame_ = nullptr;
	AVDecoder decoder_;
	AVPacketDataList pkt_list_;
	AVHandlerPackageAVType av_type_;

	AVRational* frame_rate_ = nullptr;
	int sample_rate_ = 0;
	int nb_samples_ = 0;

	int scaler_width_ = -1;
	int scaler_height_ = -1;
	int scaler_fmt_ = -1;
	bool is_need_scale_ = false;
	AVVideoScaler* video_scaler_ = nullptr;
};

#endif