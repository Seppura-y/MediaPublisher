#include "av_codec_base.h"
#include "av_data_tools.h"

#include <iostream>
extern"C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
}

#ifdef _WIN32
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")
#endif

using namespace std;

static void PrintError(int err)
{
	char buffer[1024] = { 0 };
	av_strerror(err, buffer, sizeof(buffer));
	cout << buffer << endl;
}

#define PRINT_ERR_P(err) if(err != 0) {PrintError(err);return nullptr;}
#define PRINT_ERR_I(err) if(err != 0) {PrintError(err);return -1;}


AVCodecBase::AVCodecBase()
{

}

AVCodecBase::~AVCodecBase()
{
	if (codec_ctx_)
	{
		avcodec_free_context(&codec_ctx_);
	}
}

AVCodecContext* AVCodecBase::CreateContext(int codec_id, bool is_decode)
{
	AVCodec* codec = nullptr;
	if (is_decode)
	{
		codec = avcodec_find_decoder((AVCodecID)codec_id);
	}
	else
	{
		codec = avcodec_find_encoder((AVCodecID)codec_id);
	}
	if (codec == nullptr)
	{
		cout << "find codec failed" << endl;
		return nullptr;
	}

	AVCodecContext* codec_context = avcodec_alloc_context3(codec);
	if (codec_context == nullptr)
	{
		cout << "alloc codec context failed" << endl;
	}

	return codec_context;
}

void AVCodecBase::SetCodecContext(AVCodecContext* codec_ctx)
{
	unique_lock<mutex> lock(mtx_);
	if (codec_ctx_)
	{
		avcodec_free_context(&codec_ctx_);
	}
	codec_ctx_ = codec_ctx;
}

int AVCodecBase::OpenContext(bool is_decode)
{
	unique_lock<mutex> lock(mtx_);
	if (codec_ctx_ == nullptr)
	{
		cout << "OpenContext failed : codec_ctx_ is null" << endl;
		return -1;
	}
	int ret = -1;
	if (!is_decode)
	{
		AVDictionary* dict = nullptr;
		//av_dict_set(&dict, "tune", "zerolatency", 0);
		//av_dict_set(&dict, "preset", "ultrafast", 0);
		ret = avcodec_open2(codec_ctx_, NULL, &dict);
	}
	else
	{
		ret = avcodec_open2(codec_ctx_, NULL, NULL);
	}
	if (ret != 0)
	{
		cout << "avcodec_open2 failed" << endl;
		return -1;
	}
	//if (codec_ctx_ 
	//	&& codec_ctx_->codec_type == AVMEDIA_TYPE_VIDEO 
	//	&& codec_ctx_->codec_id == AV_CODEC_ID_H264 
	//	&& codec_ctx_->extradata && codec_ctx_->extradata_size > 0)
	//{
	//	int data_size = codec_ctx_->extradata_size;
	//	uint8_t* data = codec_ctx_->extradata;
	//	uint8_t num_of_sps;
	//	uint8_t num_of_pps;
	//	uint8_t* sps;
	//	uint8_t* pps;
	//	uint8_t sps_index = -1;
	//	uint8_t pps_index = -1;
	//	sps_size_ = 0;
	//	pps_size_ = 0;
	//	for (int i = 1; i < data_size; i++)
	//	{
	//		if (data[i] == 0x67 && data[i - 2] == 0x00)
	//		{
	//			num_of_sps = data[i - 3] & 0x1f;
	//			uint8_t tmp_size = data[i - 4];
	//			sps = &data[i];
	//			sps_index = sps - data;
	//			do
	//			{
	//				sps_data_.append((const char*)sps, tmp_size);
	//				sps_size_ += tmp_size;
	//				sps = sps + tmp_size;
	//				tmp_size = *(sps + 2);
	//			} while (--num_of_sps);

	//			//num_of_pps = *(sps + 1);
	//			break;
	//		}
	//	}
	//	pps_index = sps_index + sps_size_ +1;
	//	num_of_pps = data[pps_index];
	//	uint8_t tmp_size = data[pps_index + 2];
	//	pps = &data[pps_index + 3];
	//	do
	//	{
	//		pps_data_.append((const char*)pps, tmp_size);
	//		pps_size_ += tmp_size;
	//		pps = pps + tmp_size;
	//		tmp_size = *(pps + 2);
	//	} while (--num_of_pps);
	//}
	
	
	//if (codec_ctx_
	//	&& codec_ctx_->codec_type == AVMEDIA_TYPE_VIDEO
	//	&& codec_ctx_->codec_id == AV_CODEC_ID_H264
	//	&& codec_ctx_->extradata && codec_ctx_->extradata_size > 0)
	//{
	//	int data_size = codec_ctx_->extradata_size;
	//	uint8_t* data = codec_ctx_->extradata;
	//	uint8_t* sps;
	//	uint8_t* pps;

	//	for (int i = 4; i < data_size; i++)
	//	{
	//		if (data[i] == 0x00 && data[i + 1] == 0x00 && data[i + 2] == 0x00 && data[i + 3] == 0x01)
	//		{
	//			sps_size_ = i - 4;
	//			pps_size_ = data_size - i - 4;
	//			pps = &data[i + 4];
	//			break;
	//		}
	//	}
	//	sps = &data[4];
	//	sps_data_.append((const char*)sps, sps_size_);
	//	pps_data_.append((const char*)pps, pps_size_);
	//}

	if (codec_ctx_
		&& codec_ctx_->codec_type == AVMEDIA_TYPE_VIDEO
		&& codec_ctx_->codec_id == AV_CODEC_ID_H264
		&& codec_ctx_->extradata && codec_ctx_->extradata_size > 0)
	{
		int data_size = codec_ctx_->extradata_size;
		uint8_t* data = codec_ctx_->extradata;
		uint8_t* sps = nullptr;
		uint8_t* pps = nullptr;

		for (int i = 0; i < data_size; i++)
		{
			if (data[i] == 0x00 && data[i + 1] == 0x00 && data[i + 2] == 0x00 && data[i + 3] == 0x01)
			{
				if (sps == nullptr)
				{
					i += 4;
					sps = &data[i];
					continue;
				}
				if (sps && pps == nullptr)
				{
					i += 4;
					pps = &data[i];

					sps_size_ = pps - data - 8;
					pps_size_ = data_size - sps_size_ - 8;
					sps_data_.append((const char*)sps, sps_size_);
					pps_data_.append((const char*)pps, pps_size_);
					break;

				}
			}
		}
	}
	return 0;
}


