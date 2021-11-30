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

//#define ITEM_LIST_CONFIG "./media_publisher/conf/configuration.json"
#define GRID_CONFIG "./config/grid_conf.json"

using namespace std;
ElementWidget::ElementWidget(int index, QWidget* parent) : QWidget(parent)
{
    widget_index_ = index;
    this->setAcceptDrops(true);

    QAction* act = menu_.addAction(QString::fromLocal8Bit("cycling"));
    act->setCheckable(true);
    act->setChecked(false);
    act->setEnabled(false);
    QObject::connect(act, &QAction::triggered, this, &ElementWidget::OnSignalSetCycling);

    act = menu_.addAction(QString::fromLocal8Bit("stop"));
    act->setEnabled(false);
    QObject::connect(act, &QAction::triggered, this, &ElementWidget::OnSignalStopPublishing);

    InitUi();
    //is_librtmp_method_ = true;
    startTimer(1);
}

ElementWidget::ElementWidget(QWidget* parent)
{
    QAction* act = menu_.addAction(QString::fromLocal8Bit("cycling"));
    act->setCheckable(false);
    //act->setChecked(false);
    QObject::connect(act, &QAction::triggered, this, &ElementWidget::OnSignalSetCycling);

    act = menu_.addAction(QString::fromLocal8Bit("stop"));
    QObject::connect(act, &QAction::triggered, this, &ElementWidget::OnSignalStopPublishing);

    InitUi();
    //is_librtmp_method_ = true;
    startTimer(1);
}

ElementWidget::~ElementWidget()
{
    DestroyAllHandler();
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
    //if (ev->button() == Qt::RightButton)
    //{
    //    cout << " right button clicked" << endl;
    //}
    cout << "mouse press event" << endl;
    return QWidget::mousePressEvent(ev);
}
void ElementWidget::dragEnterEvent(QDragEnterEvent* ev)
{
    ev->acceptProposedAction();
    return QWidget::dragEnterEvent(ev);
}

void ElementWidget::contextMenuEvent(QContextMenuEvent* ev)
{
    menu_.exec(QCursor::pos());
    ev->accept();
    return QWidget::contextMenuEvent(ev);
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
    this->str_width_ = obj.find("width").value().toString();
    this->str_height_ = obj.find("height").value().toString();

    if (this->str_width_ != "Source width" && this->str_height_ != "Source height")
    {
        this->output_width_ = this->str_width_.toInt();
        this->output_height_ = this->str_height_.toInt();
        this->widget_type_ = WidgetType::WID_TYPE_SCALED;
    }
    else
    {
        this->widget_type_ = WidgetType::WID_TYPE_SOURCE;
    }

    emit SigConfigAndStartHandler();
    //ev->setDropAction(Qt::MoveAction);
    //ev->accept();
    return QWidget::dropEvent(ev);
}


void ElementWidget::OnSignalSetCycling()
{
    if (menu_.actions().at(0)->isChecked())
    {
        if (demux_handler_)
        {
            demux_handler_->set_is_cyling(true);
            mux_handler_->set_is_cyling(true);
            qDebug() << "set cycling enabled";
        }
    }
    else
    {
        if (demux_handler_)
        {
            demux_handler_->set_is_cyling(false);
            mux_handler_->set_is_cyling(false);
            qDebug() << "set cycling disabled";
        }
    }
}

