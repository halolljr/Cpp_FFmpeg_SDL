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
//以秒为单位
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

	//非FFmpeg公开API，不可以直接调用
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
	//1.打开多媒体文件

	AVFormatContext* pFmtCtx = nullptr;
	int res_1 = -1;
	res_1 = avformat_open_input(&pFmtCtx, LOCALFILE, NULL, NULL);//详细阅读avformat_open_input及其配套函数
	if (res_1 < 0) {
		char errbuf[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuf, sizeof(errbuf));
		av_log(pFmtCtx, AV_LOG_ERROR, "Error:%s\n", errbuf);//将错误码转化为错误信息描述
		//av_log(NULL, AV_LOG_ERROR, "Error: %s\n", av_err2str(res_1)); av_err2str()可能在某些FFmpeg版本不可用
		exit(-1);
	}
	//2.从多媒体文件中找到音频流

	auto index = av_find_best_stream(pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);//详细阅读av_find_best_stream及其配套函数
	if (index < 0) {
		av_log(pFmtCtx, AV_LOG_ERROR, "Error:Don't include any audio stream...\n");
		if (pFmtCtx)
			avformat_close_input(&pFmtCtx);
		exit(-1);
	}

	//3.创建输出文件的上下文
	AVFormatContext* oFmtCtx = nullptr;
	oFmtCtx = avformat_alloc_context();//详细阅读avformat_alloc_context()及其配套函数
	if (!oFmtCtx) {
		av_log(NULL, AV_LOG_ERROR, "Error:Memory apply failed...\n");
		if (pFmtCtx)
			avformat_close_input(&pFmtCtx);
		exit(-1);
	}
	std::string dst = OUTPUTFILE_A;//疑惑点-------
	const AVOutputFormat* oFmt = NULL;
	oFmt = av_guess_format(nullptr, dst.c_str(), NULL);//通过目的文件找到输出的文件的一些参数---疑惑点
	oFmtCtx->oformat = oFmt;

	//4.为输出文件创建一个新的音频流
	AVStream* outStream = avformat_new_stream(oFmtCtx, nullptr);

	//5.设置输出文件的音频参数--这里不做改变
	avcodec_parameters_copy(outStream->codecpar, pFmtCtx->streams[index]->codecpar);
	outStream->codecpar->codec_tag = 0;	//设置为0->根据多媒体文件自动设置编解码器

	//6.写多媒体文件头到输出文件的上下文
	auto res_2 = avio_open2(&oFmtCtx->pb, dst.c_str(), AVIO_FLAG_WRITE, NULL, nullptr);//将目的文件与输出文件绑定
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
	//写入头信息
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

	//7.从源多媒体文件中读到音频数据到目的文件中
	AVPacket pkt;
	while (av_read_frame(pFmtCtx, &pkt) >= 0) {
		if (pkt.stream_index == index) {//取所需的音频流
			//改变时间戳--对于音频来说（dts==pts）
			pkt.pts = av_rescale_q_rnd(pkt.pts, pFmtCtx->streams[index]->time_base, outStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.dts = pkt.pts;
			pkt.duration = av_rescale_q(pkt.duration, pFmtCtx->streams[index]->time_base, outStream->time_base);
			pkt.stream_index = 0;	//只有一路音频
			pkt.pos = -1;	//自动进行计算
			av_interleaved_write_frame(oFmtCtx, &pkt);
			av_packet_unref(&pkt);	//减少pkt的引用次数
		}
	}

	//8.写多媒体文件尾巴到输出文件上下文中
	av_write_trailer(oFmtCtx);

	//9.释放资源
	if (pFmtCtx) {
		avformat_close_input(&pFmtCtx);//用close(即关闭文件流也释放本身内存)
		pFmtCtx = NULL;
	}
	if (oFmtCtx->pb) {
		avio_close(oFmtCtx->pb);
	}
	if (oFmtCtx) {
		avformat_free_context(oFmtCtx);//用free（仅仅释放本身内存）
		oFmtCtx = NULL;
	}

}

