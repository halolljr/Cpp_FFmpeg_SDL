#pragma once
//@file		:	source
//@author	:	jiarong Liang
//@date		:	2024/9/18 10:35:22
//@brief	:	静水流深
#include <iostream>
#include <SDL.h>
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


enum EventType {
	REFRESH_EVENT = SDL_USEREVENT + 1,
	QUIT_EVENT = SDL_USEREVENT + 2
};
typedef struct USERDATA_HEAD_LENGTH  
{
	//地址形式，可以灵活取数据(尤其是如果buffer_len会随时更新)
	Uint8* audio_pos;
	Uint32* buffer_len;
}Usr_ptr_len;