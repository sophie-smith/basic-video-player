/**
 * @file display.c
 * @brief SDL Video Display
 */

#include "display.h"

SDL_Texture *texture;
SDL_Renderer *renderer;

// Creates SDL renderer and texture to update frames
int display_init(AVCodecContext *codec_context) 
{
    if (!codec_context) {
        printf("Uninitialized input variable.\n");
        return -1;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        printf("Error initializing SDL - %s\n", SDL_GetError());
        return -1;
    }

    // Create SDL Window
    SDL_Window *window = SDL_CreateWindow(
        "SDL Video Player",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        codec_context->width / 2,
        codec_context->height / 2,
        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!window) {
        printf("Unable to create video window.");
        return -1;
    }

    SDL_GL_SetSwapInterval(1);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    texture = SDL_CreateTexture(
        renderer, 
        SDL_PIXELFORMAT_YV12,
        SDL_TEXTUREACCESS_STREAMING,
        codec_context->width,
        codec_context->height
    );

    if (!renderer || !texture) {
        printf("Error creating renderer and/or texture.\n");
        return -1;
    }

    return 0;
}

void render_frame(AVFrame *frame, AVCodecContext *codec_context, double fps) {
    if (!frame) return;

    // Delay each frame by proper frame rate
    SDL_Delay(fps);

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = codec_context->width;
    rect.h = codec_context->height;

    SDL_UpdateYUVTexture(
        texture,
        &rect,
        frame->data[0],
        frame->linesize[0],
        frame->data[1],
        frame->linesize[1],
        frame->data[2],
        frame->linesize[2]
    );

    SDL_RenderClear(renderer);
    SDL_RenderCopy(
        renderer,
        texture,
        NULL,
        NULL
    );

    // Update screen with any rendering performed since previous call
    SDL_RenderPresent(renderer);
}