//#include "av_devide_handler.h"
//extern "C"
//{
//#include <libavformat/avformat.h>
//#include <libavutil/avutil.h>
//}
//
//#ifdef _WIN32
//#pragma comment(lib,"avformat.lib")
//#pragma comment(lib,"avutil.lib")
//#endif
//
//using namespace std;
//
//void AVDevideHandler::Handle(AVHandlerPackage* pkt)
//{
//	if (pkt->type_ == AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET)
//	{
//		if (pkt->av_type_ == AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_AUDIO)
//		{
//			audio_pkt_list_.Push(pkt->payload_.packet_);
//		}
//		else if (pkt->av_type_ == AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_VIDEO)
//		{
//			video_pkt_list_.Push(pkt->payload_.packet_);
//		}
//	}
//}
//
//void AVDevideHandler::Loop()
//{
//	AVPacket* pkt = nullptr;
//	AVHandlerPackage package;
//	while (1)
//	{
//		if (audio_handler_)
//		{
//			pkt = audio_pkt_list_.Pop();
//			package.av_type_ = AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_AUDIO;
//			package.type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET;
//			package.payload_.packet_ = pkt;
//			audio_handler_->Handle(&package);
//			av_packet_unref(pkt);
//		}
//
//		if (video_handler_)
//		{
//			pkt = video_pkt_list_.Pop();
//			package.av_type_ = AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_VIDEO;
//			package.type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET;
//			package.payload_.packet_ = pkt;
//			audio_handler_->Handle(&package);
//			av_packet_unref(pkt);
//		}
//
//		//this_thread::sleep_for(1ms);
//	}
//}