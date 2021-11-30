#ifndef AV_CODEC_BASE_H
#define AV_CODEC_BASE_H


#include <mutex>

#include "av_data_tools.h"

struct AVCodec;
struct AVCodecContext;

class AVCodecBase
{
public:
	AVCodecBase();
	~AVCodecBase();

	static AVCodecContext* CreateContext(int codec_id,bool is_decode);
	void SetCodecContext(AVCodecContext* codec_ctx);
	int OpenContext(bool is_decode);
	int SetOption(const char* key, const char* value);
	int SetOption(const char* key, const int value);


	virtual AVCodecContext* get_codec_ctx();
	std::shared_ptr<AVParamWarpper> CopyCodecParam();
	std::shared_ptr<AVParametersWarpper>CopyCodecParameters();

	int GetCodecExtraData(uint8_t* buffer, int& size);
	uint8_t* GetSpsData();
	uint8_t* GetPpsData();
	int GetSpsSize();
	int GetPpsSize();
protected:
	std::mutex mtx_;
	AVCodecContext* codec_ctx_ = nullptr;

	int sps_size_ = -1;
	int pps_size_ = -1;
	std::string sps_data_;
	std::string pps_data_;
private:

};

#endif