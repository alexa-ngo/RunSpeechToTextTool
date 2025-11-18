#!/bin/sh

echo "HI there!"
gcc demux_decode.c -I /usr/include/ffmpeg -lavcodec -lavutil -lavformat -o demux_decode
./demux_decode MLKDream-old.wav z-vid z-audio
ffplay -f f32le -ar 44100 z-audio

# Wav file needs to follow this structure in order to be used with Whisper
ffmpeg -f f32le -ar 16000 -ac 2 -i your_input_audio your_output_audio.wav
