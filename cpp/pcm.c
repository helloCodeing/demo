#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "libavutil/audio_fifo.h"
#include "libavutil/time.h"

#define STREAM_DURATION   10.0

typedef struct FileContext{
    AVFormatContext     *formatContext;
    AVInputFormat       *inputFormat;
    AVCodecContext      *codecContext;
    AVCodec             *codec;
    AVFrame             *frame;
    AVFrame             *tmp_frame;
    struct SwsContext   *sws_ctx;
    struct SwrContext   *swr_ctx;
    AVStream            *stream;
    AVOutputFormat      *outFormat;
    char    fileName[64];
    int     audioIndex;
} FileContext;

void CloseContext(FileContext *ost)
{
    avcodec_free_context(&ost->codecContext);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
    avformat_free_context(ost->formatContext);
}

AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                           uint64_t channel_layout,
                           int sample_rate, int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }

    return frame;
}

int write_frame(AVFormatContext*fmt_ctx,
        const AVRational *time_base,
        AVStream *st,
        AVPacket *pkt
        )
{
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index = st->index;
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

AVFrame *AllocAudioFrame(enum AVSampleFormat sample_fmt,
                         uint64_t channel_layout,
                         int sample_rate,
                         int nb_samples)
{
    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame)
    {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples)
    {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0)
        {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }

    return frame;
}


