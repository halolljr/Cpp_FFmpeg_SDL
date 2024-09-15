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
const int SDL_WINDOW_WIDTH = 608;
const int SDL_WINDOW_HEIGHT = 368;
const int VIDEO_WIDTH = 608;
const int VIDEO_HEIGHT = 368;
const std::string INPUTFILE("");
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

	if (SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "Couldn't initialize SDL2 - "<<SDL_GetError() << std::endl;
		return;
	}

	SDL_Window* window_ = SDL_CreateWindow("YUV_PLAYER", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT,SDL_WINDOW_SHOWN);
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
		
		std::ifstream inputFile(INPUTFILE,std::ios::binary);
		if (!inputFile.is_open()) {
			throw std::runtime_error("Failed to open file" + INPUTFILE);
		}
		inputFile.read(reinterpret_cast<char*>(video_buff.get()), yuv_frame_len);
		if (!inputFile) {
			throw std::runtime_error("Failed to read data from yuv file");
		}

		Uint8* video_pos = video_buff.get();

		//创建线程
		SDL_Event event;
		do 
		{
			SDL_WaitEvent(&event);
			switch (event.type)
			{
			default:
				break;
			}


		} while (true);
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

int main(int argc, char* argv[])
{
	//demo_first();

	demo_second();

	//yuv_player_demo();
	return 0;
}