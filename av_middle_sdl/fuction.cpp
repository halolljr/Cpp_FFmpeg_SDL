#include "fuction.h"

void refresh_video_timeer(std::mutex& mtx,std::atomic_bool& thread_exit)
{
	mtx.lock();
	thread_exit.store(false);
	mtx.unlock();
	SDL_Event event_;
	while (!thread_exit) {
		std::lock_guard<std::mutex> lck(mtx);
		event_.type = EventType::REFRESH_EVENT;
		SDL_PushEvent(&event_);
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	}
	std::lock_guard<std::mutex> lck(mtx);
	thread_exit.store(false);
	event_.type = EventType::QUIT_EVENT;
	SDL_PushEvent(&event_);
	return;
}

void destroy_sdl(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture) {
	if (window) {
		SDL_DestroyWindow(window);
	}
	if (renderer) {
		SDL_DestroyRenderer(renderer);
	}
	if (texture) {
		SDL_DestroyTexture(texture);
	}
	SDL_CloseAudio();
	SDL_Quit();
}

void read_audio_data(void* udata, Uint8* stream, int len) {
	Usr_ptr_len* audio_data = static_cast<Usr_ptr_len*>(udata);
	if (audio_data->buffer_len == 0) {
		return;
	}

	//Çå¿ÕÉù¿¨µÄ»º³åÇø
	SDL_memset(stream, 0, len);

	len = (len < (*audio_data->buffer_len)) ? len : (*audio_data->buffer_len);
	SDL_MixAudio(stream, audio_data->audio_pos, len, SDL_MIX_MAXVOLUME);

	audio_data->audio_pos += len;
	(*audio_data->buffer_len) -= len;
}
