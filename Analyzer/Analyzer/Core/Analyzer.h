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

		void SDLShow(unsigned char* data);// ��ʾbgr��ͼƬ
		void SDLShow(int linesize[8], unsigned char* data[8]);// ��ʾyuv��ͼƬ

	private:
		int initSDL();
		int destorySDL();

	private:
		Scheduler* mScheduler;
		Control* mControl;

		std::vector<AlgorithmDetectObject> mDetects;

#if PLAY
		SDL_Window* mSDLWindow{}; // ����
		SDL_Renderer* mSDLRenderer{};// ��Ⱦ��
		SDL_Texture* mSDLTexture_IYUV{}; // yuv����
		SDL_Texture* mSDLTexture_BGR24{}; // bgr����
		SDL_Event mSDLEvent{};// �����¼�
		SDL_Rect  mSDLRect{}; // ��������
#endif // PLAY



	};
}
#endif //ANALYZER_ANALYZER_H

