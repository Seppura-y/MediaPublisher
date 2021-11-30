#pragma once
#include "iav_base_handler.h"
#include "av_muxer.h"

struct AVRational;

struct MuxInfo
{
	int64_t last_pts;
	int64_t last_dts;
	int64_t next_pts;
	int64_t next_dts;

	int64_t ts_delta;

	int64_t duration;

	int vs_index;
	int64_t vs_max_pts;
	int64_t vs_min_pts;
	uint64_t vs_nb_packets;

	int as_index;
	int64_t as_max_pts;
	int64_t as_min_pts;
	uint64_t as_nb_packets;

	AVRational* time_base;
	//AVRational* a_time_base;
};

class AVMuxHandler : public IAVBaseHandler
{
public:
	int Open();
	int MuxerInit(std::string url, std::shared_ptr<AVParametersWarpper>v_para, std::shared_ptr<AVParametersWarpper>a_para, uint8_t* extra_data, int extra_data_size);
	int MuxerInit(std::string url, AVCodecParameters* v_param, AVRational* v_timebase, AVCodecParameters* a_param, AVRational* a_timebase, uint8_t* extra_data, int extra_data_size);
	//int Open(std::string url,AVCodecParameters* v_param,AVRational* v_timebase,AVCodecParameters* a_param,AVRational* a_timebase,uint8_t* extra_data,int extra_data_size);
	//int MuxerInit(std::string url, AVCodecParameters* v_param, AVRational* v_timebase, AVCodecParameters* a_param, AVRational* a_timebase, uint8_t* extra_data, int extra_data_size,int v_idx,int a_idx);

	virtual void Handle(AVHandlerPackage* pkg) override;
	void CloseContextIO() { std::unique_lock<std::mutex> lock(mtx_); muxer_.CloseContext(); }

	void set_is_cyling(bool status) { std::unique_lock<std::mutex> lock(mtx_); is_cycling_ = status; }
	bool is_cycling() { std::unique_lock<std::mutex> lock(mtx_); return is_cycling_; }
protected:
	virtual void Loop() override;
private:
	std::string url_{ "" };
	AVMuxer muxer_;
	//AVParamWarpper* audio_param_{ nullptr };
	//AVParamWarpper* video_param_{ nullptr };
	AVParametersWarpper* audio_para_{ nullptr };
	AVParametersWarpper* video_para_{ nullptr };
	AVPacketDataList pkt_list_;

	uint8_t* extradata_ = nullptr;
	int extradata_size_ = -1;

	bool is_cycling_ = false;
	bool has_audio_ = false;
	bool has_video_ = false;
	AVProtocolType protocol_type_ = AVProtocolType::AV_PROTOCOL_TYPE_RTMP;

	int video_stream_index_ = -1;
	int audio_stream_index_ = -1;
	int64_t start_time_ = -1;
	int64_t last_pts_ = -1;
	int64_t last_dts_ = -1;
	int64_t next_pts_ = -1;
	int64_t next_dts_ = -1;
	bool is_first_packet_ = true;

	int64_t total_duration_ = -1;
	int64_t video_duration_ = -1;
	int64_t audio_duration_ = -1;

	AVRational* video_timebase_;
	AVRational* audio_timebase_;

	MuxInfo mux_info_;
};