void ElementWidget::OnSignalStopPublishing()
{
    if (demux_handler_)
    {
        demux_handler_->Stop();
        delete demux_handler_;
        demux_handler_ = nullptr;
    }

    if (v_decode_handler_)
    {
        v_decode_handler_->Stop();
        delete v_decode_handler_;
        v_decode_handler_ = nullptr;
    }

    if (v_encode_handler_)
    {
        v_encode_handler_->Stop();
        delete v_encode_handler_;
        v_encode_handler_ = nullptr;
    }

    if (mux_handler_)
    {
        mux_handler_->Stop();
        delete mux_handler_;
        mux_handler_ = nullptr;
    }
    if (view_)
    {
        view_->ResetView();
    }

    //this->resize(this->width() + 1, this->height() + 1);
    //this->resize(this->width() - 1, this->height() - 1);

    this->hide();
    this->show();

    menu_.actions().at(0)->setEnabled(false);
    menu_.actions().at(1)->setEnabled(false);
    //DestroyAllHandler();
    //emit SigWidgetDestroyed(widget_index_);
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

    QAction* act = menu_.actions().at(1);
    act->setEnabled(true);
}

int ElementWidget::ConfigHandlers()
{
	bool ret = false;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //config demux handler
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
    //auto para = demux_handler_->CopyVideoParameters();
    //auto a_para = demux_handler_->CopyAudioParameters();

    shared_ptr<AVParametersWarpper> video_param = demux_handler_->CopyVideoParameters();
    shared_ptr<AVParametersWarpper> audio_param = demux_handler_->CopyAudioParameters();
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

    if (!video_param->para)
    {
        qDebug() << "video_param->para";
        return -1;
    }
    //if (!audio_param->para)
    //{
    //    qDebug() << "audio_param->para";
    //    return -1;
    //}
    ret = v_decode_handler_->Open(video_param->para);
    if (ret != 0)
    {
        qDebug() << "decode_handler_->Open failed";
        return -1;
    }
    v_decode_handler_->SetNeedPlay(true);
    demux_handler_->SetNextVideoHandler(v_decode_handler_);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //reset encode handler
    if (this->widget_type_ == WidgetType::WID_TYPE_SCALED)
    {
        if (!v_encode_handler_)
        {
            v_encode_handler_ = new AVEncodeHandler();
        }
        else
        {
            v_encode_handler_->Stop();
        }
        if (this->output_width_ > 0 && this->output_height_ > 0)
        {
#if 1
            video_param->dst_width_ = output_width_;
            video_param->dst_height_ = output_height_;
            ret = v_encode_handler_->EncodeHandlerInit(video_param);
#else
            ret = v_encode_handler_->EncodeHandlerInit(para->para, output_width_, output_height_, demux_handler_->GetVideoSrcTimebase(), demux_handler_->GetVideoSrcFrameRate());
#endif
        }
        else
        {
#if 1
            video_param->dst_width_ = video_param->para->width;
            video_param->dst_height_ = video_param->para->height;
            ret = v_encode_handler_->EncodeHandlerInit(video_param);
#else
            ret = v_encode_handler_->EncodeHandlerInit(para->para, para->para->width, para->para->height, demux_handler_->GetVideoSrcTimebase(), demux_handler_->GetVideoSrcFrameRate());
#endif
        }

        if (ret != 0)
        {
            qDebug() << "v_encode_handler_->EncodeHandlerInit(video_param) failed";
            return -1;
        }
        v_encode_handler_->SetEncodePause(true);
        v_decode_handler_->SetNextHandler(v_encode_handler_);

        auto codec_param = v_encode_handler_->CopyCodecParameters();

        flv_on_metadata_.has_video_ = true;
        flv_on_metadata_.video_codec_id_ = codec_param->para->codec_id;
        flv_on_metadata_.video_data_rate_ = codec_param->para->bit_rate;
        flv_on_metadata_.frame_rate_ = 25 / 1;
        flv_on_metadata_.width_ = output_width_;
        flv_on_metadata_.height_ = output_height_;

        flv_on_metadata_.has_audio_ = false;
        flv_on_metadata_.audio_data_rate_ = 64;
        flv_on_metadata_.audio_sample_rate_ = 44100;
        flv_on_metadata_.audio_sample_size_ = 16;
        flv_on_metadata_.channels_ = 2;
        flv_on_metadata_.pts_ = 0;
    }
    else
    {
        if (v_encode_handler_)
        {
            v_encode_handler_->Stop();
        }
    }

    if (!a_decode_handler_)
    {
        a_decode_handler_ = new AVDecodeHandler();
    }
    else
    {
        a_decode_handler_->Stop();
    }

    if (audio_param->para)
    {
        ret = a_decode_handler_->Open(audio_param->para);
        if (ret != 0)
        {
            qDebug() << "decode_handler_->Open failed";
            return -1;
        }

        a_decode_handler_->SetNeedPlay(false);
        //demux_handler_->SetNextAudioHandler(a_decode_handler_);

        if (!a_encode_handler_)
        {
            a_encode_handler_ = new AVAudioEncodeHandler();
        }
        else
        {
            a_encode_handler_->Stop();
        }
#if 1
        audio_param->dst_channel_layout_ = audio_param->para->channel_layout;
        audio_param->dst_sample_fmt_ = audio_param->para->format;
        audio_param->dst_sample_rate_ = audio_param->para->sample_rate;
        a_encode_handler_->AudioEncodeHandlerInit(audio_param);
#else
        a_encode_handler_->AudioEncodeHandlerInit(a_para->para, a_para->para->channels, AV_SAMPLE_FMT_S16, 1024, a_para->para->channel_layout);
#endif
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //config view
    if (view_)
    {
        view_->ResetView();
    }
    else if (!view_)
    {
        view_ = IVideoView::CreateView(RenderType::RENDER_TYPE_SDL);
        view_->SetWindowId((void*)this->winId());
    }
    view_->InitView(video_param->para);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //reset push
    if (is_librtmp_method_)
    {
        if (!rtmp_pusher_)
        {
            rtmp_pusher_ = new RtmpPusher(RtmpBaseType::RTMP_BASE_TYPE_PUSH, server_url_.toStdString());
        }
        v_encode_handler_->SetVideoCallback(std::bind(&ElementWidget::VideoEncodeCallback, this, std::placeholders::_1));
        v_encode_handler_->SetVideoCallbackEnable(true);
    }
    else
    {
        int extra_data_size = 0;
        uint8_t extra_data[4096] = { 0 };
        std::shared_ptr<AVParametersWarpper> v_codec_param;
        if (this->widget_type_ == WidgetType::WID_TYPE_SCALED)
        {
            v_encode_handler_->SetVideoCallbackEnable(false);
            v_encode_handler_->SetNextHandler(nullptr);
            v_codec_param = v_encode_handler_->CopyCodecParameter();
            ret = v_encode_handler_->CopyCodecExtraData(extra_data, extra_data_size);
            if (ret != 0)
            {
                qDebug() << "v_encode_handler_->CopyCodecExtraData(extra_data, extra_data_size);";
                return -1;
            }
        }
        else
        {
            v_codec_param = demux_handler_->CopyVideoParameters();
            ret = v_decode_handler_->CopyVCodecExtraData(extra_data, extra_data_size);
            if (ret != 0)
            {
                qDebug() << "v_decode_handler_->CopyCodecExtraData(extra_data, extra_data_size);";
                return -1;
            }
        }

        shared_ptr<AVParametersWarpper> a_encode_param;
        //a_encode_param = a_encode_handler_->CopyCodecParameter();
        a_encode_param = demux_handler_->CopyAudioParameters();
        a_encode_param->para->codec_tag = 0;


        
        if (!mux_handler_)
        {
            mux_handler_ = new AVMuxHandler();
        }
        else
        {
            mux_handler_->Stop();
        }
        //v_codec_param->para;  video_param->para;
        //avcodec_parameters_copy(v_codec_param->para, video_param->para);
        //v_codec_param->para->codec_tag = 0;
        ret = mux_handler_->MuxerInit(server_url_.toStdString(), v_codec_param, a_encode_param, extra_data, extra_data_size);
        //ret = mux_handler_->MuxerInit(server_url_.toStdString(), codec_param->para, codec_param->time_base, nullptr, nullptr, extra_data, extra_data_size);
        if (ret != 0)
        {
            qDebug() << "mux_handler_ init failed";
            return -1;
        }
        ret = mux_handler_->Open();
        if (ret != 0)
        {
            qDebug() << "mux_handler_ open failed";
            return -1;
        }

        if (this->widget_type_ == WidgetType::WID_TYPE_SCALED)
        {
            v_encode_handler_->SetNextHandler(mux_handler_);
        }
        else
        {
            demux_handler_->SetNextVideoHandler(mux_handler_);
        }

        //demux_handler_->SetNextAudioHandler(mux_handler_);
    }
    return 0;
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

    this_thread::sleep_for(50ms);
    
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

    menu_.actions().at(0)->setEnabled(true);
    menu_.actions().at(0)->setChecked(false);
    menu_.actions().at(1)->setEnabled(true);
    return true;
}

void ElementWidget::DemuxVideoCallback(AVPacket* pkt)
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
            //if (IsVideoSeqHeaderNeeded())
            //{
                VideoSequenceHeaderMessage* video_seq_header = new VideoSequenceHeaderMessage(
                    demux_handler_->GetSpsData(),
                    demux_handler_->GetSpsSize(),
                    demux_handler_->GetPpsData(),
                    demux_handler_->GetPpsSize()
                );
                rtmp_pusher_->Post(MessagePayloadType::MESSAGE_PAYLOAD_TYPE_VIDEO_SEQ,
                    video_seq_header, false);
                SetVideoSeqHeaderNeeded(false);
            //}

            NALUStruct* nalu = new NALUStruct(pkt->data, pkt->size);
            nalu->pts_ = AVPublishTime::GetInstance()->GetVideoPts();
            rtmp_pusher_->Post(MessagePayloadType::MESSAGE_PAYLOAD_TYPE_NALU,nalu, false);
        }
    }
    else
    {
        if (mux_handler_)
        {
            AVHandlerPackage* payload = new AVHandlerPackage();
            payload->type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET;
            payload->payload_.packet_ = av_packet_alloc();
            av_packet_ref(payload->payload_.packet_, pkt);
            mux_handler_->Handle(payload);
            av_packet_unref(pkt);
        }
    }
}

