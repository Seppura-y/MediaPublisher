#ifndef AV_BS_FILTER_H
#define AV_BS_FILTER_H

#include <mutex>
struct AVBSFContext;
struct AVCodecParameters;
struct AVPacket;
struct AVFrame;
class AVBSFilter
{
public:
	AVBSFilter();
	~AVBSFilter();
	static AVBSFContext* CreateBSFContext(AVCodecParameters* param);
	int SetBSFContext(AVBSFContext* ctx);
	int Send(AVPacket* pkt);
	int Recv(AVPacket* pkt);

protected:
	std::mutex mtx_;
private:
	AVBSFContext* bsf_ctx_ = nullptr;

};

#endif
