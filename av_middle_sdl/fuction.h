#pragma once
//@file		:	fuction
//@author	:	jiarong Liang
//@date		:	2024/9/18 10:38:13
//@brief	:	静水流深
#include <iostream>
#include <SDL.h>
#include <mutex>
#include "source.h"
//库外文件必须在后面含入，不能先于本机头文件
extern "C" {

#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avdevice.lib")
#pragma comment (lib, "avfilter.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "swresample.lib")
#pragma comment (lib, "swscale.lib")
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>

}

void refresh_video_timeer(std::mutex& mtx,std::atomic_bool& thread_exit);
void destroy_sdl(SDL_Window* window,SDL_Renderer* renderer,SDL_Texture* texture);
void read_audio_data(void* udata, Uint8* stream, int len);