void ElementWidget::DemuxAudioCallback(AVPacket* pkt)
{
    if (is_librtmp_method_)
    {
        if (rtmp_pusher_)
        {

        }
    }
    else
    {
        if (mux_handler_)
        {
            AVHandlerPackage* payload = new AVHandlerPackage();
            payload->type_ = AVHandlerPackageType::AVHANDLER_PACKAGE_TYPE_PACKET;
            payload->av_type_ = AVHandlerPackageAVType::AVHANDLER_PACKAGE_AV_TYPE_AUDIO;
            payload->payload_.packet_ = av_packet_alloc();
            av_packet_ref(payload->payload_.packet_, pkt);
            mux_handler_->Handle(payload);
            av_packet_unref(pkt);
        }
    }
}

void ElementWidget::VideoEncodeCallback(AVPacket* v_pkt)
{
    if (IsVideoSeqHeaderNeeded())
    {
        rtmp_pusher_->Post(MessagePayloadType::MESSAGE_PAYLOAD_TYPE_METADATA, &flv_on_metadata_, false);
        this_thread::sleep_for(1ms);
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

void ElementWidget::VideoInfoCallback(AVRational frame_rate)
{
    if (demux_handler_)
    {
        demux_handler_->set_frame_rate(frame_rate);
    }
}

void ElementWidget::AudioInfoCallback(int sample_rate, int nb_sample)
{
    if (demux_handler_)
    {
        demux_handler_->set_sample_rate(sample_rate);
        demux_handler_->set_nb_samples(nb_sample);
    }
}

void ElementWidget::DestroyAllHandler()
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