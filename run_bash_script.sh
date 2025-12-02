#!/bin/sh

#echo "HI there!"
md5sum /etc/mtab > "echo $(date + "%Y_%m_%d_%I_%M_%p").log"
#gcc demux_decode.c -I /usr/include/ffmpeg -lavcodec -lavutil -lavformat -o demux_decode3
#mkdir audio
#./demux_decode3 Dream.mp4 audio/z-audio4.wav
#ffplay -f f32le -ar 44100 audio/z-audio4.wav

# Wav file needs to follow this structure in order to be used with Whisper
#ffmpeg -f f32le -ar 16000 -ac 2 -i your_input_audio your_output_audio.wav
#echo "Done running3"
#echo "$(date + "%Y_%m_%d_%I_%M_%p").log"