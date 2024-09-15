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
		//AVERROR(EAGAIN) ��ʾ���������������ǰ��û���㹻���������һ�������İ�����Ҫ�����������������µİ���
		//�������ͨ���������첽�������֡����ʱ������Ҫ���� avcodec_send_frame�����ڱ�������
		//���� avcodec_send_packet�����ڽ������������ṩ�������ݺ��ٴε��� avcodec_receive_packet �Ի�ȡ���������ݰ���
		
		//ret == AVERROR_EOF��
		//AVERROR_EOF ��ʾ��������������Ѿ��������Ľ�����End of File����û�и����֡�����ݿ��������
		//��ͨ�����ڱ����������ʱ���صġ�
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
	temp_str.reserve(10);	//��ǰԤ���ռ䣬��ֹ�����ռ��Ƶ������ռ併��Ч��
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
	std::string fullpath = "images_PGM/" + name;	//Ŀ¼����
	//C++17֮���������ļ�Ŀ¼
	if (!std::filesystem::exists(fullpath)) {
		std::filesystem::create_directory("images_PGM");
	}
	outFile.open(fullpath, std::ios::app | std::ios::binary);	//ʹ�ö�����ģʽ׷�Ӵ��ļ�
	if (!outFile.is_open()) {
		std::cerr << "Can't open file : " << name << std::endl;
		return false;
	}
	// д�� PGM (P5) ͷ
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
	temp_str.reserve(20);	//��ǰԤ���ռ䣬��ֹ�����ռ��Ƶ������ռ併��Ч��
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
			struct SwsContext* first_swsCtx = sws_getContext(ctx->width, ctx->height, ctx->pix_fmt,	//src�����ݸ�ʽ
											1280, 720, AV_PIX_FMT_BGR24,	//dst�����ݸ�ʽ,��߱���������
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
	//1.�Ƚ���ת������YUV Frameת��Ϊ BGR24 Frame
	//�����÷�--make_sharedΪ���ϵĶ������new��������ʹ��Ĭ�ϵ�delete�������ڴ棬���ӹ�ɾ����
	//��std::make_shared<T>(args...)��ʹ��T�Ĺ��캯�����Ҳ��ӹ�ɾ����
	// ��FFmpeg���ַ��س�ʼ���õ�ָ������ͣ�����ȥ����std::make_shared<T>(args...)
	//av_frame_alloc()�᷵��һ���Ѿ��ڶ��Ϸ�����ڴ�Ķ���
	//std::shared_ptr<AVFrame> frameBGR = std::make_shared<AVFrame>(av_frame_alloc(), FFmpegDeleteer<AVFrame>());

	//��
	//std::shared_ptr<AVFrame> frameBGR = std::make_shared<AVFrame>();
	//reset()....

	//��
 
	
	//���������������------------�ڴ��ͷŲ���
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
	//2.����BITMAPINFOHEADER
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

	//3.����BITMAPFILEHEADER
	int dataSize = width * height * 3;	//BGR24 24λ�պ�Ϊ3���ֽ�
	
	BITMAPFILEHEADER fileHeader;
	fileHeader.bfType = 0x4d42;	//2���ַ�"BM"
	fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dataSize;
	fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	//4.������д���ļ���
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
	//��YUV����ת��ΪBGR����֮��ԭ����data[0]-Y,data[1]-U,data[V]-v,��Ϊȫ�����ݶ���data[0]��
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