void OpenOutputFile(FileContext  *out)
{
    int ret = 0;
    int i;

    AVDictionary        *opt        = NULL;



    ret = avformat_alloc_output_context2(&out->formatContext,out->outFormat,NULL,out->fileName);
    if (ret < 0)
    {
        printf("avformat_alloc_output_context2 failed\n");
    }
    printf("avformat_alloc_output_context2 successful\n");

    out->outFormat = out->formatContext->oformat;
    if (out->outFormat->audio_codec == AV_CODEC_ID_NONE)
    {
        printf("audio_codec failed\n");
        exit(1);
    }

    out->codec = avcodec_find_encoder(out->outFormat->audio_codec);
    if (!out->codec)
    {
        printf("Could not find encoder for '%s'\n", avcodec_get_name(out->outFormat->audio_codec));
        exit(1);
    }

    out->stream = avformat_new_stream(out->formatContext, NULL);
    if (!out->stream)
    {
        printf("Could not allocate stream\n");
        exit(1);
    }
    out->stream->id = out->formatContext->nb_streams-1;
    out->codecContext = avcodec_alloc_context3(out->codec);
    if (!out->codecContext)
    {
        printf("Could not alloc an encoding context\n");
        exit(1);
    }


    out->codecContext->sample_fmt  = (out->codec)->sample_fmts ? (out->codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    out->codecContext->bit_rate    = 64000;
    out->codecContext->sample_rate = 44100;
    if ((out->codec)->supported_samplerates)
    {
        out->codecContext->sample_rate = (out->codec)->supported_samplerates[0];
        for (i = 0; (out->codec)->supported_samplerates[i]; i++)
        {
            if ((out->codec)->supported_samplerates[i] == 44100)
            {
                out->codecContext->sample_rate = 44100;
                break;
            }
        }
    }
    out->codecContext->channels        = av_get_channel_layout_nb_channels(out->codecContext->channel_layout);
    out->codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
    if ((out->codec)->channel_layouts)
    {
        out->codecContext->channel_layout = (out->codec)->channel_layouts[0];
        for (i = 0; (out->codec)->channel_layouts[i]; i++)
        {
            if ((out->codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
            {
                out->codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
                break;
            }
        }
    }
    out->codecContext->channels        = av_get_channel_layout_nb_channels(out->codecContext->channel_layout);
    out->stream->time_base = (AVRational){ 1, out->codecContext->sample_rate };


    if (out->formatContext->oformat->flags & AVFMT_GLOBALHEADER)
    {
        out->codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }


    ret = avcodec_open2(out->codecContext, out->codec, &opt);
    av_dict_free(&opt);
    if (ret < 0)
    {
        printf("Could not open audio codec: %s\n", av_err2str(ret));
        exit(1);
    }

    int nb_samples;
    if (out->codecContext->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
    {
        nb_samples = 10000;
    }
    else
    {
        nb_samples = out->codecContext->frame_size;
    }

    out->frame     = AllocAudioFrame(out->codecContext->sample_fmt, out->codecContext->channel_layout, out->codecContext->sample_rate, nb_samples);
    out->tmp_frame = AllocAudioFrame(AV_SAMPLE_FMT_S16, out->codecContext->channel_layout, out->codecContext->sample_rate, nb_samples);


    ret = avcodec_parameters_from_context(out->stream->codecpar, out->codecContext);
    if (ret < 0)
    {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }

    out->swr_ctx = swr_alloc();
    if (!out->swr_ctx)
    {
        printf("swr_alloc failed\n");
        exit(1);
    }


    av_opt_set_int       (out->swr_ctx, "in_channel_count",   out->codecContext->channels,       0);
    av_opt_set_int       (out->swr_ctx, "in_sample_rate",     out->codecContext->sample_rate,    0);
    av_opt_set_sample_fmt(out->swr_ctx, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int       (out->swr_ctx, "out_channel_count",  out->codecContext->channels,       0);
    av_opt_set_int       (out->swr_ctx, "out_sample_rate",    out->codecContext->sample_rate,    0);
    av_opt_set_sample_fmt(out->swr_ctx, "out_sample_fmt",     out->codecContext->sample_fmt,     0);

    ret = swr_init(out->swr_ctx);
    if (ret < 0)
    {
        printf("swr_init Failed\n");
        exit(1);
    }

    av_dump_format(out->formatContext, 0, out->fileName, 1);

//    CloseContext(out);
//    free(out);
}


int OpenInputFile(FileContext *in)
{
    int ret = 0;
    size_t i = 0;


    in->formatContext = NULL;
    in->formatContext = avformat_alloc_context();
    in->inputFormat = av_find_input_format("avfoundation");
    in->codecContext = NULL;
    in->codec  = NULL;

    ret = avformat_open_input(&in->formatContext,in->fileName,in->inputFormat,NULL);
    if (ret < 0)
    {
        printf("format_open_input failed\n");
        exit(1);
    }

    printf("format_open_input successful\n");
    av_dump_format(in->formatContext,0,in->fileName,0);

    if(avformat_find_stream_info(in->formatContext,NULL)<0)
    {
        printf("Couldn't find stream information.\n");
        exit(1);
    }

    for(; i < in->formatContext->nb_streams; i++)
    {
        if(in->formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            in->audioIndex=(int)i;
            break;
        }
    }
    if(in->audioIndex==-1)
    {
        printf("Didn't find a video stream.\n");
        exit(1);
    }
    in->codecContext = in->formatContext->streams[in->audioIndex]->codec;
    in->codec = avcodec_find_decoder(in->codecContext->codec_id);
    if(in->codec == NULL)
    {
        printf("Codec not found.\n");
        exit(1);
    }
    if(avcodec_open2(in->codecContext,in->codec,NULL)<0)
    {
        printf("Could not open codec.\n");
        exit(1);
    }
    printf("avcodec_open2 successful\n");
    // CloseContext(in);
    return 0;
}


int main(void) {
    FileContext *in;
    FileContext *out;

    const char *open_url = ":0";
    const char *out_file = "hello.pcm";
    in = (FileContext*)malloc(sizeof(struct FileContext));
    memset(in, 0, sizeof(struct FileContext));

    out =(FileContext*) malloc(sizeof(struct FileContext));
    memset(out, 0, sizeof(struct FileContext));

    strncpy(in->fileName, open_url, strlen(open_url));
    strncpy(out->fileName, out_file, strlen(out_file));

    av_register_all();
    avdevice_register_all();

    OpenInputFile(in);
    //OpenOutputFile(out);
    FILE *p = NULL;
    p = fopen(out_file,"wb+");
    if(!p)
    {
        printf("open %s failed\n",out_file);
        exit(1);
    }
    AVFrame *frame = alloc_audio_frame(in->formatContext->streams[in->audioIndex]->codecpar->format,
                                       in->formatContext->streams[in->audioIndex]->codecpar->channel_layout,
                                       in->formatContext->streams[in->audioIndex]->codecpar->sample_rate,
                                       in->formatContext->streams[in->audioIndex]->codecpar->frame_size);





    AVPacket pkt;
    int64_t  frameCount = 1000;
    while(frameCount--)
    {
        int ret = av_read_frame(in->formatContext,&pkt);
        if(ret < 0)
        {
            printf("av_read_frame failed\n");
            exit(1);
        }

        if(pkt.stream_index == in->audioIndex)
        {
            avcodec_send_packet(in->codecContext,&pkt);
            if(avcodec_receive_frame(in->codecContext,frame) < 0)
            {
                printf("avcodec_receive_frame failed\n");
                fflush(stdout);
                continue;
            }
            fwrite(frame->data[0],1,frame->linesize[0],p);

        }
        av_packet_unref(&pkt);
    }

    fclose(p);

    // free ?
}