void av_abstract_video() {
	//-------2024.9.4-------
	av_log_set_level(AV_LOG_DEBUG);
	//1.打开多媒体文件
	// 方法一
	//std::shared_ptr<AVFormatContext> iFmtCtx=std::make_shared<AVFormatContext>();
	//iFmtCtx.reset(temp_iFmtCtx, FFmpegDeleteer<AVFormatContext>());
	
	std::shared_ptr<AVFormatContext> iFmtCtx(NULL, FFmpegDeleteer<AVFormatContext>());
	AVFormatContext* temp_iFmtCtx = nullptr;	//不能取智能指针包裹的指针的地址，因为其返回的是一个临时值（右值）
	auto res_1 = avformat_open_input(&temp_iFmtCtx, LOCALFILE, NULL, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(nullptr, AV_LOG_ERROR, "Error:%s\n",errbuff);
		return;
		//exit(-1);智能指针不会执行析构函数，因为程序会强行终止，不回收栈了
	}
	
	//或者这样子
	// 方法二
	//std::shared_ptr<AVFormatContext> iFmtCtx(temp_iFmtCtx, FFmpegDeleteer<AVFormatContext>());
	
	//当你这样子写的时候就会报错
	//iFmtCtx.reset(temp_iFmtCtx);
	//当调用reset函数的时候，源智能指针管理的对象就会被销毁，
	//调用了reset相当于创建一个新的智能指针，但你没有传其需要的删除器
	
	iFmtCtx.reset(temp_iFmtCtx, FFmpegDeleteer<AVFormatContext>());
	//temp_iFmtCtx = nullptr;
	//2.从多媒体文件找到视频流
	auto index = av_find_best_stream(iFmtCtx.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (index < 0) {
		av_log(iFmtCtx.get(), AV_LOG_ERROR, "Error:Don't include any video stream...\n");
		return;
	}

	//3.创建输出文件的上下文
	AVFormatContext* temp_oFmtCtx = nullptr;
	temp_oFmtCtx = avformat_alloc_context();
	if (!temp_oFmtCtx) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Memory equipped failed...\n");
		return;
	}
	const AVOutputFormat* outFmt = nullptr;
	outFmt = av_guess_format(NULL, OUTPUTFILE_V, NULL);	//获取输出文件的格式
	temp_oFmtCtx->oformat = outFmt;	//绑定
	std::shared_ptr<AVFormatContext> oFmtCtx=std::make_shared<AVFormatContext>();
	oFmtCtx.reset(temp_oFmtCtx, FFmpegDeleteer<AVFormatContext>());
	//temp_oFmtCtx = nullptr;
	//4.为输出文件创建音频流
	AVStream* oStream = nullptr;
	oStream = avformat_new_stream(oFmtCtx.get(), nullptr);
	//5.设置音频参数
	AVStream* iStream = iFmtCtx->streams[index];
	avcodec_parameters_copy(oStream->codecpar, iStream->codecpar);
	oStream->codecpar->codec_tag = 0;

	//6.绑定(此时会打开文件流为pb分配空间)
	auto res_2 = avio_open2(&(oFmtCtx.get()->pb), OUTPUTFILE_V, AVIO_FLAG_WRITE, NULL, NULL);
	if (res_2 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_2, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n",errbuff);
		return;
	}

	//7.写多媒体文件头到输出文件上下文
	auto res_3 = avformat_write_header(oFmtCtx.get(), nullptr);
	if (res_3 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_3, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}

	//8.从源多媒体读到视频数据到输出文件中
	AVPacket pkt;
	while (av_read_frame(iFmtCtx.get(), &pkt)>=0) {
		if (pkt.stream_index == index) {
			pkt.pts = av_rescale_q_rnd(pkt.pts, iStream->time_base, oStream->time_base, 
				(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			//视频的pts不一定等于dts
			pkt.dts = av_rescale_q_rnd(pkt.dts, iStream->time_base, oStream->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
			pkt.duration = av_rescale_q(pkt.duration, iStream->time_base, oStream->time_base);
			pkt.stream_index = 0;
			pkt.pos = -1;
			av_interleaved_write_frame(oFmtCtx.get(), &pkt);
			av_packet_unref(&pkt);
		}
	}

	//9.写多媒体文件尾到输出文件中
	av_write_trailer(oFmtCtx.get());
	//av_log(oFmtCtx.get(), AV_LOG_INFO, "HI~\n");
	return;
}

void av_turn() {
	av_log_set_level(AV_LOG_DEBUG);
	//1.打开多媒体文件
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

	//2.创建输出文件的上下文
	
	// -----------方法①-----
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
	//	//失败会返回空指针，因此不用释放AVOuutputFormat的内存s
	//	return;
	//}
	//_oFmtCtx->oformat = _outFmt;
	
	//------------方法②-------
	AVFormatContext* _oFmtCtx = nullptr;
	avformat_alloc_output_context2(&_oFmtCtx, nullptr, nullptr, REMUX_FILE);
	if (!_oFmtCtx) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		avformat_free_context(_oFmtCtx);
		return;
	}
	std::shared_ptr<AVFormatContext> oFmtCtx = std::make_shared<AVFormatContext>();
	oFmtCtx.reset(_oFmtCtx,FFmpegDeleteer<AVFormatContext>());

	//3.为目的文件创建需要的流
	std::vector<int> stream_map(iFmtCtx->nb_streams, -1);
	int stream_index = 0;//流序号从0开始
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
			//4.设置流的输出参数
			avcodec_parameters_copy(oStream->codecpar, iStream->codecpar);
			oStream->codecpar->codec_tag = 0;
		}
	}
	//5.将输出文件的上下文与输出文件绑定
	auto res_2 = avio_open2(&(oFmtCtx->pb), REMUX_FILE, AVIO_FLAG_WRITE, NULL, NULL);
	if (res_2 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_2, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}

	//6.向输出文件上下文写入头信息
	auto res_3 = avformat_write_header(oFmtCtx.get(), NULL);
	if (res_3 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_3, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}

	//7.从源多媒体文件读取数据到输出文件的上下文中
	AVPacket* _pkt = av_packet_alloc();
	if (!_pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		return;
	}
	std::shared_ptr<AVPacket> pkt = std::make_shared<AVPacket>();
	pkt.reset(_pkt, FFmpegDeleteer<AVPacket>());
	//这里别乱
	while ((av_read_frame(iFmtCtx.get(),pkt.get()) >= 0)) {
		if (stream_map.at(pkt->stream_index) < 0) {
			av_packet_unref(pkt.get());
			continue;
		}
		//--------修改时间戳的方法②
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
	//8.写多媒体文件尾到输出文件的上下文中
	av_write_trailer(oFmtCtx.get());
}

void av_cut() {
	av_log_set_level(AV_LOG_DEBUG);
	//1.打开多媒体文件
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
	//2.创建输出文件的上下文
	AVFormatContext* _oFmtCtx = nullptr;
	avformat_alloc_output_context2(&_oFmtCtx, nullptr, nullptr, CUT_FILE);
	if (!_oFmtCtx) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		avformat_free_context(_oFmtCtx);
		return;
	}
	std::shared_ptr<AVFormatContext> oFmtCtx = std::make_shared<AVFormatContext>();
	oFmtCtx.reset(_oFmtCtx, FFmpegDeleteer<AVFormatContext>());
	//3.读取目标文件的流
	std::vector<int> stream_map(iFmtCtx->nb_streams, -1);
	int stream_index = 0;//流序号从0开始
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
			//4.设置流的输出参数
			avcodec_parameters_copy(oStream->codecpar, iStream->codecpar);
			oStream->codecpar->codec_tag = 0;
		}
	}
	//5.将输出文件的上下文与输出文件绑定
	auto res_2 = avio_open2(&(oFmtCtx->pb), CUT_FILE, AVIO_FLAG_WRITE, NULL, NULL);
	if (res_2 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_2, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	//6.向输出文件上下文写入头信息
	auto res_3 = avformat_write_header(oFmtCtx.get(), NULL);
	if (res_3 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_3, errbuff, sizeof(errbuff));
		av_log(oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	//7.从何处截取数据（注意音频流的截取和视频流的截取是不一样的，因为视频有I、P、B帧）
	auto res_4=av_seek_frame(iFmtCtx.get(), -1, START_TIME*AV_TIME_BASE,AVSEEK_FLAG_BACKWARD);
	if (res_4 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_4, errbuff, sizeof(errbuff));
		av_log(iFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	//8.从源多媒体文件读取数据到输出文件的上下文中
	AVPacket* _pkt = av_packet_alloc();
	if (!_pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		return;
	}
	std::shared_ptr<AVPacket> pkt = std::make_shared<AVPacket>();
	pkt.reset(_pkt, FFmpegDeleteer<AVPacket>());
	//由于从截取点开始，因此时间戳改变---下面两个vector是记录每一路我们所需要的流的从截取点开始即为时间戳的起点
	std::vector<int64_t> _dts_start_arry(iFmtCtx->nb_streams,-1);
	std::vector<int64_t> _pts_start_arry(iFmtCtx->nb_streams,-1);
	while ((av_read_frame(iFmtCtx.get(), pkt.get()) >= 0)) {
		if (stream_map.at(pkt->stream_index) < 0) {	//不是所需流
			av_packet_unref(pkt.get());
			continue;
		}
		if (_pts_start_arry.at(pkt->stream_index) == -1 && pkt->pts > 0) {	//每一路流的第一个包
			_pts_start_arry.at(pkt->stream_index) = pkt->pts;
		}
		if (_dts_start_arry.at(pkt->stream_index) == -1 && pkt->dts > 0) {	//每一路流的第一个包,记录此刻的时间戳即为开始的时间戳
			_dts_start_arry.at(pkt->stream_index) = pkt->dts;
		}
		pkt->pts = pkt->pts - _pts_start_arry.at(pkt->stream_index);
		pkt->dts = pkt->dts - _dts_start_arry.at(pkt->stream_index);
		if (pkt->pts < pkt->dts) {
			pkt->pts = pkt->dts;	//保证如果 PTS 小于 DTS，表示播放时间早于解码时间，这是不合逻辑的。
									//在这种情况下，修改 PTS 为 DTS 可以避免这种情况，确保帧的播放顺序合理。
		}
		//--------修改时间戳的方法②
		AVStream* iStream;
		AVStream* oStream;

		iStream = iFmtCtx->streams[pkt->stream_index];
		//到达截取的终止时间
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
	//8.写多媒体文件尾到输出文件的上下文中
	av_write_trailer(oFmtCtx.get());
}

void av_mix() {
	av_log_set_level(AV_LOG_DEBUG);
	AVFormatContext* _iFmtCtx1_A = nullptr;	//音频文件
	AVFormatContext* _iFmtCtx2_V = nullptr;	//视频文件
	//打开流媒体文件
	auto res_1 = avformat_open_input(&_iFmtCtx1_A, INPUTFILE_A, NULL, NULL);	//音频媒体
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

	//2.创建输出文件的上下文
	AVFormatContext* _oFmtCtx_ = nullptr;
	avformat_alloc_output_context2(&_oFmtCtx_, NULL, NULL, OUTPUTFILE);
	if (!_oFmtCtx_) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		avformat_free_context(_oFmtCtx_);
		return;
	}
	std::shared_ptr<AVFormatContext> _oFmtCtx = std::make_shared<AVFormatContext>();
	_oFmtCtx.reset(_oFmtCtx_, FFmpegDeleteer<AVFormatContext>());

	//3.找出音频流和视频流
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

	//4.为输出文件创建流并赋值
	//音频流
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
	//视频流
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
	av_dump_format(_oFmtCtx.get(), 0, OUTPUTFILE, 1);	//输出一些格式和信息
	//5.确定最大长度
	double max_duration = 0;
	if (iStream_A->duration * av_q2d(iStream_A->time_base) > 
		iStream_V->duration * av_q2d(iStream_V->time_base)) {
		max_duration = iStream_V->duration * av_q2d(iStream_V->time_base);
	}
	else {
		max_duration = iStream_A->duration * av_q2d(iStream_A->time_base);
	}
	//6.绑定并打开输出文件
	auto res_5 = avio_open2(&(_oFmtCtx->pb), OUTPUTFILE, AVIO_FLAG_WRITE, NULL, NULL);
	if (res_5 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_5, errbuff, sizeof(errbuff));
		av_log(_oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	//7.向输出文件写下文件头内容
	auto res_6 = avformat_write_header(_oFmtCtx.get(), NULL);
	if (res_6 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_6, errbuff, sizeof(errbuff));
		av_log(_oFmtCtx.get(), AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
	}
	//8.读取音频数据
	AVPacket* _pkt = av_packet_alloc();
	if (!_pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error: Memory alloced failed...\n");
		return;
	}
	std::shared_ptr<AVPacket> pkt = std::make_shared<AVPacket>();
	pkt.reset(_pkt, FFmpegDeleteer<AVPacket>());
	while ( av_read_frame(iFmtCtx1_A.get(), pkt.get() )>=0) {
		if (pkt->pts * av_q2d(iStream_A->time_base) > max_duration) {	//读取的时间大于最长时间
			av_packet_unref(pkt.get());
			continue;	//试试break;
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
	//9.写尾巴信息
	av_write_trailer(_oFmtCtx.get());
}

void av_encodec_v() {
	//1.查找编码器
	av_log_set_level(AV_LOG_DEBUG);
	const AVCodec* codec = nullptr;
	codec = avcodec_find_encoder_by_name(CODEC_NAME);
	if (!codec) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Don't find Codec: %s", CODEC_NAME);
		return;
	}
	//2.创建编码器上下文
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Memory alloc failed...\n");
		return;
	}
	//3.设置编码器参数
	ctx->height = 640;
	ctx->width = 640;
	ctx->bit_rate = 500000;//码率越大，清晰度越高，有限额的--->这里是500kbps

	ctx->time_base = av_make_q(1, 25);
	ctx->framerate = av_make_q(25, 1);

	ctx->gop_size = 10;//每10帧一组
	ctx->max_b_frames = 1;	//每组最大一个b帧
	ctx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264) {
		av_opt_set(ctx->priv_data, "preset", "slow",0);
	}
	std::shared_ptr<AVCodecContext> _ctx = std::make_shared <AVCodecContext>();
	_ctx.reset(ctx, FFmpegDeleteer<AVCodecContext>());
	//4.将编码器和编码器文件上下文绑定在一起
	auto res_1 = avcodec_open2(_ctx.get(), codec, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open codec :%s\n", errbuff);
		return;
	}
	//5.创建输出文件(二进制)
	std::ofstream outFile(CODEC_FILE_V, std::ios::out | std::ios::binary);
	if (!outFile) {
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open the file...\n");
		outFile.close();
		return;
	}
	//6.创建AVFrame
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
	//7.创建AVPacket
	AVPacket* pkt = av_packet_alloc();
	if (!pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVPacket Memory alloc failed...\n");
		outFile.close();
		return;
	}
	std::shared_ptr<AVPacket> _pkt = std::make_shared<AVPacket>();
	_pkt.reset(pkt, FFmpegDeleteer<AVPacket>());
	//8.生成视频数据（一般来说你可以获取摄像头或者抓取作为视频源，这里是为了简便）
	//这里生成25帧
	for (int i = 0; i < 25; ++i) {
		res_1=av_frame_make_writable(frame);	//确保frame的data域可用
		if (res_1 < 0) {
			break;
		}
		//对每一帧的像素点
		//Y分量
		for (int y = 0; y < _ctx->height; ++y) {
			for (int x = 0; x < _ctx->width; ++x) {
				//yuv数据处理
				_frame->data[0][y*_frame->linesize[0]+x]=x+y+i*3;
			}
		}
		//U、V分量
		for (int y = 0; y < _ctx->height / 2; ++y) {
			for (int x = 0; x < _ctx->width / 2; ++x) {
				_frame->data[1][y * _frame->linesize[1] + x] = 128 + y + i * 2;
				_frame->data[2][y * _frame->linesize[2] + x] = 64 + x + i * 5;
			}
		}
		_frame->pts = i;//标号
		res_1=encodec(_ctx.get(), _frame.get(), _pkt.get(), outFile);
		if (res_1 < 0) {
			outFile.close();
			return;
		}
	}
	//9.在编码器的缓冲区可能还残存有数据
	//在读取所有可用帧后适当处理解码器内部的缓冲区
	encodec(_ctx.get(), NULL, _pkt.get(), outFile);
	outFile.close();
	return;
}

void av_encodec_a() {
	//1.查找编码器
	av_log_set_level(AV_LOG_DEBUG);
	const AVCodec* codec = nullptr;

	//codec = avcodec_find_encoder_by_name(CODEC_NAME);
	codec=avcodec_find_encoder_by_name("libfdk_aac");	//使用第三方aac库（推荐）
	//codec=avcodec_find_encoder(AV_CODEC_ID_AAC);	//使用FFmpeg内置 ※※（解释）
	if (!codec) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Can't find Codec\n");
		return;
	}
	//2.创建编码器上下文
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Memory alloc failed...\n");
		return;
	}
	//3.设置编码器参数
	/*ctx->height = 640;
	ctx->width = 640;*/

	ctx->bit_rate = 64000;//码率越大，清晰度越高，有限额的--->这里是500kbps

	/*ctx->time_base = av_make_q(1, 25);
	ctx->framerate = av_make_q(25, 1);*/

	//ctx->gop_size = 10;//每10帧一组
	//ctx->max_b_frames = 1;	//每组最大一个b帧
	//ctx->pix_fmt = AV_PIX_FMT_YUV420P;
	
	//设置采样率
	//※※ ctx->sample_fmt=AV_SAMPLE_FMT_FLTP
	ctx->sample_fmt = AV_SAMPLE_FMT_S16;	//S:有符号；16：二字节；设置采样率格式

	if (!check_sample_fmt(codec, ctx->sample_fmt)) {	//检查编码器是否支持该采样率格式
		av_log(NULL, AV_LOG_ERROR, "Error:Encoder doesn't support sample format...\n");
		avcodec_free_context(&ctx);
		return;
	}
	ctx->sample_rate = std::stoi(select_best_sample_rate(codec));

	//设置声道
	
	//报错为：应输入表达式s
	//const AVChannelLayout* av_channel_layout = &(AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
	
	//AV_CHANNEL_LAYOUT_STEREO 是一个宏定义，它并不一定是一个实际对象的地址，
	//而可能是一个临时的结构体或者常量表达式，因此无法对其取地址。
	//const AVChannelLayout* av_channel_layout = const_cast<AVChannelLayout*>(&AV_CHANNEL_LAYOUT_STEREO);
	
	//※※ AVChannelLayout av_channel_layout = AV_CHANNEL_LAYOUT_MONO;
	AVChannelLayout av_channel_layout = AV_CHANNEL_LAYOUT_STEREO;
	av_channel_layout_copy(&ctx->ch_layout, &av_channel_layout);

	/*if (codec->id == AV_CODEC_ID_H264) {
		av_opt_set(ctx->priv_data, "preset", "slow", 0);
	}*/
	std::shared_ptr<AVCodecContext> _ctx = std::make_shared <AVCodecContext>();
	_ctx.reset(ctx, FFmpegDeleteer<AVCodecContext>());
	//4.将编码器和编码器文件上下文绑定在一起
	auto res_1 = avcodec_open2(_ctx.get(), codec, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open codec :%s\n", errbuff);
		return;
	}
	//5.创建输出文件(二进制)
	std::ofstream outFile(CODEC_FILE_A, std::ios::out | std::ios::binary);
	if (!outFile) {
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open the file...\n");
		outFile.close();
		return;
	}
	//6.创建AVFrame
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
	//7.创建AVPacket
	AVPacket* pkt = av_packet_alloc();
	if (!pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVPacket Memory alloc failed...\n");
		outFile.close();
		return;
	}
	std::shared_ptr<AVPacket> _pkt = std::make_shared<AVPacket>();
	_pkt.reset(pkt, FFmpegDeleteer<AVPacket>());
	//8.生成音频数据（一般来说你可以获取扬声器或耳机作为音频源，这里是为了简便）
	//与视频帧不同，这里是音频流
	uint16_t* samples = nullptr;
	double t = 0;
	double tincr = 2 * M_PI * 440 / _ctx->sample_rate;
	for (int i = 0; i < 200; ++i) {
		res_1=av_frame_make_writable(_frame.get());
		if (res_1 < 0) {
			av_log(NULL, AV_LOG_ERROR, "Error:Can't alloc space...\n");
			return;
		}
		samples =(uint16_t*) _frame->data[0];	//※※ FLTP为32位的(uint32_t*)※※
		for (int j = 0; j < _ctx->frame_size; ++j) {	//※※*2都改为*4
			samples[2 * j] = (int)(sin(t) * 10000);	//※※可能也要修改
			for (int k = 1; k < _ctx->ch_layout.nb_channels; ++k) {
				samples[2 * j + k] = samples[2 * j];
			}
			t += tincr;
		}
		encodec(_ctx.get(), _frame.get(), _pkt.get(), outFile);
	}
	//9.在编码器的缓冲区可能还残存有数据
	//在读取所有可用帧后适当处理解码器内部的缓冲区
	encodec(_ctx.get(), NULL, _pkt.get(), outFile);
	outFile.close();
	return;
}

