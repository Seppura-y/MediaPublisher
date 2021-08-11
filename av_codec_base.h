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
	int OpenContext();
	int SetOption(const char* key, const char* value);
	int SetOption(const char* key, const int value);

	virtual AVCodecContext* get_codec_ctx();
	int GetCodecExtraData(uint8_t* buffer,int& size);
	std::shared_ptr<AVParamWarpper> CopyCodecParam();
protected:
	std::mutex mtx_;
	AVCodecContext* codec_ctx_ = nullptr;
private:

};

#endif