#include "time.h"
#include "iostream"


extern "C"
{
#include "SDL2/SDL.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>
#include <libavutil/imgutils.h>
};
using namespace std;



/*
    Description: 使用ffmpeg和sdl2进行简单视频播放器（解封装再转成yuv）
    Copyright: 2023-3-6 
*/

int main(int argc, char* argv[])
{
    //avformat_network_init();        // 初始化网络模块
    //char *VideoPath = "/home/rose/work/test_video/akiyo_qcif.h264";
    char *VideoPath = "TSU_854x480.mp4";
    //av_register_all();                  // 使用解封装前要进行注册，这是新版本的函数
    

    AVFormatContext *pFormatCtx = NULL;     // 视频文件上下文
    AVCodecContext *pCodecCtx;                      // 编解码上下文
    AVCodec *pCodec = NULL;                   // 解码器
    AVCodecParameters *pCodeParameters = NULL;   // 解码器相关参数
    AVFrame *pFrame = av_frame_alloc();             // 解码后的帧
    AVPacket *pkt = av_packet_alloc();              // 解码前的pack
    int VideoIdx = -1;              // 视频id
    int ret;                        // 检测标志

    // 初始化SDL库和相关配置
    int width = 640;
    int height = 480;
    SDL_Rect rect;                  // 渲染显示面积
    SDL_Window *win = NULL;         // 窗口
    SDL_Renderer *render = NULL;    // 着色器
    SDL_Texture *texture = NULL;    // 纹理
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        cout<<"Can not initialize SDL"<<endl;
        return -1;
    }

    // ffmpeg对数据进行处理
    // 解封装：解协议--》解格式
    pFormatCtx = avformat_alloc_context();
    if((ret=avformat_open_input(&pFormatCtx, VideoPath,NULL, NULL)) != 0)
    {
	    cout<<ret<<endl;        
	    cout<<"打开流文件失败"<<endl;
        return -1;
    }

    // 查找码流信息
    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
    {
        cout<<"不能查找到流信息"<<endl;
        return -1;
    }


    cout << "打印输入流信息：" << endl;
    av_dump_format(pFormatCtx, 0, VideoPath, 0);


    // 获取流文件
    for(int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            VideoIdx = i;
            cout<<"videoIdx: "<<VideoIdx<<endl;
            break;
        }
    }
    //VideoIdx = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, 0, NULL);

    // ***************新版本 寻找编码器********************************
    // 寻找合适的解码器（根据流拿到解码器的上下文）
    pCodecCtx = avcodec_alloc_context3(NULL);
    pCodecCtx = pFormatCtx->streams[VideoIdx]->codec;
    // 再根据上下文拿到编码器id，通过该id拿到编码器
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    //pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if(pCodec == NULL)
    {
        cout<<"编码器没有找到"<<endl;
        return -1;
    }

    //  *****************旧版本 寻找编码器********************
    /*
    pCodeParameters = pFormatCtx->streams[VideoIdx]->codecpar;      // 从视频流中获取解码器参数
    //pCodec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
    if(pFormatCtx->streams[VideoIdx]->codecpar->codec_id == AV_CODEC_ID_H264)
    {
        cout<<"teafeafefefa"<<endl;
    }
    cout<<"feafaefae"<<endl;
    pCodec = avcodec_find_decoder(pFormatCtx->streams[VideoIdx]->codecpar->codec_id);        // 获取解码器
    //pCodec = avcodec_find_decoder_by_name();
    if(!pCodec)
    {
        cout<<"Can not find decoder"<<endl;
        return -1;
    }
    // 解码器信息配置
    pCodecCtx = avcodec_alloc_context3(NULL);             // 初始化codec的上下文
    cout<<"faefaf"<<endl;
    // 将解码器里面的信息保存到解码器上下文当中
    // (1)视频流解码器上下文信息
    //pCodecCtx = pFormatCtx->streams[VideoIdx]->codec; 
    // (2)新版本： 拷贝解码器参数到pCodecCtx中
    if((ret = avcodec_parameters_to_context(pCodecCtx,pCodeParameters)) != 0)
    {
        cout<<"avcodec parameters copy failed: "<<endl;
        return -1;
    }
    cout<<"geafe"<<endl;
    */
    

    // 打开解码器, 将解码器和解码器上下文关联
    ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if(ret < 0)
    {
        cout<<"打开编码器失败"<<endl;
        return -1;
    }


    // 初始化创建一个packet保存流中的packet
    // pkt = av_packet_alloc();
    // pFrame = av_frame_alloc();

    // SDL 获取是视频的高宽
    width = pCodecCtx->width;
    height = pCodecCtx->height;
    cout<<"width "<<width<<"  height "<<height<<endl;
    // SDL 窗口初始化
    win = SDL_CreateWindow("音视频播放器",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            width, height,
                            //SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
                            SDL_WINDOW_SHOWN);
    if(win == NULL)
    {
        cout<<"Create sdl window error, "<<SDL_GetError()<<endl;
        return -1;
    }
    // 创建Renderer，并初始化
    render = SDL_CreateRenderer(win, -1, 0);
    if(!render)
    {
        cout<<"Create sdl renderer error, "<<SDL_GetError()<<endl;
        return -1;
    }
    // 创建texture，并初始化
    // SDL视频格式与纹理初始化
    Uint32 pixformat = SDL_PIXELFORMAT_IYUV;
    texture = SDL_CreateTexture(render, 
                                pixformat,
                                SDL_TEXTUREACCESS_STREAMING,
                                width, height);
    
    int fps = 30;       // 帧数
    double oneFpsTime = 1000 / fps;     // 一帧花费的时间，单位ms
    // 循环接收一个packet，并将其解码成frame
    while(av_read_frame(pFormatCtx, pkt) >=0)           // 如果读到一个帧的数据，这里是没有进行解码的帧
    {
        // 开始计时（计算一阵的时间，如果比定义的fps时间小则延时，否则不延时）
        clock_t start, end;
        start = clock();
        int i = 0;

        if(pkt->stream_index == VideoIdx)               // 如果读取到的该帧数据是视频帧
        {
            avcodec_send_packet(pCodecCtx, pkt);      // 解码是：send_packet()-->receive_frame()
            while(avcodec_receive_frame(pCodecCtx, pFrame) == 0)
            {
                i++;
                cout<<i<<endl;
                // SDL刷新纹理
                // 将yuv的二进制文件转化为纹理信息
                SDL_UpdateYUVTexture(texture, NULL, 
                                  pFrame->data[0], pFrame->linesize[0],
                                  pFrame->data[1], pFrame->linesize[1],
                                  pFrame->data[2], pFrame->linesize[2]);
                // SDL设置需要渲染目标的显示区域
                rect.x = 0;
                rect.y = 0;
                rect.w = pCodecCtx->width;
                rect.h = pCodecCtx->height;        // 窗口的全部区域都绘制

                // SDL清空渲染器的内容
                SDL_SetRenderDrawColor(render,255,255,255,255);     // 白色清空
                SDL_RenderClear(render);      
                // 将Texture的信息加载到着色器里面
                SDL_RenderCopy(render, texture, NULL, &rect);
                // 进行显示，渲染
                SDL_RenderPresent(render);
                //延时40ms，大约25帧
                //SDL_Delay(20);
            }
        }
        av_packet_unref(pkt);
        
        // SDL事件
        SDL_Event event;
        SDL_PollEvent(&event);   // 轮询事件
        if(event.type == SDL_QUIT)
        {
            return 1;
        }
        
        // 一帧画面结束时间
        end = clock();
        // 一帧画面解码+渲染总体时间
        double cost = end - start;  
        cout<<"一帧画面所需时间： "<<cost<<endl;
        // 如果一帧画面的渲染时间小于固定时间，则延时，否则不延时
        if(cost < oneFpsTime)
        {
            SDL_Delay(oneFpsTime - cost);
        }
        
    }



    // 释放的资源
    if(pFrame)
    {
        av_frame_free(&pFrame);
        pFrame = nullptr;
    }
    if(pCodecCtx)
    {
        avcodec_close(pCodecCtx);
        pCodecCtx = nullptr;
        pCodec = nullptr;
    }
    if(pFormatCtx)
    {
        avformat_free_context(pFormatCtx);
        pFormatCtx = nullptr;
    }
    if(texture)
    {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    if(render)
    {
        SDL_DestroyRenderer(render);
        render = nullptr;
    }
    if(win)
    {
        SDL_DestroyWindow(win);
        win = nullptr;
    }

    SDL_Quit();
    cout<<"Success!"<<endl;
    return 1;
    

}
