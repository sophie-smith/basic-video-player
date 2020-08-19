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

    // Determine index corresponding to video 
    int video_idx = 0;
    if (get_stream_index(avfc, &video_idx) < 0) {
        printf("Error finding video stream.\n");
        goto cleanup_context;
    }

    // Obtain corresponding decoder
    AVCodec *codec = NULL;
    AVCodecContext *codec_context = NULL;
    if (get_decoder(avfc, video_idx, &codec, &codec_context) < 0) {
        printf("Incompatible format for decoding.\n");
        goto cleanup_context;
    }

    // Initialize SDL display
    if (display_init(codec_context) < 0) {
        printf("Error initializing SDL display.\n");
        goto cleanup_codec_context;
    }

    // Allocate frame and packets for later use
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        printf("Unable to allocate video frame.\n");
        goto cleanup_codec_context;
    }
    AVPacket *packet = av_packet_alloc();
    if (!packet) {
        printf("Unable to allocate space for packet.\n");
        goto cleanup_frame;
    }

    // Read and render frames to screen 
    int response = 0;
    SDL_Event event;
    while (av_read_frame(avfc, packet) >= 0) {

        // Poll events to ensure renders to display
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    goto done;
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

done:
    display_destroy();
    av_packet_free(&packet);
cleanup_frame:
    av_frame_free(&frame);
cleanup_codec_context:
    avcodec_free_context(&codec_context);
cleanup_context:
    avformat_close_input(&avfc);

    return 0;
}

/**
 * @brief Retrieves index corresponding to video stream 
 * 
 * @param context Additional format information
 * @param index Variable to store video stream index
 * 
 * @return Negative on error, 0 on success 
 */
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

/**
 * @brief Open codec and find decoder for video stream
 * 
 * @param context Additional format information
 * @param index Video stream index
 * @param codec Store obtained codec
 * @param codec_context Store additional codec information
 * 
 * @return Negative on error, 0 on success
 */
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

/**
 * @brief Decodes packet to obtain uncompressed frame
 * 
 * @param packet Compressed data
 * @param context Additional codec information
 * @param frame Store frame after decompressing and render to frame
 * @param fps Fps for video stream
 * 
 * @return Negative on error, 0 on success
 */
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