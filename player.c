/**
 * @file player.c
 * @brief Main file for video playing routine
 */

#include "player.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Requires filename as argument.\n");
        return -1;
    }

    // Open file 
    AVFormatContext *avfc = NULL;
    if (avformat_open_input(&avfc, argv[1], NULL, NULL) < 0) {
        printf("Error opening video file.\n");
        return -1;
    }

    int video_idx = 0;
    if (get_stream_index(avfc, &video_idx) < 0) {
        printf("Error finding video stream.\n");
        return -1;
    }

    AVCodec *codec = NULL;
    AVCodecContext *codec_context = NULL;
    if (get_decoder(avfc, video_idx, &codec, &codec_context) < 0) {
        printf("Incompatible format for decoding.\n");
        return -1;
    }

    if (display_init(codec_context) < 0) {
        printf("Error initializing SDL display.\n");
        return -1;
    }

    // TODO: Initialize frame pool and threading
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        printf("Unable to allocate video frame.\n");
        return -1;
    }

    AVPacket *packet = av_packet_alloc();
    if (!packet) {
        printf("Unable to allocate space for packet.\n");
        return -1;
    }

    int response = 0;

    SDL_Event event;
    while (av_read_frame(avfc, packet) >= 0) {

        // Poll events to ensure renders to display
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    // TODO: Add error checking here
                    return -1;
            }
        }

        // Process single frame
        if (packet->stream_index == video_idx) {
            response = decode_packet(
                packet, 
                codec_context, 
                frame, 
                av_q2d(avfc->streams[video_idx]->r_frame_rate)
            );
            if (response < 0) break;

        }
        av_packet_unref(packet);
    }

    return 0;
}

// returns the stream index
int get_stream_index(AVFormatContext *context, int *index)
{
    if (!context || !index) {
        printf("Uninitialized input variable(s).\n");
        return -1;
    }

    // Find stream info for video file
    if (avformat_find_stream_info(context, NULL) < 0) {
        printf("Error finding stream information.\n");
        return -1;
    }

    // Walk through streams until find video stream
    int video_stream_idx = -1;
    for (int i = 0; i < context->nb_streams; i++) {
        if (context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_idx = i;
            break;
        }
    }

    if (video_stream_idx == -1) {
        printf("Unable to find video stream.\n");
        return -1;
    }

    // Store the video index
    *index = video_stream_idx;

    return 0;
}

// open codec and find decoder for video stream
int get_decoder(AVFormatContext *context, int index, AVCodec **codec, AVCodecContext **codec_context) 
{
    if (!codec || !codec_context) {
        printf("Uninitialized input variable(s).\n");
        return -1;
    }

    *codec = avcodec_find_decoder(context->streams[index]->codecpar->codec_id);
    if (!codec) {
        printf("Unsupported codec.\n");
        return -1;
    }

    *codec_context = avcodec_alloc_context3(*codec);
    if (!codec_context) {
        printf("Unable to allocate codec context.\n");
        return -1;
    }

    if (avcodec_parameters_to_context(*codec_context, context->streams[index]->codecpar) < 0) {
        printf("Unable to get codec context.\n");
        avcodec_free_context(codec_context);
        return -1;
    }

    // Open codec
    if (avcodec_open2(*codec_context, *codec, NULL) < 0) {
        printf("Unable to open codec.\n");
        avcodec_free_context(codec_context);
        return -1;
    }

    return 0;
}

int decode_packet(AVPacket *packet, AVCodecContext *context, AVFrame *frame, double fps) 
{
    // supply raw packet data as input to decoder
    int response = avcodec_send_packet(context, packet);
    if (response < 0) {
        printf("Error sending packet to decoder.\n");
        return response;
    }

    while (response >= 0) {
        // return decoded output data from decoder
        response = avcodec_receive_frame(context, frame);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) break;
        if (response < 0) {
            printf("Error occurred while receiving frame from decoder.\n");
            return response;
        }

        render_frame(frame, context, fps);

    }

    return 0;
}