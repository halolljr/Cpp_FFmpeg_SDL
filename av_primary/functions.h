#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <filesystem>
#include <memory>
#include "source.h"
const int COUNT_PIX = 5;
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
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
}
int encodec(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt, std::ofstream& outFile);
bool check_sample_fmt(const AVCodec* codec, enum AVSampleFormat& sample_fmt);
std::string select_best_sample_rate(const AVCodec* codec);
int decodec_pix_PGM(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt,std::string& filename,int& count);
bool savePic_PGM(unsigned char* buf, int& linesize, int& width, int& height, std::string& name, int& count);
int decodec_pix_BMP(AVCodecContext* ctx, std::shared_ptr<struct SwsContext>& swsCtx,AVFrame* frame, AVPacket* pkt, std::string& filename, int& count,bool& fist_time);
bool savePic_BMP(struct SwsContext* swsCtx,AVFrame* frame,const int& width,const int& height,std::string& name,int& count);