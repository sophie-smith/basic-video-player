/**
 * @file display.h
 * @brief Header for video display functions
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <libavformat/avformat.h>

// Global variables for SDL 
extern SDL_Texture *texture;
extern SDL_Renderer *renderer;

int display_init();

void render_frame(AVFrame *frame, AVCodecContext *codec_context, double fps);

#endif /* DISPLAY_H */