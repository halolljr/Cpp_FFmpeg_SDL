#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include "source.h"
#include "functions.h"
//@file		:	$itemname$
//@author	:	jiarong Liang
//@date		:	$time$
//@brief	:	
const char* LOCALFILE = "src.mp4";
const char* OUTPUTFILE_A = "dst.aac";
//const char* OUTPUTFILE_A = "dst.aac";
const char* OUTPUTFILE_V = "dst.h264";
const char* REMUX_FILE = "dst.flv";
const char* CUT_FILE = "cur_part.mp4";
//����Ϊ��λ
const int START_TIME = 15;
const int END_TIME = 360;
const char* INPUTFILE_A = "srcc.mp4";
const char* INPUTFILE_V = "srccc.mp4";
const char* OUTPUTFILE = "output.mp4";
const char* CODEC_FILE_V = "oo.h264";
const char* CODEC_NAME = "libx264";
const char* CODEC_FILE_A = "oo.aac";
//const int COUNT_PIX = 5;
extern "C" {

#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avdevice.lib")
#pragma comment (lib, "avfilter.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "swresample.lib")
#pragma comment (lib, "swscale.lib") 
#include <libavutil/log.h>
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
}


void av_logs() {

	//-------2024.9.2-------

	av_log_set_level(AV_LOG_DEBUG);
	av_log(NULL, AV_LOG_INFO, "Hello World...\n");

	//��FFmpeg����API��������ֱ�ӵ���
	/*int res_ = avpriv_io_move("t1.txt", "t2.txt");
	if (res_ < 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to move 't1.txt' to 't2.txt'\n");
		return;
	}
	int res_ = avpriv_io_delete("test.txt");
	if (res_ < 0) {
		av_log(NULL, AV_LOG_ERROR, "Failed to delete 'test.txt'\n");
		return;
	}*/

}

