
//@file		:	$itemname$
//@author	:	jiarong Liang
//@date		:	$time$
//@brief	:	静水流深
#include <iostream>
#include <SDL.h>
#include <random>
#include <ctime>
#include <fstream>
#include <memory>
#include <mutex>
#include <thread>
#include <filesystem>
#include "source.h"
#include "fuction.h"
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
//播放器窗口大小（可自由指定，一般按正规比例）
int SDL_WINDOW_WIDTH = 608;
int SDL_WINDOW_HEIGHT = 368;
//yuv数据格式（要严格与容器的格式一致）
const int VIDEO_WIDTH = 3840;
const int VIDEO_HEIGHT = 2160;
const std::string INPUTFILE_YUV("src_3840_2160.yuv");
const std::string INPUTFILE_PCM("src.pcm");
const std::string DIRECTORY("source");
const int BLOCK_SIZE = 4096000;
const int FREQUENCE = 44100;
const int CHANNELS = 2;
void demo_first() {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	SDL_Window* window_ = nullptr;
	window_ = SDL_CreateWindow("SDL2_WINDOW", 200, 200, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);	//X、Y坐标+宽高
	if (!window_) {
		std::cerr << "Failed to create a SDL_Window..." << std::endl;
		SDL_Quit();
		return;
	}

	SDL_Renderer* renderer_ = nullptr;
	renderer_ = SDL_CreateRenderer(window_, -1, 0);
	if (!renderer_) {
		std::cerr << "Failed to create a SDL_Renderer..." << std::endl;
		SDL_DestroyWindow(window_);
		SDL_Quit();
		return;
	}
	SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);	//RGB+透明度
	SDL_RenderClear(renderer_);	//清除窗口并渲染
	SDL_RenderPresent(renderer_);
	SDL_Delay(5000);	//滞留2秒钟(1s = 1000ms)

	SDL_DestroyWindow(window_);
	SDL_DestroyRenderer(renderer_);
	SDL_Quit();

	return;
}

void demo_second() {
	SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);

	SDL_Window* window_ = SDL_CreateWindow("SDL2_WINDOW",200,200,640,480,SDL_WINDOW_SHOWN);
	if (!window_) {
		std::cerr << "Failed to create a SDL_Window..." << std::endl;
		SDL_Quit();
		return;
	}

	SDL_Renderer* renderer_ = SDL_CreateRenderer(window_, -1, 0);
	if (!renderer_) {
		std::cerr << "Failed to create a SDL_Renderer..." << std::endl;
		SDL_DestroyWindow(window_);
		SDL_Quit();
		return;
	}

	//设置纹理
	SDL_Texture* texture_=SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
							640, 480);
	if (!texture_) {
		std::cerr << "Failed to create a SDL_Texture..." << std::endl;
		SDL_DestroyWindow(window_);
		SDL_DestroyRenderer(renderer_);
		SDL_Quit();
		return;
	}

	//随机数
	std::default_random_engine e;
	e.seed(time(0));
	std::uniform_int_distribution<int> dis_x(0, 601);
	std::uniform_int_distribution<int> dis_y(0, 451);

	//紧接着纹理创建方块
	SDL_Rect rect_;
	rect_.w = 30;
	rect_.h = 30;

	SDL_Event sdl_event;
	bool is_quit = false;
	do 
	{
		SDL_WaitEvent(&sdl_event);	//换成SDL_PollEvent()方块会不停地动
		switch (sdl_event.type)
		{
		case SDL_QUIT:
			is_quit = true;
			SDL_Log("SDL2 Exit...\n");
			break;
		default:
			SDL_Log("Event type is %d \n", sdl_event.type);
			break;
		}

		rect_.x = dis_x(e);
		rect_.y = dis_y(e);

		//设置纹理
		SDL_SetRenderTarget(renderer_, texture_);
		SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
		SDL_RenderClear(renderer_);
		//设置方块
		SDL_RenderDrawRect(renderer_, &rect_);
		SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 0);
		SDL_RenderFillRect(renderer_, &rect_);

		SDL_SetRenderTarget(renderer_, nullptr);
		SDL_RenderCopy(renderer_, texture_, NULL, NULL);

		SDL_RenderPresent(renderer_);//展示

	} while (!is_quit);

	SDL_DestroyWindow(window_);
	SDL_DestroyRenderer(renderer_);
	SDL_DestroyTexture(texture_);
	SDL_Quit();
	return;
}