void av_decodec_pix_PGM() {

	av_log_set_level(AV_LOG_DEBUG);

	//1.打开多媒体文件
	std::shared_ptr<AVFormatContext> iFmtCtx = std::make_shared<AVFormatContext>();
	//以下的初始化方式会报错-----出现内存问题
	//std::shared_ptr<AVFormatContext> iFmtCtx(NULL, FFmpegDeleteer<AVFormatContext>());
	AVFormatContext* temp_iFmtCtx = nullptr;	//不能取智能指针包裹的指针的地址，因为其返回的是一个临时值（右值）
	auto res_1 = avformat_open_input(&temp_iFmtCtx, LOCALFILE, NULL, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(nullptr, AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
		//exit(-1);智能指针不会执行析构函数，因为程序会强行终止，不回收栈了
	}
	iFmtCtx.reset(temp_iFmtCtx, FFmpegDeleteer<AVFormatContext>());

	//2.从多媒体文件找到视频流
	auto index = av_find_best_stream(iFmtCtx.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (index < 0) {
		av_log(iFmtCtx.get(), AV_LOG_ERROR, "Error:Don't include any video stream...\n");
		return;
	}

	//3.查找解码器
	const AVCodec* codec = nullptr;
	AVStream* inStream = iFmtCtx->streams[index];
	codec = avcodec_find_decoder(inStream->codecpar->codec_id);
	if (!codec) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Don't find libx264...\n");
		return;
	}

	//4.创建解码器上下文
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Memory alloc failed...\n");
		return;
	}
	avcodec_parameters_to_context(ctx, inStream->codecpar);
	std::shared_ptr<AVCodecContext> _ctx = std::make_shared <AVCodecContext>();
	_ctx.reset(ctx, FFmpegDeleteer<AVCodecContext>());

	//5.将解码器和解码器文件上下文绑定在一起
	res_1 = avcodec_open2(_ctx.get(), codec, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open codec :%s\n", errbuff);
		return;
	}

	//7.创建AVFrame
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVFrame Memory alloc failed...\n");
		return;
	}
	std::shared_ptr<AVFrame> _frame = std::make_shared<AVFrame>();
	_frame.reset(frame, FFmpegDeleteer<AVFrame>());
	
	//8.创建AVPacket
	AVPacket* pkt = av_packet_alloc();
	if (!pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVPacket Memory alloc failed...\n");
		return;
	}
	std::shared_ptr<AVPacket> _pkt = std::make_shared<AVPacket>();
	_pkt.reset(pkt, FFmpegDeleteer<AVPacket>());
	
	//9.从源多媒体读到视频数据到输出文件中
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
	//在读取所有可用帧后适当处理解码器内部的缓冲区
	//处理解码器内部的缓冲区
	decodec_pix_PGM(_ctx.get(), _frame.get(), nullptr, filename,count);
	return;
}

