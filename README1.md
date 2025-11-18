# Speech to Text (STT Tool)

## Demultiplex a Video Input File to an Audio File

1. Compile the demux_decode program to be able to separate the video and the audio file
    - gcc demux_decode.c -I /usr/include/ffmpeg -lavcodec -lavutil -lavformat -o demux_decode
    - ./demux_decode coca_cola.mp4 z-vid z-audio
2. Play the z-audio file to ensure it works
    - ffplay -f f32le -ar 44100 z-audio
3. Convert the audio file to a .wav file. 
   Have the wav file follow the command in order to be used with Whisper
    - ffmpeg -f f32le 16000 -ac 2 -i z-audio z-audio-output.wav

## Input a .wav file to be Transcribed by Whisper
    - Follow these Whisper instructions: https://github.com/ggml-org/whisper.cpp?tab=readme-ov-file

    - git clone the Whisper.cpp repository
    - install the medium.en model instead of the base.en 
    - install cmake. Since I am using Fedora, use sudo dnf install cmake
    - use ./build/bin/whisper-cli -f $HOME/Downloads/MLKDream.wav



