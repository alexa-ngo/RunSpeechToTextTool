#!/bin/sh


# bash script would take in an absolute path.

# ./demux_decode Dream.mp4 absolute_filepath_of_demux_audio
# ./demux_decode Dream.mp4 /home/ango/Code/RunSpeechToTextTool/171414-audio
./demux_decode $1 $2

# ffmpeg -f f32le 16000 -ac 2 -i Dream.mp4 /home/ango/Code/RunSpeechToTextTool/videos/171414-output.wav
ffmpeg -f f32le 16000 -ac 2 -i $1 $2