void yuv_player_demo() {
	//-----播放失败原因-----
	//---YUV 文件的分辨率（宽和高）必须与原始视频文件（如 MP4）中提取出来的 YUV 数据的分辨率保持一致---
	if (SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "Couldn't initialize SDL2 - "<<SDL_GetError() << std::endl;
		return;
	}

	SDL_Window* window_ = SDL_CreateWindow("-----YUV_PLAYER-----", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	if (!window_) {
		std::cerr << "Failed to create a SDL_Window , "<<SDL_GetError() << std::endl;
		SDL_Quit();
		return;
	}

	SDL_Renderer* renderer_ = SDL_CreateRenderer(window_, -1, 0);
	if (!renderer_) {
		std::cerr << "Couldn't create a SDL_Renderer , " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window_);
		SDL_Quit();
		return;
	}

	SDL_Texture* texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_IYUV,	//IYUV(Y+U+V)---YV12(Y+V+U)
													SDL_TEXTUREACCESS_STREAMING,	//
													VIDEO_WIDTH, VIDEO_HEIGHT);
	if (!texture_) {
		std::cerr << "Couldn't create a SDL_Texture , " << SDL_GetError() << std::endl;
		SDL_DestroyWindow(window_);
		SDL_DestroyRenderer(renderer_);
		SDL_Quit();
		return;
	}
	
	const unsigned int yuv_frame_len = VIDEO_HEIGHT * VIDEO_WIDTH * 12 / 8;	//详细参考yuv420的计算大小格式（八小时码字员）
	unsigned int temp_yuv_frame_len = yuv_frame_len;
	//为什么要控制长度是16字节的倍数
	//对于某些硬件加速、缓存或 SIMD 指令来说是很重要的，可以提高处理速度和性能。
	if (yuv_frame_len & 0xF) {	//检查后四位是否是0---即检查长度是否为16的倍数
		temp_yuv_frame_len = (yuv_frame_len & 0XFFF0) + 0x10;	//低四位清零后加0x10即16,变成大于源长度的下一个16的倍数
	}
	try{
		std::unique_ptr<Uint8[]> video_buff = std::make_unique<Uint8[]>(temp_yuv_frame_len);
		std::string fullpath(DIRECTORY + "/" + INPUTFILE_YUV);
		if (!std::filesystem::exists(DIRECTORY)) {
			std::filesystem::create_directory(DIRECTORY);
		}
		std::ifstream inputFile(fullpath,std::ios::binary);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file" + INPUTFILE_YUV);
		}
		inputFile.read((char*)(video_buff.get()), yuv_frame_len);
		if (!inputFile) {
			throw std::runtime_error("Failed to read data from yuv file");
		}

		//Uint8* video_pos = video_buff.get();

		//创建线程
		std::atomic_bool thread_exit(false);
		std::mutex mtx;
		std::thread t(refresh_video_timeer, std::ref(mtx), std::ref(thread_exit));
		SDL_Event event;
		SDL_Rect rect;
		int video_buff_len = 0;
		do 
		{
			SDL_WaitEvent(&event);
			if (event.type == EventType::REFRESH_EVENT) {
				SDL_UpdateTexture(texture_, nullptr, video_buff.get(), VIDEO_WIDTH);
				//FID:If window is resize
				rect.x = 0;
				rect.y = 0;
				rect.w = SDL_WINDOW_WIDTH;
				rect.h = SDL_WINDOW_HEIGHT;
				SDL_RenderClear(renderer_);
				SDL_RenderCopy(renderer_, texture_, nullptr, &rect);
				SDL_RenderPresent(renderer_);

				//read the remain block
				if (!inputFile.read((char*)(video_buff.get()), yuv_frame_len)) {
					std::lock_guard<std::mutex> lck(mtx);
					thread_exit.store(true);
					continue;
				}
			}
			else if (event.type == SDL_WINDOWEVENT) {
				SDL_GetWindowSize(window_, &SDL_WINDOW_WIDTH, &SDL_WINDOW_HEIGHT);
			}
			else if (event.type == SDL_QUIT) {
				std::lock_guard<std::mutex> lck(mtx);
				thread_exit.store(true);
			}
			else if (event.type == EventType::QUIT_EVENT) {
				break;
			}
		} while (true);
		t.join();
		inputFile.close();
		SDL_DestroyWindow(window_);
		SDL_DestroyRenderer(renderer_);
		SDL_DestroyTexture(texture_);
		SDL_Quit();
	}
	catch (const std::bad_alloc& e){
		std::cerr << "Memory allocation failed : " << e.what() <<std::endl;
		SDL_DestroyWindow(window_);
		SDL_DestroyRenderer(renderer_);
		SDL_DestroyTexture(texture_);
		SDL_Quit();
	}
	catch (const std::exception& e) {
		std::cerr << "Error : " << e.what() << std::endl;
		SDL_DestroyWindow(window_);
		SDL_DestroyRenderer(renderer_);
		SDL_DestroyTexture(texture_);
		SDL_Quit();
	}
	return;
}

