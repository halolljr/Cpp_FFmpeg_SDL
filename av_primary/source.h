#pragma once
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
#include <libswscale/swscale.h>
}
template <typename T>
class FFmpegDeleteer {
public:
	void operator()(T* ptr) const {
		if (ptr) {
			delete ptr;
		}
	}
};
template<>
class FFmpegDeleteer<AVFormatContext> {
public:
	void operator()(AVFormatContext* ptr) const{
		if (ptr) { 
			if (ptr->iformat) {
				// ���������������
				avformat_close_input(&ptr);
			}
			else if (ptr->oformat) {
				// ��������������
				if (ptr->pb && !(ptr->oformat->flags & AVFMT_NOFILE)) {
					avio_closep(&ptr->pb);  // �ر� IO
				}
				avformat_free_context(ptr);  // �ͷ����������
			}
		}
	}
};
template<>
class FFmpegDeleteer<AVPacket> {
public:
	void operator()(AVPacket* pkt) const{
		if (pkt) {
			av_packet_free(&pkt);
		}
	}
};
//AVCodec ����Ҫ�ͷš�������Ϊ��ֻ�������Ե���Ϣ�����Ƕ�̬������ڴ档
//template<>
//class FFmpegDeleteer<AVCodec>{
//public:
//	void operator()(AVCodec* ptr) const{
//		if (ptr) {
//
//		}
//	}
//};
template<>
class FFmpegDeleteer<AVCodecContext> {
public:
	void operator()(AVCodecContext* ptr)const {
		if (ptr) {
			avcodec_free_context(&ptr);
		}
	}
};
template<>
class FFmpegDeleteer<AVFrame> {
public:
	void operator()(AVFrame* ptr)const {
		if (ptr) {
			av_frame_free(&ptr);
		}
	}
};
template<>
class FFmpegDeleteer<SwsContext> {
public:
	void operator()(SwsContext* ptr)const {
		if (ptr) {
			sws_freeContext(ptr);
			ptr = nullptr;
		}
	}
};
//����ʹ������ָ�룬��Ҳ���Է�װ�࣬���ҷ���RAII����Դ��ȡ����ʼ���������ģʽ
class AVFormatContextWrapper {
public:
	AVFormatContext* ptr;
	//���캯������ʼ��
	explicit AVFormatContextWrapper(const char* url) :ptr(nullptr) {
		int ret = avformat_open_input(&ptr, url, nullptr, nullptr);
		if ( ret < 0) {
			ptr = nullptr;
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_strerror(ret, errbuf, sizeof(errbuf));
			av_log(nullptr, AV_LOG_ERROR, "Error:%s\n", errbuf);
			//std::cerr << "Error: Could not open input file " << url << " (" << errbuf << ")" << std::endl;
		}
	}
	~AVFormatContextWrapper() {
		if (ptr) {
			if (ptr->iformat) {
				// ���������������
				avformat_close_input(&ptr);
			}
			else if (ptr->oformat) {
				// ��������������
				if (ptr->pb && !(ptr->oformat->flags & AVFMT_NOFILE)) {
					avio_closep(&ptr->pb);  // �ر� IO
				}
				avformat_free_context(ptr);  // �ͷ����������
			}
		}
	}
	//���ÿ������캯��
	AVFormatContextWrapper(const AVFormatContextWrapper&) = delete;
	//���ÿ�����ֵ����
	AVFormatContextWrapper& operator=(const AVFormatContextWrapper&) = delete;
	//�����ƶ����캯��
	AVFormatContextWrapper(AVFormatContextWrapper&& other) noexcept :ptr(other.ptr) {
		other.ptr = nullptr;
	}
	//�����ƶ���ֵ����
	AVFormatContextWrapper& operator=(AVFormatContextWrapper&& other)noexcept {
		if (this != &other) {
			this->~AVFormatContextWrapper();
			ptr = other.ptr;
			other.ptr = nullptr;
		}
		return *this;
	}
};
using DWORD = uint32_t;
using WORD = uint16_t;
using LONG = int32_t;
typedef struct tagBITMAPINFOHEADER {
	DWORD biSize;
	LONG  biWidth;
	LONG  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG  biXPelsPerMeter;
	LONG  biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER, * LPBITMAPINFOHEADER, * PBITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER {
	WORD  bfType;
	DWORD bfSize;
	WORD  bfReserved1;
	WORD  bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER, * LPBITMAPFILEHEADER, * PBITMAPFILEHEADER;