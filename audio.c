#include <stdio.h>
#include <string.h>
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

#define STREAM_DURATION   10.0

typedef struct FileContext{
    AVFormatContext *formatContext; 
    AVInputFormat *inputFormat;
    AVCodecContext *codecContext;
    AVCodec *codec;
    char  fileName[64];
    int audioIndex;
}FileContext;

typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
}OutputStream;



void add_stream(OutputStream *ost, AVFormatContext *oc,
                AVCodec **codec,
                enum AVCodecID codec_id)
{
    AVCodecContext *c;
    int i;
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec))
    {
        fprintf(stderr, "Could not find encoder for '%s'\n",
                avcodec_get_name(codec_id));
        exit(1);
    }

    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st)
    {
        fprintf(stderr, "Could not allocate stream\n");
        exit(1);
    }
    ost->st->id = oc->nb_streams-1;
    c = avcodec_alloc_context3(*codec);
    if (!c)
    {
        fprintf(stderr, "Could not alloc an encoding context\n");
        exit(1);
    }
    ost->enc = c;


    c->sample_fmt  = (*codec)->sample_fmts ?
                     (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    c->bit_rate    = 64000;
    c->sample_rate = 44100;
    if ((*codec)->supported_samplerates)
    {
        c->sample_rate = (*codec)->supported_samplerates[0];
        for (i = 0; (*codec)->supported_samplerates[i]; i++)
        {
            if ((*codec)->supported_samplerates[i] == 44100)
            {
                c->sample_rate = 44100;
            }
        }
    }
    c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
    c->channel_layout = AV_CH_LAYOUT_STEREO;
    if ((*codec)->channel_layouts)
    {
        c->channel_layout = (*codec)->channel_layouts[0];
        for (i = 0; (*codec)->channel_layouts[i]; i++)
        {
            if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
            {
                c->channel_layout = AV_CH_LAYOUT_STEREO;
            }
        }
    }
    c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
    ost->st->time_base = (AVRational){ 1, c->sample_rate };


    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
    {
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
}


int OpenInputFile(FileContext *in,const char*open_url)
{
    int ret = 0;
    size_t i = 0;
    memset(in->fileName,0,sizeof(in->fileName));
    strncpy(in->fileName,open_url,strlen(open_url));

    in->formatContext = NULL;
    in->formatContext = avformat_alloc_context();
    in->inputFormat = av_find_input_format("alsa");
    in->codecContext = NULL;
    in->codec  = NULL;

    ret = avformat_open_input(&in->formatContext,in->fileName,in->inputFormat,NULL);
    if (ret < 0)
    {
        printf("avformat_open_input failed\n");
        exit(1);
    }

    printf("avformat_open_input successful\n");
    av_dump_format(in->formatContext,0,in->fileName,0);

    if(avformat_find_stream_info(in->formatContext,NULL)<0)
    {
        printf("Couldn't find stream information.\n");
        exit(1);
    }
    printf("avformat_find_stream_info successful\n");

    for(; i < in->formatContext->nb_streams; i++)
    {
        if(in->formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            in->audioIndex=i;
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

    return 0;
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

void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    AVCodecContext *c;
    int nb_samples;
    int ret;
    AVDictionary *opt = NULL;

    c = ost->enc;

    /* open it */
    av_dict_copy(&opt, opt_arg, 0);
    ret = avcodec_open2(c, codec, &opt);
    av_dict_free(&opt);
    if (ret < 0) {
        fprintf(stderr, "Could not open audio codec: %s\n", av_err2str(ret));
        exit(1);
    }

    /* init signal generator */
    ost->t     = 0;
    ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

    if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
        nb_samples = 10000;
    else
        nb_samples = c->frame_size;

    ost->frame     = alloc_audio_frame(c->sample_fmt, c->channel_layout,
                                       c->sample_rate, nb_samples);
    ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout,
                                       c->sample_rate, nb_samples);

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }

    /* create resampler context */
    ost->swr_ctx = swr_alloc();
    if (!ost->swr_ctx) {
        fprintf(stderr, "Could not allocate resampler context\n");
        exit(1);
    }

    /* set options */
    av_opt_set_int       (ost->swr_ctx, "in_channel_count",   c->channels,       0);
    av_opt_set_int       (ost->swr_ctx, "in_sample_rate",     c->sample_rate,    0);
    av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int       (ost->swr_ctx, "out_channel_count",  c->channels,       0);
    av_opt_set_int       (ost->swr_ctx, "out_sample_rate",    c->sample_rate,    0);
    av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt",     c->sample_fmt,     0);

    /* initialize the resampling context */
    if ((ret = swr_init(ost->swr_ctx)) < 0) {
        fprintf(stderr, "Failed to initialize the resampling context\n");
        exit(1);
    }
}
AVFrame *get_audio_frame(OutputStream *ost)
{
    AVFrame *frame = ost->tmp_frame;
    int j, i, v;
    int16_t *q = (int16_t*)frame->data[0];

    /* check if we want to generate more frames */
    if (av_compare_ts(ost->next_pts, ost->enc->time_base,
                      STREAM_DURATION, (AVRational){ 1, 1 }) >= 0)
        return NULL;

    for (j = 0; j <frame->nb_samples; j++) {
        v = (int)(sin(ost->t) * 10000);
        for (i = 0; i < ost->enc->channels; i++)
            *q++ = v;
        ost->t     += ost->tincr;
        ost->tincr += ost->tincr2;
    }

    frame->pts = ost->next_pts;
    ost->next_pts  += frame->nb_samples;

    return frame;
}

int write_audio_frame(AVFormatContext *oc, OutputStream *ost)
{
    AVCodecContext *c;
    AVPacket pkt = { 0 }; // data and size must be 0;
    AVFrame *frame;
    int ret;
    int got_packet;
    int dst_nb_samples;

    av_init_packet(&pkt);
    c = ost->enc;

    frame = get_audio_frame(ost);

    if (frame) {
        /* convert samples from native format to destination codec format, using the resampler */
            /* compute destination number of samples */
            dst_nb_samples = av_rescale_rnd(swr_get_delay(ost->swr_ctx, c->sample_rate) + frame->nb_samples,
                                            c->sample_rate, c->sample_rate, AV_ROUND_UP);
            av_assert0(dst_nb_samples == frame->nb_samples);

        /* when we pass a frame to the encoder, it may keep a reference to it
         * internally;
         * make sure we do not overwrite it here
         */
        ret = av_frame_make_writable(ost->frame);
        if (ret < 0)
            exit(1);

        /* convert to destination format */
        ret = swr_convert(ost->swr_ctx,
                          ost->frame->data, dst_nb_samples,
                          (const uint8_t **)frame->data, frame->nb_samples);
        if (ret < 0) {
            fprintf(stderr, "Error while converting\n");
            exit(1);
        }
        frame = ost->frame;

        frame->pts = av_rescale_q(ost->samples_count, (AVRational){1, c->sample_rate}, c->time_base);
        ost->samples_count += dst_nb_samples;
    }

    ret = avcodec_encode_audio2(c, &pkt, frame, &got_packet);
    if (ret < 0) {
        fprintf(stderr, "Error encoding audio frame: %s\n", av_err2str(ret));
        exit(1);
    }

    if (got_packet) {
        ret = av_write_frame(oc,&pkt);
        if (ret < 0) {
            fprintf(stderr, "Error while writing audio frame: %s\n",
                    av_err2str(ret));
            exit(1);
        }
    }

    return (frame || got_packet) ? 0 : 1;
}


int OpenOutputFile(FileContext *out,const char*fileName,OutputStream *audioStream)
{
    int ret = 0;
    AVOutputFormat *outFormat = NULL;
    /*OutputStream audioStream = { 0 };*/
    out->formatContext = NULL;
    AVDictionary *opt = NULL;
    memset(out->fileName,0,sizeof(out->fileName));
    strncpy(out->fileName,fileName,strlen(fileName));

    ret = avformat_alloc_output_context2(&out->formatContext,outFormat,NULL,fileName);
    if (ret < 0)
    {
        printf("avformat_alloc_output_context2 failed\n");
    }
    printf("avformat_alloc_output_context2 successful\n");

    outFormat = out->formatContext->oformat;
    if (outFormat->audio_codec == AV_CODEC_ID_NONE)
    {
        printf("audio_codec failed\n");
        exit(1);
    }
    add_stream(audioStream,out->formatContext,&out->codec,outFormat->audio_codec);
    open_audio(out->formatContext,out->codec,audioStream,opt);

    av_dump_format(out->formatContext, 0, out->fileName, 1);

    if (!(outFormat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&out->formatContext->pb, out->fileName, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            fprintf(stderr, "Could not open '%s': %s\n", out->fileName, av_err2str(ret));
            exit(1);
        }
    }


    ret = avformat_write_header(out->formatContext, &opt);
    if (ret < 0)
    {
        fprintf(stderr, "Error occurred when opening output file: %s\n", av_err2str(ret));
        exit(1);
    }
    return 0;
}

void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->tmp_frame);
    sws_freeContext(ost->sws_ctx);
    swr_free(&ost->swr_ctx);
}

void write_file(FileContext*out,OutputStream *audioStream)
{
    int num = 0;
    while(1)
    {
    
       write_audio_frame(out->formatContext, audioStream);        
       num++;
       if (num > 1000)
       {
            printf("write_file over\n");
            break;
       }
    }
}

int main(void)
{
    FileContext in;
    FileContext out;
    OutputStream audioStream = {0};
    const char* open_url = "hw:0,0";
    /*const char* out_file = "one.aac";*/
    const char* out_file = "one.mp3";
    av_register_all();
    avdevice_register_all();

    OpenInputFile(&in,open_url);
    OpenOutputFile(&out,out_file,&audioStream);
    write_file(&out,&audioStream);

    av_write_trailer(out.formatContext);
    close_stream(out.formatContext, &audioStream);
    if (!(out.formatContext->oformat->flags & AVFMT_NOFILE))
    {
        avio_closep(&out.formatContext->pb);
    }
    avformat_free_context(out.formatContext);

    return 0;
}