int AVCodecBase::SetOption(const char* key, const char* value)
{
	unique_lock<mutex> lock(mtx_);
	if (codec_ctx_ == nullptr)
	{
		cout << "SetOption failed : codec_ctx_ is null" << endl;
		return -1;
	}
	int ret = av_opt_set(codec_ctx_->priv_data, key, value, 0);
	PRINT_ERR_I(ret);

	return 0;
}


int AVCodecBase::SetOption(const char* key, const int value)
{
	unique_lock<mutex> lock(mtx_);
	if (codec_ctx_ == nullptr)
	{
		cout << "SetOption failed : codec_ctx_ is null " << endl;
		return -1;
	}

	int ret = av_opt_set_int(codec_ctx_->priv_data, key, value, 0);
	PRINT_ERR_I(ret);

	return 0;
}

int AVCodecBase::GetCodecExtraData(uint8_t* buffer,int& size)
{
	unique_lock<mutex> lock(mtx_);
	if (!codec_ctx_)
	{
		cout << "GetCodecExtraData failed : codec_ctx_ is null" << endl;
		return -1;
	}
	if (codec_ctx_->codec_id == AV_CODEC_ID_H264)
	{
		memcpy(buffer, codec_ctx_->extradata, codec_ctx_->extradata_size);
		size = codec_ctx_->extradata_size;
	}
	else
	{
		return -1;
	}
	return 0;
}

shared_ptr<AVParamWarpper> AVCodecBase::CopyCodecParam()
{
	shared_ptr<AVParamWarpper> param;
	unique_lock<mutex> lock(mtx_);
	if (!codec_ctx_)
	{
		cout << "copy codec param failed : codec_ctx_ is null" << endl;
		return param;
	}
	param.reset(AVParamWarpper::Create());
	*param->time_base = codec_ctx_->time_base;
	avcodec_parameters_from_context(param->para, codec_ctx_);
	return param;
}

AVCodecContext* AVCodecBase::get_codec_ctx()
{
	unique_lock<mutex> lock(mtx_);
	return codec_ctx_;
}

uint8_t* AVCodecBase::GetSpsData()
{
	return (uint8_t*)sps_data_.c_str();
}

uint8_t* AVCodecBase::GetPpsData()
{
	return (uint8_t*)pps_data_.c_str();
}

int AVCodecBase::GetSpsSize()
{
	return sps_size_;
}

int AVCodecBase::GetPpsSize()
{
	return pps_size_;
}

