# Basic Video Player
## Introduction
This repository contains a sample implementation of a basic video player implementing using FFmpeg and SDL. Currently it only supports a sequential implementation of single video player. Plans for future support include seeking, handling decoding frames via threads, supporting a pool of AVFrame elements to prevent unnecessary allocations, etc.

This was implemented through referencing the following GitHub tutorials: leandromoreira/ffmpeg-libav-tutorial and rambodrahmani/ffmpeg-video-player.

If anyone looks at this, please let me know if you spot any errors. I'm still becoming familiar with the library.

## Implementation
`` player.c/.h ``: Contain main execution code and all decoding/video processing routines

``display.c/.h``: Contains all SDL-related routines to render frames to the screen

## Execution
To compile the code:

``make videoplayer ``

To run the video player on a sample video, i.e. video.mov:

`` ./videoplayer video.mov ``