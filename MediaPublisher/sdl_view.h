#ifndef SDL_VIEW_H
#define SDL_VIEW_H

#include "ivideo_view.h"

struct SDL_Window;
struct SDL_Texture;
struct SDL_Renderer;
class SDLView : public IVideoView
{
public:
	virtual int InitView(int width,int height,PixFormat fmt,void* win_id)override;
	virtual int DestoryView()override;
	virtual int ResetView();
protected:
	int InitSdlLibrary();
	virtual int DrawView(uint8_t* buffer, int size)override;
	virtual int DrawView(uint8_t* y_buffer, int y_size, uint8_t* u_buffer, int u_size, uint8_t* v_buffer, int v_size)override;
private:
	bool is_init_lib_ = false;
	SDL_Window* window_ = nullptr;
	SDL_Texture* texture_ = nullptr;
	SDL_Renderer* renderer_ = nullptr;
};

#endif