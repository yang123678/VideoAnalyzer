#ifndef ANALYZER_ANALYZER_H
#define ANALYZER_ANALYZER_H

#include <vector>
#include <AVSAlgorithm.h>
using namespace AVSAlgorithmLib;
namespace AVSAnalyzer {
#define PLAY 0

#if PLAY
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#endif // PLAY

	struct Control;
	class Scheduler;

	class Analyzer
	{
	public:
		explicit Analyzer(Scheduler* scheduler, Control* control);
		~Analyzer();
	public:
		bool checkVideoFrame(bool check, int64_t frameCount, unsigned char* data, float& happenScore);
		bool checkAudioFrame(bool check, int64_t frameCount, unsigned char* data, int size);

		void SDLShow(unsigned char* data);// 显示bgr的图片
		void SDLShow(int linesize[8], unsigned char* data[8]);// 显示yuv的图片

	private:
		int initSDL();
		int destorySDL();

	private:
		Scheduler* mScheduler;
		Control* mControl;

		std::vector<AlgorithmDetectObject> mDetects;

#if PLAY
		SDL_Window* mSDLWindow{}; // 窗口
		SDL_Renderer* mSDLRenderer{};// 渲染器
		SDL_Texture* mSDLTexture_IYUV{}; // yuv纹理
		SDL_Texture* mSDLTexture_BGR24{}; // bgr纹理
		SDL_Event mSDLEvent{};// 监听事件
		SDL_Rect  mSDLRect{}; // 矩形区域
#endif // PLAY



	};
}
#endif //ANALYZER_ANALYZER_H

