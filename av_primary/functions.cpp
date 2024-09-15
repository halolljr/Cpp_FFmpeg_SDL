#include "functions.h"
int encodec(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt, std::ofstream& outFile) {
	int ret = -1;
	ret = avcodec_send_frame(ctx, frame);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Error:Failed to send frame to encodec...\n");
		return -1;
	}
	while (ret >= 0) {
		ret = avcodec_receive_packet(ctx, pkt);
		//AVERROR(EAGAIN) 表示编码器或解码器当前还没有足够的数据输出一个完整的包。需要更多的输入才能生成新的包。
		//这种情况通常发生在异步处理或逐帧处理时。你需要调用 avcodec_send_frame（对于编码器）
		//或者 avcodec_send_packet（对于解码器），来提供更多数据后，再次调用 avcodec_receive_packet 以获取完整的数据包。
		
		//ret == AVERROR_EOF：
		//AVERROR_EOF 表示编码器或解码器已经到达流的结束（End of File），没有更多的帧或数据可以输出。
		//这通常是在编码或解码完成时返回的。
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return 0;
		}
		else if (ret < 0) {
			return -1;
		}
		av_log(NULL, AV_LOG_DEBUG, "Size=%d\n", pkt->size);
		outFile.write((char*)pkt->data, pkt->size);
		av_packet_unref(pkt);
	}
	return 0;
}
bool check_sample_fmt(const AVCodec* codec, enum AVSampleFormat& sample_fmt) {
	const enum AVSampleFormat* p = codec->sample_fmts;
	while (*p != AV_SAMPLE_FMT_NONE) {
		if (*p == sample_fmt) {
			return true;
		}
		p++;
	}
	return false;
}
std::string select_best_sample_rate(const AVCodec* codec) {
	const int* p = nullptr;
	int best_sample_rate = 0;
	if (!codec->supported_samplerates) {
		return std::to_string(44100);
	}
	p = codec->supported_samplerates;
	while (*p){
		if (!best_sample_rate || abs(44100 - *p) < abs(44100 - best_sample_rate)) {
			best_sample_rate = *p;
		}
		p++;
	}
	return std::to_string(best_sample_rate);
}
int decodec_pix_PGM(AVCodecContext* ctx, AVFrame* frame, AVPacket* pkt,std::string& filename,int& count) {
	int ret = -1;
	std::string temp_str;
	temp_str.reserve(10);	//提前预留空间，防止超出空间而频繁申请空间降低效率
	ret = avcodec_send_packet(ctx, pkt);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Failed to send frame to decoder...\n");
		return -1;
	}
	while (ret >= 0) {
		ret = avcodec_receive_frame(ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return 0;
		}
		else if (ret < 0) {
			return -1;
		}
		temp_str = std::format("{}-{}.pgm", filename, ctx->frame_num);
		if (savePic_PGM(frame->data[0], frame->linesize[0], frame->width, frame->height, temp_str, count)) {
			if (count >= COUNT_PIX) {
				av_packet_unref(pkt);
				return COUNT_PIX;
			}
		}
		else {
			av_packet_unref(pkt);
			return -1;
		}
		if (pkt) {
			av_packet_unref(pkt);
		}
	}
	return 0;
}

