#include "element_widget.h"
#include "item_listwidget.h"
#include "camera_menu.h"
#include "item_set_dialog.h"
#include "avtimebase.h"

extern"C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#ifdef _WIN32
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avcodec.lib")
#pragma comment(lib,"avutil.lib")
#endif

#include <thread>

#include <QPainter>
#include <QStyleOption>
#include <QAction>
#include <QString>
#include <QDebug>
#include <QListWidget>
#include <QTabWidget>
#include <QMimeData>
#include <QJsonObject>
#include <QJsonDocument>
#include <QPixMap>

#define ITEM_LIST_CONFIG "./media_publisher/conf/configuration.json"
#define GRID_CONFIG "./config/grid_conf.json"

using namespace std;
ElementWidget::ElementWidget(int index, QWidget* parent) : QWidget(parent)
{
    widget_index_ = index;
    this->setAcceptDrops(true);

    QAction* act = menu_.addAction(QString::fromLocal8Bit("set"));
    QObject::connect(act, &QAction::triggered, this, &ElementWidget::OnSigalSet);

    InitUi();
    is_librtmp_method_ = true;
    startTimer(1);
}

ElementWidget::ElementWidget(QWidget* parent)
{
    
}

ElementWidget::~ElementWidget()
{
    DestoryAllHandler();
}

void ElementWidget::InitUi()
{
    QObject::connect(this, &ElementWidget::SigConfigAndStartHandler, this, &ElementWidget::OnConfigAndStartHandler);
}

void ElementWidget::timerEvent(QTimerEvent* ev)
{
    if (v_decode_handler_ && view_)
    {
        AVFrame* frame = v_decode_handler_->GetPlayFrame();
        if (!frame || !frame->buf[0] || frame->linesize[0] <= 0)
        {
            //av_frame_free(&frame);
            return;
        }
        view_->DrawFrame(frame);
        //av_frame_free(&frame);
        av_frame_unref(frame);
    }
}