void pcm_player_demo() {

	if (SDL_Init(SDL_INIT_AUDIO)) {	//不为0--初始化失败
		SDL_Log("Failed to initial...\n");
		return;
	}
	SDL_Window* window_ = nullptr;
	SDL_Renderer* renderer_ = nullptr;
	SDL_Texture* texture_ = nullptr;
	try
	{
		std::string fullpath(DIRECTORY + "/" + INPUTFILE_PCM);	//“/”别带有空格
		if (!std::filesystem::exists(fullpath)) {
			std::filesystem::create_directory(DIRECTORY);
		}
		std::ifstream ifs(fullpath, std::ios::binary);
		if (!ifs.is_open()) {
			throw std::ios_base::failure("Failed to open file:" + fullpath);
		}
		//std::shared_ptr<Uint8> Buff(new Uint8[BLOCK_SIZE], std::default_delete<Uint8[]>());
		std::shared_ptr<Uint8[]> Buff = std::make_shared<Uint8[]>(BLOCK_SIZE);
		
		//声卡不断需求资源，调用回调函数
		SDL_AudioSpec spec;
		//检查好pcm的参数
		spec.freq = FREQUENCE;	//采样率
		spec.channels = CHANNELS;	//声道数量
		//spec.silence = 0;
		spec.format = AUDIO_S16SYS;	//采样大小
		spec.callback = read_audio_data;	//回调函数(注意格式)
		Usr_ptr_len usdata;
		spec.userdata = &usdata;	//自定义参数，将局部参数传递给回调函数（传入地址），回调函数中会回调回来--对应回调函数的udata
		if (SDL_OpenAudio(&spec, nullptr)) {	//返回0才代表成功
			throw std::runtime_error("Failed to open audio");
		}
		SDL_PauseAudio(0);	//0为播放
		//声卡需要数据时候会copy我们读出来的数据
		//设定声卡读取的时候的参数
		size_t buffer_len = 0;
		usdata.audio_pos = Buff.get();
		usdata.buffer_len = reinterpret_cast<Uint32*>(& buffer_len);
		std::cout << "----------------------start to play pcm data------------------" << std::endl;
		do 
		{
			//你可以声卡消耗多少数据就读多少数据
			//你也可以先读一大段然后判断数据是否消耗完了，消耗完了再继续读取
			ifs.read(reinterpret_cast<char*>(Buff.get()), BLOCK_SIZE);
			buffer_len = ifs.gcount();
			while (usdata.audio_pos < (Buff.get() + buffer_len)) {
				SDL_Delay(1);
			}

		} while (buffer_len != 0);
		//当声卡读完Buff的数据循环退出来到这里
		//在后续的destroy_sdl()中有SDL_CloseAduio()
		//可如果声卡数据块缓冲区还没有使用完这段数据就关闭了呢？
		ifs.close();
	}
	catch (std::bad_alloc& e)
	{
		std::cerr << "Error:" << e.what() << std::endl;
	}
	catch (std::ios_base::failure& e)
	{
		std::cerr << "Error:" << e.what() << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Error:" << e.what() << std::endl;
	}

	destroy_sdl(window_, renderer_, texture_);
	return;
}

int main(int argc, char* argv[])
{
	//demo_first();

	//demo_second();

	//yuv_player_demo();

	pcm_player_demo();

	return 0;
}