void av_decodec_pix_BMP() {
	av_log_set_level(AV_LOG_DEBUG);

	//1.打开多媒体文件
	std::shared_ptr<AVFormatContext> iFmtCtx = std::make_shared<AVFormatContext>();
	//以下的初始化方式会报错-----出现内存问题
	//std::shared_ptr<AVFormatContext> iFmtCtx(NULL, FFmpegDeleteer<AVFormatContext>());
	AVFormatContext* temp_iFmtCtx = nullptr;	//不能取智能指针包裹的指针的地址，因为其返回的是一个临时值（右值）
	auto res_1 = avformat_open_input(&temp_iFmtCtx, LOCALFILE, NULL, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(nullptr, AV_LOG_ERROR, "Error:%s\n", errbuff);
		return;
		//exit(-1);智能指针不会执行析构函数，因为程序会强行终止，不回收栈了
	}
	iFmtCtx.reset(temp_iFmtCtx, FFmpegDeleteer<AVFormatContext>());

	//2.从多媒体文件找到视频流
	auto index = av_find_best_stream(iFmtCtx.get(), AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (index < 0) {
		av_log(iFmtCtx.get(), AV_LOG_ERROR, "Error:Don't include any video stream...\n");
		return;
	}

	//3.查找解码器
	const AVCodec* codec = nullptr;
	AVStream* inStream = iFmtCtx->streams[index];
	codec = avcodec_find_decoder(inStream->codecpar->codec_id);
	if (!codec) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Don't find libx264...\n");
		return;
	}

	//4.创建解码器上下文
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx) {
		av_log(nullptr, AV_LOG_ERROR, "Error:Memory alloc failed...\n");
		return;
	}
	avcodec_parameters_to_context(ctx, inStream->codecpar);
	std::shared_ptr<AVCodecContext> _ctx = std::make_shared <AVCodecContext>();
	_ctx.reset(ctx, FFmpegDeleteer<AVCodecContext>());

	//5.将解码器和解码器文件上下文绑定在一起
	res_1 = avcodec_open2(_ctx.get(), codec, NULL);
	if (res_1 < 0) {
		char errbuff[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(res_1, errbuff, sizeof(errbuff));
		av_log(NULL, AV_LOG_ERROR, "Error:Don't open codec :%s\n", errbuff);
		return;
	}

	//5.1与PGM无色图片相比，BMP图片多了这个
	//有错误，会丢帧
	//AVPixelFormat pix_fmt = _ctx->pix_fmt;
	//if (pix_fmt == AV_PIX_FMT_NONE) {
	//	av_log(nullptr, AV_LOG_WARNING, "Pixel format not set yet...\n");	//可能是输入文件没有预先设置像素格式。解码器无法立即设置pix_fmt
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
	//					//解码成功,获取到pix_fmt
	//					pix_fmt = _ctx->pix_fmt;
	//					av_packet_unref(packet);
	//					av_log(nullptr, AV_LOG_WARNING, "Pixel format set successfully...\n");
	//					break;
	//				}
	//			}
	//		}
	//		av_packet_unref(packet);
	//	}
	//	//回滚到先前的位置，不妨碍后续的流程
	//	av_seek_frame(iFmtCtx.get(), index, current_pos, AVSEEK_FLAG_BACKWARD);
	//	avcodec_flush_buffers(_ctx.get());
	//	
	//	av_frame_free(&frame);
	//	av_packet_free(&packet);
	//}
	
	//6.创建一个SwsContext对象
	std::shared_ptr<struct SwsContext> swsCtx_shared_ptr(nullptr, FFmpegDeleteer<struct SwsContext>());
	
	//7.创建AVFrame
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVFrame Memory alloc failed...\n");
		return;
	}
	std::shared_ptr<AVFrame> _frame = std::make_shared<AVFrame>();
	_frame.reset(frame, FFmpegDeleteer<AVFrame>());

	//8.创建AVPacket
	AVPacket* pkt = av_packet_alloc();
	if (!pkt) {
		av_log(NULL, AV_LOG_ERROR, "Error:AVPacket Memory alloc failed...\n");
		return;
	}
	std::shared_ptr<AVPacket> _pkt = std::make_shared<AVPacket>();
	_pkt.reset(pkt, FFmpegDeleteer<AVPacket>());

	//9.从源多媒体读到视频数据到输出文件中
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
	//在读取所有可用帧后适当处理解码器内部的缓冲区
	//处理解码器内部的缓冲区
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
	
	//av_decodec_pix_BMP();//--失败

	//printf("%s\n", avcodec_configuration());
	return 0;
}