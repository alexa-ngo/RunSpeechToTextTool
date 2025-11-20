/**

This program decodes a .mkv file into a mp3 audio file

Run this program:
    ./demux-decode sample.mkv your-output-audio.mp3

*/

#include <libavutil/imgutils.h>
#include <libavutil/timestamp.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

static AVCodecContext* audio_dec_context = NULL;
static AVStream *audio_stream = NULL;
static AVFormatContext* format_context = NULL;

static AVFrame* frame = NULL;
static AVPacket* packet = NULL;

static FILE *audio_dst_file = NULL;
static const char* audio_dst_filename = NULL;
static int audio_frame_count = 0;
static int audio_stream_idx = -1;
static const char* source_filename = NULL;


static int output_audio_frame(AVFrame* frame) {
    size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(frame->format);
    //printf("audio_frame n:%d nb_samples:%d pts:%s\n", audio_frame_count++, frame->nb_samples, av_ts2timestr(frame->pts, &audio_dec_context->time_base));

    /* Write the raw audio data samples of the first plane. This works
     * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
     * most audio decoders output planar audio, which uses a separate
     * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
     * In other words, this code will write only the first audio channel
     * in these cases.
     * You should use libswresample or libavfilter to convert the frame
     * to packed data. */
    fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);

    return 0;
}

static int decode_packet(AVCodecContext *dec, const AVPacket *packet) {
    int ret = 0;

    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, packet);
    if (ret < 0) {
        fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }

    // get all the available frames from the decoder
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec, frame);
        if (ret < 0) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                return 0;

            fprintf(stderr, "Error during decoding (%s)\n", av_err2str(ret));
            return ret;
        }

        // write the frame data to output file
        ret = output_audio_frame(frame);

        av_frame_unref(frame);
    }

    return ret;
}

static int open_codec_context(int *stream_idx,
                              AVCodecContext **dec_context, AVFormatContext *format_context, enum AVMediaType type) {
    int ret, stream_index;
    AVStream *st;
    const AVCodec *dec = NULL;

    ret = av_find_best_stream(format_context, type, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                av_get_media_type_string(type), source_filename);
        return ret;
    } else {
        stream_index = ret;
        st = format_context->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            fprintf(stderr, "Failed to find %s codec\n",
                    av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        *dec_context = avcodec_alloc_context3(dec);
        if (!*dec_context) {
            fprintf(stderr, "Failed to allocate the %s codec context\n",
                    av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_context, st->codecpar)) < 0) {
            fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                    av_get_media_type_string(type));
            return ret;
        }

        /* Init the decoders */
        if ((ret = avcodec_open2(*dec_context, dec, NULL)) < 0) {
            fprintf(stderr, "Failed to open %s codec\n",
                    av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

static int get_format_from_sample_format(const char **format, enum AVSampleFormat sample_format) {
    int i;
    struct sample_format_entry {
        enum AVSampleFormat sample_format;
        const char *format_be, *format_le;}
    sample_format_entries[] = {
        { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
        { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
        { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
        { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
        { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *format = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_format_entries); i++) {
        struct sample_format_entry *entry = &sample_format_entries[i];
        if (sample_format == entry->sample_format) {
            *format = AV_NE(entry->format_be, entry->format_le);
            return 0;
        }
    }

    fprintf(stderr, "sample format %s is not supported as output format\n", av_get_sample_fmt_name(sample_format));
    return -1;
}

int main (int argc, char **argv) {

    int ret = 0;
    if (argc != 3) {
        fprintf(stderr, "usage: %s  input_file audio_output_file\n"
                "API example program to show how to read frames from an input file.\n"
                "This program reads frames from a file, decodes them, and writes decoded\n"
                "audio frames to a rawaudio file named audio_output_file.\n",
                argv[0]);
        exit(1);
    }
    source_filename = argv[1];
    audio_dst_filename = argv[2];

    /* open input file, and allocate format context */
    if (avformat_open_input(&format_context, source_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", source_filename);
        exit(1);
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(format_context, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

    if (open_codec_context(&audio_stream_idx, &audio_dec_context, format_context, AVMEDIA_TYPE_AUDIO) >= 0) {
        audio_stream = format_context->streams[audio_stream_idx];
        audio_dst_file = fopen(audio_dst_filename, "wb");
        if (!audio_dst_file) {
            fprintf(stderr, "Could not open destination file %s\n", audio_dst_filename);
            ret = 1;
            goto end;
        }
    }

    /* dump input information to stderr */
    av_dump_format(format_context, 0, source_filename, 0);

    if (!audio_stream) {
        fprintf(stderr, "Could not find audio stream in the input, aborting\n");
        ret = 1;
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    packet = av_packet_alloc();
    if (!packet) {
        fprintf(stderr, "Could not allocate packet\n");
        ret = AVERROR(ENOMEM);
        goto end;
    }

    if (audio_stream)
        printf("Demuxing audio from file '%s' into '%s'\n", source_filename, audio_dst_filename);

    /* read frames from the file */
    while (av_read_frame(format_context, packet) >= 0) {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if (packet->stream_index == audio_stream_idx)
            ret = decode_packet(audio_dec_context, packet);
        av_packet_unref(packet);
        if (ret < 0)
            break;
    }

    /* flush the decoders */
    if (audio_dec_context)
        decode_packet(audio_dec_context, NULL);

    printf("Demuxing succeeded.\n");

    if (audio_stream) {
        enum AVSampleFormat sformat = audio_dec_context->sample_fmt;
        int n_channels = audio_dec_context->ch_layout.nb_channels;
        const char *format;

        if (av_sample_fmt_is_planar(sformat)) {
            const char *packed = av_get_sample_fmt_name(sformat);
            printf("Warning: the sample format the decoder produced is planar "
                   "(%s). This example will output the first channel only.\n",
                   packed ? packed : "?");
            sformat = av_get_packed_sample_fmt(sformat);
            n_channels = 1;
        }

        if ((ret = get_format_from_sample_format(&format, sformat)) < 0)
            goto end;

        printf("Play the output audio file with the command:\n"
               "ffplay -f %s -ac %d -ar %d %s\n",
               format, n_channels, audio_dec_context->sample_rate,
               audio_dst_filename);
    }

end:
    avcodec_free_context(&audio_dec_context);
    avformat_close_input(&format_context);

    // Close the audio file
    if (audio_dst_file)
        fclose(audio_dst_file);
    av_packet_free(&packet);
    av_frame_free(&frame);

    return ret < 0;
}
