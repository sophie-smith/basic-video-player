/**
 * @file player.h
 * @brief Header for main video player functions
 */

#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include "display.h"
#include "framepool.h"
#include <libavformat/avformat.h>
#include <stdio.h>
#include <stdbool.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

int get_stream_index(AVFormatContext *context, int *index);

int get_decoder(AVFormatContext *context, int index, AVCodec **codec, AVCodecContext **codec_context);

int decode_packet(AVPacket *packet, AVCodecContext *context, AVFrame *frame, double fps);

#endif /* VIDEOPLAYER_H */