bool savePic_PGM(unsigned char* buf, int& linesize, int& width, int& height, std::string& name, int& count) {
	std::ofstream outFile;
	std::string fullpath = "images_PGM/" + name;	//目录名称
	//C++17之后引进的文件目录
	if (!std::filesystem::exists(fullpath)) {
		std::filesystem::create_directory("images_PGM");
	}
	outFile.open(fullpath, std::ios::app | std::ios::binary);	//使用二进制模式追加打开文件
	if (!outFile.is_open()) {
		std::cerr << "Can't open file : " << name << std::endl;
		return false;
	}
	// 写入 PGM (P5) 头
	outFile << "P5\n" << width << " " << height << "\n" << 255 << "\n";
	for (int i = 0; i < height; ++i) {
		outFile.write(reinterpret_cast<const char*>(buf + i * linesize), width);
	}
	count++;
	outFile.close();
	return true;
}
int decodec_pix_BMP(AVCodecContext* ctx, std::shared_ptr<struct SwsContext>& swsCtx, AVFrame* frame, AVPacket* pkt,
					std::string& filename, int& count,bool& first_time) {
	int ret = -1;
	std::string temp_str;
	temp_str.reserve(20);	//提前预留空间，防止超出空间而频繁申请空间降低效率
	ret = avcodec_send_packet(ctx, pkt);
	if (ret < 0) {
		av_log(nullptr, AV_LOG_ERROR, "Failed to send frame to decoder...\n");
		return -1;
	}
	while (ret >= 0) {
		ret = avcodec_receive_frame(ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return 0;
		}
		else if (ret < 0) {
			return -1;
		}
		temp_str = std::format("{}-{}.bmp", filename, ctx->frame_num);
		if (first_time) {
			struct SwsContext* first_swsCtx = sws_getContext(ctx->width, ctx->height, ctx->pix_fmt,	//src的数据格式
											1280, 720, AV_PIX_FMT_BGR24,	//dst的数据格式,宽高变成正规比例
											SWS_BICUBIC, NULL, NULL, NULL);
			swsCtx.reset(first_swsCtx, FFmpegDeleteer<SwsContext>());
			first_time = false;
		}
		if (savePic_BMP(swsCtx.get(), frame, frame->width, frame->height, temp_str, count)) {
			if (count >= COUNT_PIX) {
				av_packet_unref(pkt);
				return COUNT_PIX;
			}
		}
		else {
			av_packet_unref(pkt);
			return -1;
		}
		if (pkt) {
			av_packet_unref(pkt);
		}
	}
	return 0;
}
bool savePic_BMP(struct SwsContext* swsCtx, AVFrame* frame, const int& width, const int& height, std::string& name,int& count) {
	//1.先进行转化，将YUV Frame转化为 BGR24 Frame
	//错误用法--make_shared为堆上的对象调用new而且总是使用默认的delete来分配内存，不接管删除器
	//即std::make_shared<T>(args...)会使用T的构造函数并且不接管删除器
	// 像FFmpeg这种返回初始化好的指针的类型，很难去适配std::make_shared<T>(args...)
	//av_frame_alloc()会返回一个已经在堆上分配好内存的对象
	//std::shared_ptr<AVFrame> frameBGR = std::make_shared<AVFrame>(av_frame_alloc(), FFmpegDeleteer<AVFrame>());

	//①
	//std::shared_ptr<AVFrame> frameBGR = std::make_shared<AVFrame>();
	//reset()....

	//②
 
	
	//可能是这里的问题------------内存释放不对
	//std::shared_ptr<AVFrame> frameBGR(av_frame_alloc(), FFmpegDeleteer<AVFrame>());
	AVFrame* frameBGR = av_frame_alloc();
	frameBGR->width = width;
	frameBGR->height = height;
	frameBGR->format = AV_PIX_FMT_BGR24;

	av_frame_get_buffer(frameBGR,0);
	sws_scale(swsCtx, reinterpret_cast<const uint8_t* const*>(frame->data),
		frame->linesize, 0,
		frame->height, reinterpret_cast<uint8_t* const*> (frameBGR->data),	
		frameBGR->linesize);
	//2.构造BITMAPINFOHEADER
	//BITMAPINFOHEADER infoHeader = { sizeof(BITMAPINFOHEADER),width,(height * (-1)),0,24,0,0,0,0,0,0 };
	BITMAPINFOHEADER infoHeader;
	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = width;
	infoHeader.biHeight = height * -1;
	infoHeader.biBitCount = 24;
	infoHeader.biCompression = 0;
	infoHeader.biSizeImage = 0;
	infoHeader.biClrImportant = 0;
	infoHeader.biClrUsed = 0;
	infoHeader.biXPelsPerMeter = 0;
	infoHeader.biYPelsPerMeter = 0;
	infoHeader.biPlanes = 1;

	//3.构造BITMAPFILEHEADER
	int dataSize = width * height * 3;	//BGR24 24位刚好为3个字节
	
	BITMAPFILEHEADER fileHeader;
	fileHeader.bfType = 0x4d42;	//2个字符"BM"
	fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dataSize;
	fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//4.将数据写入文件中
	std::ofstream outFile;
	std::string fullpath = "images_BMP/" + name;
	if (!std::filesystem::exists(fullpath)) {
		std::filesystem::create_directory("images_BMP");
	}
	outFile.open(fullpath,std::ios::binary);
	if (!outFile) {
		std::cerr << "Can't open " << fullpath << std::endl;
		return false;
	}
	outFile.write(reinterpret_cast<const char*>(&fileHeader), sizeof(BITMAPFILEHEADER));
	outFile.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BITMAPINFOHEADER));
	//将YUV数据转化为BGR数据之后，原来的data[0]-Y,data[1]-U,data[V]-v,变为全部数据都在data[0]中
	outFile.write(reinterpret_cast<const char*>(frameBGR->data[0]), dataSize);
	outFile.close();
	/*FILE* F;
	F = fopen(fullpath.c_str(), "wb");
	fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, F);
	fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, F);
	fwrite(frameBGR->data[0], 1, dataSize, F);
	fclose(F);*/
	count++;
	av_freep(&frameBGR->data[0]);
	av_free(frameBGR);
	return true;
}