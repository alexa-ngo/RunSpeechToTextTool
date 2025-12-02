#!/bin/sh

./demux_decode Dream.mp4 171414-audio
ffmpeg -f f32le 16000 -ac 2 -i 171414-audio 171414-output.wav