void ElementWidget::paintEvent(QPaintEvent* ev)
{
    QStyleOption opt;
    opt.init(this);

    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void ElementWidget::mousePressEvent(QMouseEvent* ev)
{
    return QWidget::mousePressEvent(ev);
}
void ElementWidget::dragEnterEvent(QDragEnterEvent* ev)
{
    ev->acceptProposedAction();
}

void ElementWidget::contextMenuEvent(QContextMenuEvent* ev)
{
    menu_.exec(QCursor::pos());
    ev->accept();
}

void ElementWidget::dropEvent(QDropEvent* ev)
{
    int itemIndex = ((ItemListWidget*)ev->source())->currentIndex().row();
    CameraMenu::ItemListType itemType = (CameraMenu::ItemListType)((ItemListWidget*)ev->source())->item_type_;

    QByteArray arr = ev->mimeData()->data("application/json");
    qDebug() << arr;
    QJsonParseError err;
    QJsonDocument doc(QJsonDocument::fromJson(arr, &err));
    QJsonObject obj = doc.object();

    this->name_ = obj.find("name").value().toString();
    this->url_ = obj.find("url").value().toString();
    this->sub_url_ = obj.find("sub_url").value().toString();
    this->server_url_ = obj.find("server_url").value().toString();
    this->item_type_ = itemType;

    emit SigConfigAndStartHandler();
    ev->setDropAction(Qt::MoveAction);
    ev->accept();
}


void ElementWidget::OnSigalSet()
{
    qDebug() << "on signal set";
}



int ElementWidget::ConfigHandlers()
{
	bool ret = false;
    //output_width_ = this->width();
    //output_height_ = this->height();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //config demux handler according to avsource
    if (!demux_handler_)
    {
        demux_handler_ = new AVDemuxHandler();
    }
    else
    {
        demux_handler_->Stop();
    }
    ret = demux_handler_->OpenAVSource(url_.toStdString().c_str());
    if (!ret)
    {
        qDebug() << "demux_handler_->OpenAVSource failed";
        return -1;
    }

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	//reset decode handler according to codec parameters
	if (!v_decode_handler_)
	{
        v_decode_handler_ = new AVDecodeHandler();
	}
	else
	{
        v_decode_handler_->Stop();
	}
    auto para = demux_handler_->CopyVideoParameters();
    if (!para)
    {
        qDebug() << "auto para = demux_handler_->CopyVideoParameters() para is null";
        return -1;
    }
    ret = v_decode_handler_->Open(para->para);
	if (ret != 0)
	{
		qDebug() << "decode_handler_->Open failed";
		return -1;
	}
    v_decode_handler_->SetNeedPlay(true);
    demux_handler_->SetNextHandler(v_decode_handler_);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //reset encode handler
    if (!v_encode_handler_)
    {
        v_encode_handler_ = new AVEncodeHandler();
    }
    else
    {
        v_encode_handler_->Stop();
    }
    ret = v_encode_handler_->EncoderInit(para->para->width, para->para->height);
    if (ret != 0)
    {
        qDebug() << "encode_th_->Open(inWidth, inHeight) failed";
        return -1;
    }
    v_encode_handler_->SetEncodePause(true);
    v_decode_handler_->SetNextHandler(v_encode_handler_);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //config view
    if (view_)
    {
        view_->ResetView();
    }
    else if (!view_)
    {
        view_ = IVideoView::CreateView(RenderType::RENDER_TYPE_SDL);
    }
    view_->SetWindowId((void*)this->winId());
    view_->InitView(para->para);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //reset mux handler(ffmpeg) or rtmp_pusher(librtmp) according to codec parameters
    if (is_librtmp_method_)
    {
        if (!rtmp_pusher_)
        {
            rtmp_pusher_ = new RtmpPusher(RtmpBaseType::RTMP_BASE_TYPE_PUSH, server_url_.toStdString());
        }
        v_encode_handler_->SetPushCallbackFunction(std::bind(&ElementWidget::VideoEncodeCallback, this, std::placeholders::_1));
        v_encode_handler_->SetCallbackEnable(true);
    }
    else
    {
        v_encode_handler_->SetCallbackEnable(false);
        v_encode_handler_->SetNextHandler(nullptr);
        if (!mux_handler_)
        {
            mux_handler_ = new AVMuxHandler();
        }
        else
        {
            mux_handler_->Stop();
        }
        auto codec_param = demux_handler_->CopyVideoParameters();
        int extra_data_size = 0;
        uint8_t extra_data[4096] = { 0 };
        ret = demux_handler_->CopyCodecExtraData(extra_data, extra_data_size);

        ret = mux_handler_->Open(server_url_.toStdString(), codec_param->para, codec_param->time_base, nullptr, nullptr, extra_data, extra_data_size);
        if (ret != 0)
        {
            qDebug() << "mux_handler_ open failed";
            return -1;
        }

        v_encode_handler_->SetNextHandler(mux_handler_);
        //demux_handler_->SetNextHandler(mux_handler_);
    }
    return 0;
	//capture_handler_->Start();
}

int ElementWidget::StartHandle()
{
    if (!is_librtmp_method_)
    {
        if (mux_handler_ && mux_handler_->IsExit())
        {
            mux_handler_->Start();
        }
    }
    
    if (v_encode_handler_)
    {
        v_encode_handler_->SetEncodePause(false);
        if (v_encode_handler_->IsExit())
        {
            v_encode_handler_->Start();
        }
    }

    if (v_decode_handler_)
    {
        if (v_decode_handler_->IsExit())
        {
            v_decode_handler_->Start();
        }
    }

    if (demux_handler_)
    {
        if (demux_handler_->IsExit())
        {
            demux_handler_->Start();
        }
    }
    return true;
}

void ElementWidget::DemuxCallback(AVPacket* pkt)
{
    if (pkt->stream_index == demux_handler_->GetVideoIndex())
    {
        if (v_decode_handler_)
        {
            AVHandlerPackage* payload = new AVHandlerPackage();
            payload->type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET;
            payload->payload_.packet_ = av_packet_alloc();
            av_packet_ref(payload->payload_.packet_, pkt);
            v_decode_handler_->Handle(payload);
            av_packet_unref(pkt);
        }
    }

    if (is_librtmp_method_)
    {
        if (rtmp_pusher_)
        {
            if (IsVideoSeqHeaderNeeded())
            {
                VideoSequenceHeaderMessage* video_seq_header = new VideoSequenceHeaderMessage(
                    demux_handler_->GetSpsData(),
                    demux_handler_->GetSpsSize(),
                    demux_handler_->GetPpsData(),
                    demux_handler_->GetPpsSize()
                );
                rtmp_pusher_->Post(MessagePayloadType::MESSAGE_PAYLOAD_TYPE_VIDEO_SEQ,
                    video_seq_header, false);
                SetVideoSeqHeaderNeeded(false);
            }

            NALUStruct* nalu = new NALUStruct(pkt->data, pkt->size);
            nalu->pts_ = AVPublishTime::GetInstance()->GetVideoPts();
            rtmp_pusher_->Post(MessagePayloadType::MESSAGE_PAYLOAD_TYPE_NALU,nalu, false);
        }
    }
    else
    {
        //if (mux_handler_)
        //{
        //    AVHandlerPackage* payload = new AVHandlerPackage();
        //    payload->type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET;
        //    payload->payload_.packet_ = av_packet_alloc();
        //    av_packet_ref(payload->payload_.packet_, pkt);
        //    mux_handler_->Handle(payload);
        //    av_packet_unref(pkt);
        //}
    }
}

void ElementWidget::VideoEncodeCallback(AVPacket* v_pkt)
{
    if (IsVideoSeqHeaderNeeded())
    {
        VideoSequenceHeaderMessage* video_seq_header = new VideoSequenceHeaderMessage(
            v_encode_handler_->GetSpsData(),
            v_encode_handler_->GetSpsSize(),
            v_encode_handler_->GetPpsData(),
            v_encode_handler_->GetPpsSize()
        );
        //video_seq_header->width_ = output_width_;
        //video_seq_header->height_ = output_height_;

        rtmp_pusher_->Post(MessagePayloadType::MESSAGE_PAYLOAD_TYPE_VIDEO_SEQ,
            video_seq_header, false);

        SetVideoSeqHeaderNeeded(false);
    }

    NALUStruct* nalu = new NALUStruct(v_pkt->data, v_pkt->size);
    //nalu->pts_ = AVPublishTime::GetInstance()->GetVideoPts();
    rtmp_pusher_->Post(MessagePayloadType::MESSAGE_PAYLOAD_TYPE_NALU,
        nalu, false);
}

void ElementWidget::OnConfigAndStartHandler()
{
    int ret = ConfigHandlers();
    if (ret != 0)
    {
        qDebug() << "ConfigHandlers() failed";
        return;
    }
    StartHandle();
}

void ElementWidget::DestoryAllHandler()
{
    if (rtmp_pusher_)
    {
        rtmp_pusher_->Stop();
        rtmp_pusher_ = nullptr;
    }
    if (mux_handler_)
    {
        mux_handler_->Stop();
        mux_handler_ = nullptr;
    }
    if (v_decode_handler_)
    {
        v_decode_handler_->Stop();
        v_decode_handler_ = nullptr;
    }
    if (demux_handler_)
    {
        demux_handler_->Stop();
        demux_handler_ = nullptr;
    }
    if (view_)
    {
        view_->DestoryView();
        view_ = nullptr;
    }
}

bool ElementWidget::IsVideoSeqHeaderNeeded()
{
    return is_video_seq_header_needed_;
}

bool ElementWidget::IsAudioSeqHeaderNeeded()
{
    return is_audio_seq_header_needed_;
}

void ElementWidget::SetVideoSeqHeaderNeeded(bool status)
{
    is_video_seq_header_needed_ = status;
}

void ElementWidget::SetAudioSeqHeaderNeeded(bool status)
{
    is_audio_seq_header_needed_ = status;
}