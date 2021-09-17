#pragma once
#include "iav_base_handler.h"
#include "av_muxer.h"

struct MuxInfo
{
	int64_t duraion;
	int64_t last_pts;
	int64_t last_dts;

	int64_t next_pts;
	int64_t next_dts;

	int64_t max_pts;
	int64_t min_pts;

	int64_t ts_delta;
};

class AVMuxHandler : public IAVBaseHandler
{
public:
	int Open();
	int MuxerInit(std::string url, AVCodecParameters* v_param, AVRational* v_timebase, AVCodecParameters* a_param, AVRational* a_timebase, uint8_t* extra_data, int extra_data_size);
	int Open(std::string url,AVCodecParameters* v_param,AVRational* v_timebase,AVCodecParameters* a_param,AVRational* a_timebase,uint8_t* extra_data,int extra_data_size);

	virtual void Handle(AVHandlerPackage* pkg) override;
	void CloseContextIO() { std::unique_lock<std::mutex> lock(mtx_); muxer_.CloseContext(); }

	void set_is_cyling(bool status) { std::unique_lock<std::mutex> lock(mtx_); is_cycling_ = status; }
	bool is_cycling() { std::unique_lock<std::mutex> lock(mtx_); return is_cycling_; }
protected:
	virtual void Loop() override;
private:
	std::string url_{ "" };
	AVMuxer muxer_;
	AVParamWarpper* audio_param_{ nullptr };
	AVParamWarpper* video_param_{ nullptr };
	AVPacketDataList pkt_list_;

	uint8_t* extradata_ = nullptr;
	int extradata_size_ = -1;

	bool is_cycling_ = false;
	bool has_audio_ = false;
	bool has_video_ = false;
	AVProtocolType protocol_type_ = AVProtocolType::AV_PROTOCOL_TYPE_RTMP;

	int64_t start_time_ = -1;
	int64_t last_pts_ = -1;
	int64_t last_dts_ = -1;
	int64_t next_pts_ = -1;
	int64_t next_dts_ = -1;
	bool is_first_packet_ = true;

	MuxInfo mux_info_;
};

