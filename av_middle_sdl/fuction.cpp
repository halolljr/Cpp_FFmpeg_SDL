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