void av_abstract_audio() {

	//-------2024.9.3-------
	av_log_set_level(AV_LOG_DEBUG);
	//1.�򿪶�ý���ļ�

	AVFormatContext* pFmtCtx = nullptr;
	int res_1 = -1;
	res_1 = avformat_open_input(&pFmtCtx, LOCALFILE, NULL, NULL);//��ϸ�Ķ�avformat_open_input�������׺���
	if (res_1 < 0) {
		char errbuf[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuf, sizeof(errbuf));
		av_log(pFmtCtx, AV_LOG_ERROR, "Error:%s\n", errbuf);//��������ת��Ϊ������Ϣ����
		//av_log(NULL, AV_LOG_ERROR, "Error: %s\n", av_err2str(res_1)); av_err2str()������ĳЩFFmpeg�汾������
		exit(-1);
	}
	//2.�Ӷ�ý���ļ����ҵ���Ƶ��

	auto index = av_find_best_stream(pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);//��ϸ�Ķ�av_find_best_stream�������׺���
	if (index < 0) {
		av_log(pFmtCtx, AV_LOG_ERROR, "Error:Don't include any audio stream...\n");
		if (pFmtCtx)
			avformat_close_input(&pFmtCtx);
		exit(-1);
	}

	//3.��������ļ���������
	AVFormatContext* oFmtCtx = nullptr;
	oFmtCtx = avformat_alloc_context();//��ϸ�Ķ�avformat_alloc_context()�������׺���
	if (!oFmtCtx) {
		av_log(NULL, AV_LOG_ERROR, "Error:Memory apply failed...\n");
		if (pFmtCtx)
			avformat_close_input(&pFmtCtx);
		exit(-1);
	}
	std::string dst = OUTPUTFILE_A;//�ɻ��-------
	const AVOutputFormat* oFmt = NULL;
	oFmt = av_guess_format(nullptr, dst.c_str(), NULL);//ͨ��Ŀ���ļ��ҵ�������ļ���һЩ����---�ɻ��
	oFmtCtx->oformat = oFmt;

	//4.Ϊ����ļ�����һ���µ���Ƶ��
	AVStream* outStream = avformat_new_stream(oFmtCtx, nullptr);

	//5.��������ļ�����Ƶ����--���ﲻ���ı�
	avcodec_parameters_copy(outStream->codecpar, pFmtCtx->streams[index]->codecpar);
	outStream->codecpar->codec_tag = 0;	//����Ϊ0->���ݶ�ý���ļ��Զ����ñ������

	//6.д��ý���ļ�ͷ������ļ���������
	auto res_2 = avio_open2(&oFmtCtx->pb, dst.c_str(), AVIO_FLAG_WRITE, NULL, nullptr);//��Ŀ���ļ�������ļ���
	if (res_2 < 0) {
		char errbuf[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_2, errbuf, sizeof(errbuf));
		av_log(oFmtCtx, AV_LOG_ERROR, "Error:%s\n", errbuf);
		if (pFmtCtx) {
			avformat_close_input(&pFmtCtx);
			pFmtCtx = NULL;
		}
		if (oFmtCtx) {
			avformat_free_context(oFmtCtx);
			oFmtCtx = NULL;
		}
		exit(-1);
	}
	//д��ͷ��Ϣ
	auto res_3 = avformat_write_header(oFmtCtx, nullptr);
	if (res_3 < 0) {
		char errbuf[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_3, errbuf, sizeof(errbuf));
		av_log(oFmtCtx, AV_LOG_ERROR, "Error:%s\n", errbuf);
		if (pFmtCtx) {
			avformat_close_input(&pFmtCtx);
			pFmtCtx = NULL;
		}
		if (oFmtCtx) {
			avformat_free_context(oFmtCtx);
			oFmtCtx = NULL;
		}
		exit(-1);
	}

	//7.��Դ��ý���ļ��ж�����Ƶ���ݵ�Ŀ���ļ���
	AVPacket pkt;
	while (av_read_frame(pFmtCtx, &pkt) >= 0) {
		if (pkt.stream_index == index) {//ȡ�������Ƶ��
			//�ı�ʱ���--������Ƶ��˵��dts==pts��
			pkt.pts = av_rescale_q_rnd(pkt.pts, pFmtCtx->streams[index]->time_base, outStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.dts = pkt.pts;
			pkt.duration = av_rescale_q(pkt.duration, pFmtCtx->streams[index]->time_base, outStream->time_base);
			pkt.stream_index = 0;	//ֻ��һ·��Ƶ
			pkt.pos = -1;	//�Զ����м���
			av_interleaved_write_frame(oFmtCtx, &pkt);
			av_packet_unref(&pkt);	//����pkt�����ô���
		}
	}

	//8.д��ý���ļ�β�͵�����ļ���������
	av_write_trailer(oFmtCtx);

	//9.�ͷ���Դ
	if (pFmtCtx) {
		avformat_close_input(&pFmtCtx);//��close(���ر��ļ���Ҳ�ͷű����ڴ�)
		pFmtCtx = NULL;
	}
	if (oFmtCtx->pb) {
		avio_close(oFmtCtx->pb);
	}
	if (oFmtCtx) {
		avformat_free_context(oFmtCtx);//��free�������ͷű����ڴ棩
		oFmtCtx = NULL;
	}

}

void av_abstract_video() {
	//-------2024.9.4-------
	av_log_set_level(AV_LOG_DEBUG);
	//1.�򿪶�ý���ļ�
	// ����һ
	//std::shared_ptr<AVFormatContext> iFmtCtx=std::make_shared<AVFormatContext>();
	//iFmtCtx.reset(temp_iFmtCtx, FFmpegDeleteer<AVFormatContext>());
	
	std::shared_ptr<AVFormatContext> iFmtCtx(NULL, FFmpegDeleteer<AVFormatContext>());
	AVFormatContext* temp_iFmtCtx = nullptr;	//����ȡ����ָ�������ָ��ĵ�ַ����Ϊ�䷵�ص���һ����ʱֵ����ֵ��
	auto res_1 = avformat_open_input(&temp_iFmtCtx, LOCALFILE, NULL, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(nullptr, AV_LOG_ERROR, "Error:%s\n",errbuff);
		return;
		//exit(-1);����ָ�벻��ִ��������������Ϊ�����ǿ����ֹ��������ջ��
	}
	
	//����������
	// ������
	//std::shared_ptr<AVFormatContext> iFmtCtx(temp_iFmtCtx, FFmpegDeleteer<AVFormatContext>());
	
	//����������д��ʱ��ͻᱨ��
	//iFmtCtx.reset(temp_iFmtCtx);
	//������reset������ʱ��Դ����ָ�����Ķ���ͻᱻ���٣�
	//������reset�൱�ڴ���һ���µ�����ָ�룬����û�д�����Ҫ��ɾ����
	
	iFmtCtx.reset(temp_iFmtCtx, FFmpegDeleteer<AVFormatContext>());
	//temp_iFmtCtx = nullptr;
	//2.�Ӷ�ý���ļ��ҵ���Ƶ��
	auto index = av_find_best_stream(iFmtCtx.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (index < 0) {
		av_log(iFmtCtx.get(), AV_LOG_ERROR, "Error:Don't include any video stream...\n");
		return;
	}

	//3.��������ļ���������
	AVFormatContext* temp_oFmtCtx = nullptr;
	temp_oFmtCtx = avformat_alloc_context();
	if (!temp_oFmtCtx) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Memory equipped failed...\n");
		return;
	}
	const AVOutputFormat* outFmt = nullptr;
	outFmt = av_guess_format(NULL, OUTPUTFILE_V, NULL);	//��ȡ����ļ��ĸ�ʽ
	temp_oFmtCtx->oformat = outFmt;	//��
	std::shared_ptr<AVFormatContext> oFmtCtx=std::make_shared<AVFormatContext>();
	oFmtCtx.reset(temp_oFmtCtx, FFmpegDeleteer<AVFormatContext>());
	//temp_oFmtCtx = nullptr;
	//4.Ϊ����ļ�������Ƶ��
	AVStream* oStream = nullptr;
	oStream = avformat_new_stream(oFmtCtx.get(), nullptr);
	//5.������Ƶ����
	AVStream* iStream = iFmtCtx->streams[index];
	avcodec_parameters_copy(oStream->codecpar, iStream->codecpar);
	oStream->codecpar->codec_tag = 0;

	//6.��(��ʱ����ļ���Ϊpb����ռ�)
	auto res_2 = avio_open2(&(oFmtCtx.get()->pb), OUTPUTFILE_V, AVIO_FLAG_WRITE, NULL, NULL);
	if (res_2 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_2, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n",errbuff);
		return;
	}

	//7.д��ý���ļ�ͷ������ļ�������
	auto res_3 = avformat_write_header(oFmtCtx.get(), nullptr);
	if (res_3 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_3, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}

	//8.��Դ��ý�������Ƶ���ݵ�����ļ���
	AVPacket pkt;
	while (av_read_frame(iFmtCtx.get(), &pkt)>=0) {
		if (pkt.stream_index == index) {
			pkt.pts = av_rescale_q_rnd(pkt.pts, iStream->time_base, oStream->time_base, 
				(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			//��Ƶ��pts��һ������dts
			pkt.dts = av_rescale_q_rnd(pkt.dts, iStream->time_base, oStream->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.duration = av_rescale_q(pkt.duration, iStream->time_base, oStream->time_base);
			pkt.stream_index = 0;
			pkt.pos = -1;
			av_interleaved_write_frame(oFmtCtx.get(), &pkt);
			av_packet_unref(&pkt);
		}
	}

	//9.д��ý���ļ�β������ļ���
	av_write_trailer(oFmtCtx.get());
	//av_log(oFmtCtx.get(), AV_LOG_INFO, "HI~\n");
	return;
}

void av_turn() {
	av_log_set_level(AV_LOG_DEBUG);
	//1.�򿪶�ý���ļ�
	AVFormatContext* _iFmtCtx = nullptr;
	auto res_1 = avformat_open_input(&_iFmtCtx, LOCALFILE, NULL, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:%s\n", errbuff);
		avformat_close_input(&_iFmtCtx);
		return;
	}
	std::shared_ptr<AVFormatContext> iFmtCtx = std::make_shared<AVFormatContext>();
	iFmtCtx.reset(_iFmtCtx,FFmpegDeleteer<AVFormatContext>());

	//2.��������ļ���������
	
	// -----------������-----
	//AVFormatContext* _oFmtCtx = avformat_alloc_context();
	//if (!_oFmtCtx) {
	//	av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
	//	avformat_free_context(_oFmtCtx);
	//	return;
	//}
	//const AVOutputFormat* _outFmt = av_guess_format(nullptr, TURN_TO_FILE, NULL);
	//if (!_outFmt) {
	//	av_log(NULL, AV_LOG_ERROR, "Error: 'av_guess_format' failed...\n");
	//	avformat_free_context(_oFmtCtx);
	//	//ʧ�ܻ᷵�ؿ�ָ�룬��˲����ͷ�AVOuutputFormat���ڴ�s
	//	return;
	//}
	//_oFmtCtx->oformat = _outFmt;
	
	//------------������-------
	AVFormatContext* _oFmtCtx = nullptr;
	avformat_alloc_output_context2(&_oFmtCtx, nullptr, nullptr, REMUX_FILE);
	if (!_oFmtCtx) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		avformat_free_context(_oFmtCtx);
		return;
	}
	std::shared_ptr<AVFormatContext> oFmtCtx = std::make_shared<AVFormatContext>();
	oFmtCtx.reset(_oFmtCtx,FFmpegDeleteer<AVFormatContext>());

	//3.ΪĿ���ļ�������Ҫ����
	std::vector<int> stream_map(iFmtCtx->nb_streams, -1);
	int stream_index = 0;//����Ŵ�0��ʼ
	for (unsigned int i = 0; i < iFmtCtx->nb_streams; ++i) {
		AVStream* iStream = iFmtCtx->streams[i];
		AVStream* oStream = nullptr;
		AVCodecParameters* iCodecPar = iStream->codecpar;
		if (iCodecPar->codec_type == AVMEDIA_TYPE_AUDIO ||
			iCodecPar->codec_type == AVMEDIA_TYPE_VIDEO ||
			iCodecPar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
			stream_map[i] = stream_index++;
			oStream = avformat_new_stream(oFmtCtx.get(), NULL);
			if (!oStream) {
				av_log(oFmtCtx.get(), AV_LOG_ERROR, "No Memory...\n");
				return;
			}
			//4.���������������
			avcodec_parameters_copy(oStream->codecpar, iStream->codecpar);
			oStream->codecpar->codec_tag = 0;
		}
	}
	//5.������ļ���������������ļ���
	auto res_2 = avio_open2(&(oFmtCtx->pb), REMUX_FILE, AVIO_FLAG_WRITE, NULL, NULL);
	if (res_2 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_2, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}

	//6.������ļ�������д��ͷ��Ϣ
	auto res_3 = avformat_write_header(oFmtCtx.get(), NULL);
	if (res_3 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_3, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}

	//7.��Դ��ý���ļ���ȡ���ݵ�����ļ�����������
	AVPacket* _pkt = av_packet_alloc();
	if (!_pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		return;
	}
	std::shared_ptr<AVPacket> pkt = std::make_shared<AVPacket>();
	pkt.reset(_pkt, FFmpegDeleteer<AVPacket>());
	//�������
	while ((av_read_frame(iFmtCtx.get(),pkt.get()) >= 0)) {
		if (stream_map.at(pkt->stream_index) < 0) {
			av_packet_unref(pkt.get());
			continue;
		}
		//--------�޸�ʱ����ķ�����
		AVStream* iStream;
		AVStream* oStream;

		iStream = iFmtCtx->streams[pkt->stream_index];
		pkt->stream_index = stream_map.at(pkt->stream_index);
		oStream = oFmtCtx->streams[pkt->stream_index];
		av_packet_rescale_ts(pkt.get(), iStream->time_base,oStream->time_base);
		pkt->pos = -1;
		av_interleaved_write_frame(oFmtCtx.get(), pkt.get());
		av_packet_unref(pkt.get());
	}
	//8.д��ý���ļ�β������ļ�����������
	av_write_trailer(oFmtCtx.get());
}

void av_cut() {
	av_log_set_level(AV_LOG_DEBUG);
	//1.�򿪶�ý���ļ�
	AVFormatContext* _iFmtCtx = nullptr;
	auto res_1 = avformat_open_input(&_iFmtCtx, LOCALFILE, NULL, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:%s\n", errbuff);
		avformat_close_input(&_iFmtCtx);
		return;
	}
	std::shared_ptr<AVFormatContext> iFmtCtx = std::make_shared<AVFormatContext>();
	iFmtCtx.reset(_iFmtCtx, FFmpegDeleteer<AVFormatContext>());
	//2.��������ļ���������
	AVFormatContext* _oFmtCtx = nullptr;
	avformat_alloc_output_context2(&_oFmtCtx, nullptr, nullptr, CUT_FILE);
	if (!_oFmtCtx) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		avformat_free_context(_oFmtCtx);
		return;
	}
	std::shared_ptr<AVFormatContext> oFmtCtx = std::make_shared<AVFormatContext>();
	oFmtCtx.reset(_oFmtCtx, FFmpegDeleteer<AVFormatContext>());
	//3.��ȡĿ���ļ�����
	std::vector<int> stream_map(iFmtCtx->nb_streams, -1);
	int stream_index = 0;//����Ŵ�0��ʼ
	for (unsigned int i = 0; i < iFmtCtx->nb_streams; ++i) {
		AVStream* iStream = iFmtCtx->streams[i];
		AVStream* oStream = nullptr;
		AVCodecParameters* iCodecPar = iStream->codecpar;
		if (iCodecPar->codec_type == AVMEDIA_TYPE_AUDIO ||
			iCodecPar->codec_type == AVMEDIA_TYPE_VIDEO ||
			iCodecPar->codec_type == AVMEDIA_TYPE_SUBTITLE) {
			stream_map[i] = stream_index++;
			oStream = avformat_new_stream(oFmtCtx.get(), NULL);
			if (!oStream) {
				av_log(oFmtCtx.get(), AV_LOG_ERROR, "No Memory...\n");
				return;
			}
			//4.���������������
			avcodec_parameters_copy(oStream->codecpar, iStream->codecpar);
			oStream->codecpar->codec_tag = 0;
		}
	}
	//5.������ļ���������������ļ���
	auto res_2 = avio_open2(&(oFmtCtx->pb), CUT_FILE, AVIO_FLAG_WRITE, NULL, NULL);
	if (res_2 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_2, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	//6.������ļ�������д��ͷ��Ϣ
	auto res_3 = avformat_write_header(oFmtCtx.get(), NULL);
	if (res_3 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_3, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	//7.�Ӻδ���ȡ���ݣ�ע����Ƶ���Ľ�ȡ����Ƶ���Ľ�ȡ�ǲ�һ���ģ���Ϊ��Ƶ��I��P��B֡��
	auto res_4=av_seek_frame(iFmtCtx.get(), -1, START_TIME*AV_TIME_BASE,AVSEEK_FLAG_BACKWARD);
	if (res_4 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_4, errbuff, sizeof(errbuff));
		av_log(iFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	//8.��Դ��ý���ļ���ȡ���ݵ�����ļ�����������
	AVPacket* _pkt = av_packet_alloc();
	if (!_pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		return;
	}
	std::shared_ptr<AVPacket> pkt = std::make_shared<AVPacket>();
	pkt.reset(_pkt, FFmpegDeleteer<AVPacket>());
	//���ڴӽ�ȡ�㿪ʼ�����ʱ����ı�---��������vector�Ǽ�¼ÿһ·��������Ҫ�����Ĵӽ�ȡ�㿪ʼ��Ϊʱ��������
	std::vector<int64_t> _dts_start_arry(iFmtCtx->nb_streams,-1);
	std::vector<int64_t> _pts_start_arry(iFmtCtx->nb_streams,-1);
	while ((av_read_frame(iFmtCtx.get(), pkt.get()) >= 0)) {
		if (stream_map.at(pkt->stream_index) < 0) {	//����������
			av_packet_unref(pkt.get());
			continue;
		}
		if (_pts_start_arry.at(pkt->stream_index) == -1 && pkt->pts > 0) {	//ÿһ·���ĵ�һ����
			_pts_start_arry.at(pkt->stream_index) = pkt->pts;
		}
		if (_dts_start_arry.at(pkt->stream_index) == -1 && pkt->dts > 0) {	//ÿһ·���ĵ�һ����,��¼�˿̵�ʱ�����Ϊ��ʼ��ʱ���
			_dts_start_arry.at(pkt->stream_index) = pkt->dts;
		}
		pkt->pts = pkt->pts - _pts_start_arry.at(pkt->stream_index);
		pkt->dts = pkt->dts - _dts_start_arry.at(pkt->stream_index);
		if (pkt->pts < pkt->dts) {
			pkt->pts = pkt->dts;	//��֤��� PTS С�� DTS����ʾ����ʱ�����ڽ���ʱ�䣬���ǲ����߼��ġ�
									//����������£��޸� PTS Ϊ DTS ���Ա������������ȷ��֡�Ĳ���˳�����
		}
		//--------�޸�ʱ����ķ�����
		AVStream* iStream;
		AVStream* oStream;

		iStream = iFmtCtx->streams[pkt->stream_index];
		//�����ȡ����ֹʱ��
		if (av_q2d(iStream->time_base) * pkt->pts > END_TIME) {
			av_log(oFmtCtx.get(), AV_LOG_INFO, "Done!\n");
			break;
		}
		pkt->stream_index = stream_map.at(pkt->stream_index);
		oStream = oFmtCtx->streams[pkt->stream_index];
		av_packet_rescale_ts(pkt.get(), iStream->time_base, oStream->time_base);
		pkt->pos = -1;
		av_interleaved_write_frame(oFmtCtx.get(), pkt.get());
		av_packet_unref(pkt.get());
	}
	//8.д��ý���ļ�β������ļ�����������
	av_write_trailer(oFmtCtx.get());
}

void av_mix() {
	av_log_set_level(AV_LOG_DEBUG);
	AVFormatContext* _iFmtCtx1_A = nullptr;	//��Ƶ�ļ�
	AVFormatContext* _iFmtCtx2_V = nullptr;	//��Ƶ�ļ�
	//����ý���ļ�
	auto res_1 = avformat_open_input(&_iFmtCtx1_A, INPUTFILE_A, NULL, NULL);	//��Ƶý��
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:%s\n", errbuff);
		avformat_close_input(&_iFmtCtx1_A);
		return;
	}
	auto res_2 = avformat_open_input(&_iFmtCtx2_V, INPUTFILE_V, NULL, NULL);
	if (res_2 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_2, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:%s\n", errbuff);
		avformat_close_input(&_iFmtCtx1_A);
		avformat_close_input(&_iFmtCtx2_V);
		return;
	}
	std::shared_ptr<AVFormatContext> iFmtCtx1_A = std::make_shared <AVFormatContext>();
	iFmtCtx1_A.reset(_iFmtCtx1_A, FFmpegDeleteer<AVFormatContext>());
	std::shared_ptr<AVFormatContext> iFmtCtx2_V = std::make_shared <AVFormatContext>();
	iFmtCtx2_V.reset(_iFmtCtx2_V, FFmpegDeleteer<AVFormatContext>());

	//2.��������ļ���������
	AVFormatContext* _oFmtCtx_ = nullptr;
	avformat_alloc_output_context2(&_oFmtCtx_, NULL, NULL, OUTPUTFILE);
	if (!_oFmtCtx_) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		avformat_free_context(_oFmtCtx_);
		return;
	}
	std::shared_ptr<AVFormatContext> _oFmtCtx = std::make_shared<AVFormatContext>();
	_oFmtCtx.reset(_oFmtCtx_, FFmpegDeleteer<AVFormatContext>());

	//3.�ҳ���Ƶ������Ƶ��
	auto Index_A = av_find_best_stream(iFmtCtx1_A.get(), AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	auto Index_V = av_find_best_stream(iFmtCtx2_V.get(), AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (Index_A < 0) {
		av_log(iFmtCtx1_A.get(), AV_LOG_ERROR, "Error:Don't include any audio stream...\n");
		return;
	}
	if (Index_V < 0) {
		av_log(iFmtCtx2_V.get(), AV_LOG_ERROR, "Error:Don't include any video stream...\n");
		return;
	}

	//4.Ϊ����ļ�����������ֵ
	//��Ƶ��
	AVStream* iStream_A = iFmtCtx1_A->streams[Index_A];
	AVStream* oStream1 = avformat_new_stream(_oFmtCtx.get(), nullptr);
	if (!oStream1) {
		av_log(_oFmtCtx.get(), AV_LOG_ERROR, "Failed to alloc out stream!\n");
		return;
	}
	auto res_3 = avcodec_parameters_copy(oStream1->codecpar, iStream_A->codecpar);
	if (res_3 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_3, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	oStream1->codecpar->codec_tag = 0;
	int stream_index_1 = 0;
	//��Ƶ��
	AVStream* iStream_V = iFmtCtx2_V->streams[Index_V];
	AVStream* oStream2 = avformat_new_stream(_oFmtCtx.get(), NULL);
	if (!oStream2) {
		av_log(_oFmtCtx.get(), AV_LOG_ERROR, "Failed to alloc out stream!\n");
		return;
	}
	auto res_4 = avcodec_parameters_copy(oStream2->codecpar, iStream_V->codecpar);
	if (res_4 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_4, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	oStream2->codecpar->codec_tag = 0;
	int stream_index_2 = 1;
	av_dump_format(_oFmtCtx.get(), 0, OUTPUTFILE, 1);	//���һЩ��ʽ����Ϣ
	//5.ȷ����󳤶�
	double max_duration = 0;
	if (iStream_A->duration * av_q2d(iStream_A->time_base) > 
		iStream_V->duration * av_q2d(iStream_V->time_base)) {
		max_duration = iStream_V->duration * av_q2d(iStream_V->time_base);
	}
	else {
		max_duration = iStream_A->duration * av_q2d(iStream_A->time_base);
	}
	//6.�󶨲�������ļ�
	auto res_5 = avio_open2(&(_oFmtCtx->pb), OUTPUTFILE, AVIO_FLAG_WRITE, NULL, NULL);
	if (res_5 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_5, errbuff, sizeof(errbuff));
		av_log(_oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	//7.������ļ�д���ļ�ͷ����
	auto res_6 = avformat_write_header(_oFmtCtx.get(), NULL);
	if (res_6 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_6, errbuff, sizeof(errbuff));
		av_log(_oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	//8.��ȡ��Ƶ����
	AVPacket* _pkt = av_packet_alloc();
	if (!_pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		return;
	}
	std::shared_ptr<AVPacket> pkt = std::make_shared<AVPacket>();
	pkt.reset(_pkt, FFmpegDeleteer<AVPacket>());
	while ( av_read_frame(iFmtCtx1_A.get(), pkt.get() )>=0) {
		if (pkt->pts * av_q2d(iStream_A->time_base) > max_duration) {	//��ȡ��ʱ������ʱ��
			av_packet_unref(pkt.get());
			continue;	//����break;
		}
		if (pkt->stream_index == Index_A) {
			/*pkt->pts = av_rescale_q_rnd(pkt->pts, iStream_A->time_base, oStream1->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt->dts = av_rescale_q_rnd(pkt->dts, iStream_A->time_base, oStream1->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt->duration = av_rescale_q(pkt->duration, iStream_A->time_base, oStream1->time_base);*/
			av_packet_rescale_ts(pkt.get(), iStream_A->time_base, oStream1->time_base);
			pkt->stream_index = stream_index_1;
			pkt->pos = -1;
			av_interleaved_write_frame(_oFmtCtx.get(), pkt.get());
			av_packet_unref(pkt.get());
		}
	}
	while (av_read_frame(iFmtCtx2_V.get(), pkt.get()) >= 0) {
		if (pkt->pts * av_q2d(iStream_V->time_base) > max_duration) {
			av_packet_unref(pkt.get());
			continue;
		}
		if (pkt->stream_index == Index_V) {
			av_packet_rescale_ts(pkt.get(), iStream_V->time_base, oStream2->time_base);
			pkt->stream_index = stream_index_2;
			pkt->pos = 1;
			av_interleaved_write_frame(_oFmtCtx.get(), pkt.get());
			av_packet_unref(pkt.get());
		}
	}
	//9.дβ����Ϣ
	av_write_trailer(_oFmtCtx.get());
}

void av_encodec_v() {
	//1.���ұ�����
	av_log_set_level(AV_LOG_DEBUG);
	const AVCodec* codec = nullptr;
	codec = avcodec_find_encoder_by_name(CODEC_NAME);
	if (!codec) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Don't find Codec: %s", CODEC_NAME);
		return;
	}
	//2.����������������
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Memory alloc failed...\n");
		return;
	}
	//3.���ñ���������
	ctx->height = 640;
	ctx->width = 640;
	ctx->bit_rate = 500000;//����Խ��������Խ�ߣ����޶��--->������500kbps

	ctx->time_base = av_make_q(1, 25);
	ctx->framerate = av_make_q(25, 1);

	ctx->gop_size = 10;//ÿ10֡һ��
	ctx->max_b_frames = 1;	//ÿ�����һ��b֡
	ctx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264) {
		av_opt_set(ctx->priv_data, "preset", "slow",0);
	}
	std::shared_ptr<AVCodecContext> _ctx = std::make_shared <AVCodecContext>();
	_ctx.reset(ctx, FFmpegDeleteer<AVCodecContext>());
	//4.���������ͱ������ļ������İ���һ��
	auto res_1 = avcodec_open2(_ctx.get(), codec, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open codec :%s\n", errbuff);
		return;
	}
	//5.��������ļ�(������)
	std::ofstream outFile(CODEC_FILE_V, std::ios::out | std::ios::binary);
	if (!outFile) {
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open the file...\n");
		outFile.close();
		return;
	}
	//6.����AVFrame
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVFrame Memory alloc failed...\n");
		outFile.close();
		return;
	}
	frame->width = _ctx->width;
	frame->height = _ctx->height;
	frame->format = _ctx->pix_fmt;
	res_1 = av_frame_get_buffer(frame, 0);
	if (res_1 < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error : av_frame_get_buffer() alloc failed...\n");
		outFile.close();
		return;
	}
	std::shared_ptr<AVFrame> _frame = std::make_shared<AVFrame>();
	_frame.reset(frame, FFmpegDeleteer<AVFrame>());
	//7.����AVPacket
	AVPacket* pkt = av_packet_alloc();
	if (!pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVPacket Memory alloc failed...\n");
		outFile.close();
		return;
	}
	std::shared_ptr<AVPacket> _pkt = std::make_shared<AVPacket>();
	_pkt.reset(pkt, FFmpegDeleteer<AVPacket>());
	//8.������Ƶ���ݣ�һ����˵����Ի�ȡ����ͷ����ץȡ��Ϊ��ƵԴ��������Ϊ�˼�㣩
	//��������25֡
	for (int i = 0; i < 25; ++i) {
		res_1=av_frame_make_writable(frame);	//ȷ��frame��data�����
		if (res_1 < 0) {
			break;
		}
		//��ÿһ֡�����ص�
		//Y����
		for (int y = 0; y < _ctx->height; ++y) {
			for (int x = 0; x < _ctx->width; ++x) {
				//yuv���ݴ���
				_frame->data[0][y*_frame->linesize[0]+x]=x+y+i*3;
			}
		}
		//U��V����
		for (int y = 0; y < _ctx->height / 2; ++y) {
			for (int x = 0; x < _ctx->width / 2; ++x) {
				_frame->data[1][y * _frame->linesize[1] + x] = 128 + y + i * 2;
				_frame->data[2][y * _frame->linesize[2] + x] = 64 + x + i * 5;
			}
		}
		_frame->pts = i;//���
		res_1=encodec(_ctx.get(), _frame.get(), _pkt.get(), outFile);
		if (res_1 < 0) {
			outFile.close();
			return;
		}
	}
	//9.�ڱ������Ļ��������ܻ��д�������
	//�ڶ�ȡ���п���֡���ʵ�����������ڲ��Ļ�����
	encodec(_ctx.get(), NULL, _pkt.get(), outFile);
	outFile.close();
	return;
}

void av_encodec_a() {
	//1.���ұ�����
	av_log_set_level(AV_LOG_DEBUG);
	const AVCodec* codec = nullptr;

	//codec = avcodec_find_encoder_by_name(CODEC_NAME);
	codec=avcodec_find_encoder_by_name("libfdk_aac");	//ʹ�õ�����aac�⣨�Ƽ���
	//codec=avcodec_find_encoder(AV_CODEC_ID_AAC);	//ʹ��FFmpeg���� ���������ͣ�
	if (!codec) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Can't find Codec\n");
		return;
	}
	//2.����������������
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Memory alloc failed...\n");
		return;
	}
	//3.���ñ���������
	/*ctx->height = 640;
	ctx->width = 640;*/

	ctx->bit_rate = 64000;//����Խ��������Խ�ߣ����޶��--->������500kbps

	/*ctx->time_base = av_make_q(1, 25);
	ctx->framerate = av_make_q(25, 1);*/

	//ctx->gop_size = 10;//ÿ10֡һ��
	//ctx->max_b_frames = 1;	//ÿ�����һ��b֡
	//ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	
	//���ò�����
	//���� ctx->sample_fmt=AV_SAMPLE_FMT_FLTP
	ctx->sample_fmt = AV_SAMPLE_FMT_S16;	//S:�з��ţ�16�����ֽڣ����ò����ʸ�ʽ

	if (!check_sample_fmt(codec, ctx->sample_fmt)) {	//���������Ƿ�֧�ָò����ʸ�ʽ
		av_log(NULL, AV_LOG_ERROR, "Error:Encoder doesn't support sample format...\n");
		avcodec_free_context(&ctx);
		return;
	}
	ctx->sample_rate = std::stoi(select_best_sample_rate(codec));

	//��������
	
	//����Ϊ��Ӧ������ʽs
	//const AVChannelLayout* av_channel_layout = &(AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
	
	//AV_CHANNEL_LAYOUT_STEREO ��һ���궨�壬������һ����һ��ʵ�ʶ���ĵ�ַ��
	//��������һ����ʱ�Ľṹ����߳������ʽ������޷�����ȡ��ַ��
	//const AVChannelLayout* av_channel_layout = const_cast<AVChannelLayout*>(&AV_CHANNEL_LAYOUT_STEREO);
	
	//���� AVChannelLayout av_channel_layout = AV_CHANNEL_LAYOUT_MONO;
	AVChannelLayout av_channel_layout = AV_CHANNEL_LAYOUT_STEREO;
	av_channel_layout_copy(&ctx->ch_layout, &av_channel_layout);

	/*if (codec->id == AV_CODEC_ID_H264) {
		av_opt_set(ctx->priv_data, "preset", "slow", 0);
	}*/
	std::shared_ptr<AVCodecContext> _ctx = std::make_shared <AVCodecContext>();
	_ctx.reset(ctx, FFmpegDeleteer<AVCodecContext>());
	//4.���������ͱ������ļ������İ���һ��
	auto res_1 = avcodec_open2(_ctx.get(), codec, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open codec :%s\n", errbuff);
		return;
	}
	//5.��������ļ�(������)
	std::ofstream outFile(CODEC_FILE_A, std::ios::out | std::ios::binary);
	if (!outFile) {
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open the file...\n");
		outFile.close();
		return;
	}
	//6.����AVFrame
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVFrame Memory alloc failed...\n");
		outFile.close();
		return;
	}
	/*frame->width = _ctx->width;
	frame->height = _ctx->height;
	frame->format = _ctx->pix_fmt;*/

	frame->nb_samples = _ctx->frame_size;
	frame->format = _ctx->sample_fmt;
	av_channel_layout_copy(&frame->ch_layout, &_ctx->ch_layout);
	
	res_1 = av_frame_get_buffer(frame, 0);
	if (res_1 < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error : av_frame_get_buffer() alloc failed...\n");
		outFile.close();
		return;
	}
	std::shared_ptr<AVFrame> _frame = std::make_shared<AVFrame>();
	_frame.reset(frame, FFmpegDeleteer<AVFrame>());
	//7.����AVPacket
	AVPacket* pkt = av_packet_alloc();
	if (!pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVPacket Memory alloc failed...\n");
		outFile.close();
		return;
	}
	std::shared_ptr<AVPacket> _pkt = std::make_shared<AVPacket>();
	_pkt.reset(pkt, FFmpegDeleteer<AVPacket>());
	//8.������Ƶ���ݣ�һ����˵����Ի�ȡ�������������Ϊ��ƵԴ��������Ϊ�˼�㣩
	//����Ƶ֡��ͬ����������Ƶ��
	uint16_t* samples = nullptr;
	double t = 0;
	double tincr = 2 * M_PI * 440 / _ctx->sample_rate;
	for (int i = 0; i < 200; ++i) {
		res_1=av_frame_make_writable(_frame.get());
		if (res_1 < 0) {
			av_log(NULL, AV_LOG_ERROR, "Error:Can't alloc space...\n");
			return;
		}
		samples =(uint16_t*) _frame->data[0];	//���� FLTPΪ32λ��(uint32_t*)����
		for (int j = 0; j < _ctx->frame_size; ++j) {	//����*2����Ϊ*4
			samples[2 * j] = (int)(sin(t) * 10000);	//��������ҲҪ�޸�
			for (int k = 1; k < _ctx->ch_layout.nb_channels; ++k) {
				samples[2 * j + k] = samples[2 * j];
			}
			t += tincr;
		}
		encodec(_ctx.get(), _frame.get(), _pkt.get(), outFile);
	}
	//9.�ڱ������Ļ��������ܻ��д�������
	//�ڶ�ȡ���п���֡���ʵ�����������ڲ��Ļ�����
	encodec(_ctx.get(), NULL, _pkt.get(), outFile);
	outFile.close();
	return;
}

void av_decodec_pix_PGM() {

	av_log_set_level(AV_LOG_DEBUG);

	//1.�򿪶�ý���ļ�
	std::shared_ptr<AVFormatContext> iFmtCtx = std::make_shared<AVFormatContext>();
	//���µĳ�ʼ����ʽ�ᱨ��-----�����ڴ�����
	//std::shared_ptr<AVFormatContext> iFmtCtx(NULL, FFmpegDeleteer<AVFormatContext>());
	AVFormatContext* temp_iFmtCtx = nullptr;	//����ȡ����ָ�������ָ��ĵ�ַ����Ϊ�䷵�ص���һ����ʱֵ����ֵ��
	auto res_1 = avformat_open_input(&temp_iFmtCtx, LOCALFILE, NULL, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(nullptr, AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
		//exit(-1);����ָ�벻��ִ��������������Ϊ�����ǿ����ֹ��������ջ��
	}
	iFmtCtx.reset(temp_iFmtCtx, FFmpegDeleteer<AVFormatContext>());

	//2.�Ӷ�ý���ļ��ҵ���Ƶ��
	auto index = av_find_best_stream(iFmtCtx.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (index < 0) {
		av_log(iFmtCtx.get(), AV_LOG_ERROR, "Error:Don't include any video stream...\n");
		return;
	}

	//3.���ҽ�����
	const AVCodec* codec = nullptr;
	AVStream* inStream = iFmtCtx->streams[index];
	codec = avcodec_find_decoder(inStream->codecpar->codec_id);
	if (!codec) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Don't find libx264...\n");
		return;
	}

	//4.����������������
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Memory alloc failed...\n");
		return;
	}
	avcodec_parameters_to_context(ctx, inStream->codecpar);
	std::shared_ptr<AVCodecContext> _ctx = std::make_shared <AVCodecContext>();
	_ctx.reset(ctx, FFmpegDeleteer<AVCodecContext>());

	//5.���������ͽ������ļ������İ���һ��
	res_1 = avcodec_open2(_ctx.get(), codec, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open codec :%s\n", errbuff);
		return;
	}

	//7.����AVFrame
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVFrame Memory alloc failed...\n");
		return;
	}
	std::shared_ptr<AVFrame> _frame = std::make_shared<AVFrame>();
	_frame.reset(frame, FFmpegDeleteer<AVFrame>());
	
	//8.����AVPacket
	AVPacket* pkt = av_packet_alloc();
	if (!pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVPacket Memory alloc failed...\n");
		return;
	}
	std::shared_ptr<AVPacket> _pkt = std::make_shared<AVPacket>();
	_pkt.reset(pkt, FFmpegDeleteer<AVPacket>());
	
	//9.��Դ��ý�������Ƶ���ݵ�����ļ���
	std::string filename("out");
	int count = 1;
	while (av_read_frame(iFmtCtx.get(), _pkt.get()) >= 0) {
		if (_pkt->stream_index == index) {
			res_1=decodec_pix_PGM(_ctx.get(), _frame.get(), _pkt.get(),filename,count);
			if (res_1 < 0) {
				return;
			}
			if (res_1 == COUNT_PIX) {
				return;
			}
		}
	}
	//�ڶ�ȡ���п���֡���ʵ�����������ڲ��Ļ�����
	//����������ڲ��Ļ�����
	decodec_pix_PGM(_ctx.get(), _frame.get(), nullptr, filename,count);
	return;
}

void av_decodec_pix_BMP() {
	av_log_set_level(AV_LOG_DEBUG);

	//1.�򿪶�ý���ļ�
	std::shared_ptr<AVFormatContext> iFmtCtx = std::make_shared<AVFormatContext>();
	//���µĳ�ʼ����ʽ�ᱨ��-----�����ڴ�����
	//std::shared_ptr<AVFormatContext> iFmtCtx(NULL, FFmpegDeleteer<AVFormatContext>());
	AVFormatContext* temp_iFmtCtx = nullptr;	//����ȡ����ָ�������ָ��ĵ�ַ����Ϊ�䷵�ص���һ����ʱֵ����ֵ��
	auto res_1 = avformat_open_input(&temp_iFmtCtx, LOCALFILE, NULL, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(nullptr, AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
		//exit(-1);����ָ�벻��ִ��������������Ϊ�����ǿ����ֹ��������ջ��
	}
	iFmtCtx.reset(temp_iFmtCtx, FFmpegDeleteer<AVFormatContext>());

	//2.�Ӷ�ý���ļ��ҵ���Ƶ��
	auto index = av_find_best_stream(iFmtCtx.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (index < 0) {
		av_log(iFmtCtx.get(), AV_LOG_ERROR, "Error:Don't include any video stream...\n");
		return;
	}

	//3.���ҽ�����
	const AVCodec* codec = nullptr;
	AVStream* inStream = iFmtCtx->streams[index];
	codec = avcodec_find_decoder(inStream->codecpar->codec_id);
	if (!codec) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Don't find libx264...\n");
		return;
	}

	//4.����������������
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Memory alloc failed...\n");
		return;
	}
	avcodec_parameters_to_context(ctx, inStream->codecpar);
	std::shared_ptr<AVCodecContext> _ctx = std::make_shared <AVCodecContext>();
	_ctx.reset(ctx, FFmpegDeleteer<AVCodecContext>());

	//5.���������ͽ������ļ������İ���һ��
	res_1 = avcodec_open2(_ctx.get(), codec, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open codec :%s\n", errbuff);
		return;
	}

	//5.1��PGM��ɫͼƬ��ȣ�BMPͼƬ�������
	//�д��󣬻ᶪ֡
	//AVPixelFormat pix_fmt = _ctx->pix_fmt;
	//if (pix_fmt == AV_PIX_FMT_NONE) {
	//	av_log(nullptr, AV_LOG_WARNING, "Pixel format not set yet...\n");	//�����������ļ�û��Ԥ���������ظ�ʽ���������޷���������pix_fmt
	//	av_log(nullptr, AV_LOG_WARNING, "Start setting pixel format...\n");
	//	AVFrame* frame=av_frame_alloc();
	//	AVPacket* packet=av_packet_alloc();
	//	auto current_pos = avio_tell(iFmtCtx->pb);
	//	while(av_read_frame(iFmtCtx.get(), packet) >= 0) {
	//		if (packet->stream_index == index) {
	//			res_1 = avcodec_send_packet(_ctx.get(), packet);
	//			if (res_1 >= 0) {
	//				res_1 = avcodec_receive_frame(_ctx.get(), frame);
	//				if (res_1 >= 0) {
	//					//����ɹ�,��ȡ��pix_fmt
	//					pix_fmt = _ctx->pix_fmt;
	//					av_packet_unref(packet);
	//					av_log(nullptr, AV_LOG_WARNING, "Pixel format set successfully...\n");
	//					break;
	//				}
	//			}
	//		}
	//		av_packet_unref(packet);
	//	}
	//	//�ع�����ǰ��λ�ã�����������������
	//	av_seek_frame(iFmtCtx.get(), index, current_pos, AVSEEK_FLAG_BACKWARD);
	//	avcodec_flush_buffers(_ctx.get());
	//	
	//	av_frame_free(&frame);
	//	av_packet_free(&packet);
	//}
	
	//6.����һ��SwsContext����
	std::shared_ptr<struct SwsContext> swsCtx_shared_ptr(nullptr, FFmpegDeleteer<struct SwsContext>());
	
	//7.����AVFrame
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVFrame Memory alloc failed...\n");
		return;
	}
	std::shared_ptr<AVFrame> _frame = std::make_shared<AVFrame>();
	_frame.reset(frame, FFmpegDeleteer<AVFrame>());

	//8.����AVPacket
	AVPacket* pkt = av_packet_alloc();
	if (!pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVPacket Memory alloc failed...\n");
		return;
	}
	std::shared_ptr<AVPacket> _pkt = std::make_shared<AVPacket>();
	_pkt.reset(pkt, FFmpegDeleteer<AVPacket>());

	//9.��Դ��ý�������Ƶ���ݵ�����ļ���
	std::string filename("output");
	int count = 0;
	bool first_time = true;
	while (av_read_frame(iFmtCtx.get(), _pkt.get()) >= 0) {
		if (_pkt->stream_index == index) {
			res_1 = decodec_pix_BMP(_ctx.get(), swsCtx_shared_ptr, _frame.get(), _pkt.get(), filename, count,first_time);
			if (res_1 < 0) {
				return;
			}
			if (res_1 == COUNT_PIX) {
				return;
			}
		}
	}
	//�ڶ�ȡ���п���֡���ʵ�����������ڲ��Ļ�����
	//����������ڲ��Ļ�����
	decodec_pix_BMP(_ctx.get(), swsCtx_shared_ptr, _frame.get(), nullptr, filename, count,first_time);
	return;
}

int main() {

	//av_abstract_audio();
	
	//av_abstract_video();
	
	//av_turn();

	//av_cut();

	//av_mix();

	//av_encodec_v();

	//av_encodec_a();
	
	//av_decodec_pix_PGM();
	
	//av_decodec_pix_BMP();//--ʧ��

	//printf("%s\n", avcodec_configuration());
	return 0;
}