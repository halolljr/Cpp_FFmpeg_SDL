#pragma once
//@file		:	source
//@author	:	jiarong Liang
//@date		:	2024/9/18 10:35:22
//@brief	:	��ˮ����
#include <iostream>
#include <SDL.h>
//�����ļ������ں��溬�룬�������ڱ���ͷ�ļ�
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
	//��ַ��ʽ���������ȡ����(���������buffer_len����ʱ����)
	Uint8* audio_pos;
	Uint32* buffer_len;
}Usr_ptr_len;