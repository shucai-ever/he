




#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
 
#include "faac.h"
#include <math.h>



#include "typeport.h"
#include "encoder.h"
#include "fmp4_encode.h"

 
#define  JOSEPH_G711A_LOCATION  "/jffs0/rawaudio"
#define  JOSEPH_H264_LOCALTION "/jffs0/test0.h264"
#define  JOSEPH_MP4_FILE   "/jffs0/test.mp4"
 
#define  MP4_DETAILS_ALL     0xFFFFFFFF
#define  NALU_SPS  0
#define  NALU_PPS  1
#define  NALU_I    2
#define  NALU_P    3
#define  NALU_SET  4

#define INPUT_BUFFER_SIZE  (1024*100)   //(1048576)   //(1024*100) 

 
typedef unsigned int  uint32_t;
typedef unsigned char   uint8_t;
 
/*
    判断nalu 包的类型
    返回：
        NALU_SPS  0
        NALU_PPS  1
        NALU_I    2
        NALU_P    3
        NALU_SET  4
        
*/
int nalu_type(unsigned char* naluData, int naluSize)
{
    int index = -1; 
     
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x67)
    {
        index = NALU_SPS;
        printf("%s[%d]====NALU_SPS\n",__FUNCTION__,__LINE__);
    }

    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x68)
    {
        index = NALU_PPS;
        printf("%s[%d]====NALU_PPS\n",__FUNCTION__,__LINE__);
    }
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x65)
    {
        index = NALU_I;
        printf("%s[%d]====NALU_I\n",__FUNCTION__,__LINE__);
    }
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x61)
    {
        index = NALU_P;
        printf("%s[%d]====NALU_P\n",__FUNCTION__,__LINE__);
    }
    if(naluData[0]==0 && naluData[1]==0 && naluData[2]==0 && naluData[3]==1 && naluData[4]==0x6)
    {
        index = NALU_SET;
        printf("%s[%d]====NALU_SET\n",__FUNCTION__,__LINE__);
    }

    return index;
}


//用来以十六进制字节流打印box
void print_array(unsigned char* box_name,unsigned char*start,unsigned int length)
{
    unsigned char*p = start;
    unsigned int i = 0;
    printf("\n  %s[]: ",box_name);
    for(i=0;i<length;i++)
    {
        printf("%x ",p[i]);
    }
    printf("\n");

}


/*********************************************main**************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define RECODE_STREAM_ID 0     //进行录像的流id       ： 0或者1
#define VIDEO_RECORD_TIME 15   //录像时长 单位：S    
#define FIND_IDR_MAX_NUM  200  //为了寻找IDR的的最大循环次数

#define AUDIO_SAMPLING_RATE (16000) //音频的原始采样率（PCM）
#define V_FRAME_RATE  (15)  //视频帧帧率
/*-----------------------------------------------------------------
1.使用 faac库：
    44.1KHZ A_FRAME_RATE = 27 帧/秒
    16KHZ   A_FRAME_RATE = 14 帧/秒
----------------------------------------------------------------*/
#define A_FRAME_RATE  (14)  //aac音频数据帧率

//void* fmp4_record(void* args)
fmp4_out_info_t* fmp4_record(fmp4_out_info_t* info)
{
    printf("\n\n******start fmp4_record************************************************************************\n");
    DEBUG_LOG("start fmp4_record..... \n");  

    //sleep(5); //如若刚开机，不要太快开始录像
 
    unsigned int skip_len = 0;
    unsigned int frame_len = 0;
    struct timeval start_record_time = {0};
    struct timeval tmp_time = {0};
    ENC_STREAM_PACK *pack = NULL;
    FRAME_HDR *header  = NULL;
    int  stream_id;
   
    //--request stream--------------------------------------
    stream_id = encoder_request_stream(0, RECODE_STREAM_ID, 1);
    
    while(1)  //保证第一帧是视频关键帧
    {
        printf("search to IDR frame...\n");
        pack = encoder_get_packet(stream_id);
        header = (FRAME_HDR *) pack->data;
        
        if(header->type != 0xF8)//寻找IDR帧
        {
            printf("release pack ...");
            encoder_release_packet(pack);
            continue;
        }
        else
        {
            printf("IDR frame finded*******!\n");
            break;
        }       
    }

    skip_len = sizeof (FRAME_HDR) + sizeof (IFRAME_INFO);
    frame_len = pack->length - skip_len;   
    char * IDR_frame = (char*)pack->data + skip_len;
    DEBUG_LOG("IDR_frame[]= %x %x %x %x %x %x\n",IDR_frame[0],IDR_frame[1],IDR_frame[2],IDR_frame[3],IDR_frame[4],IDR_frame[5]);           
    int ret = Fmp4_encode_init(info,IDR_frame , frame_len ,V_FRAME_RATE, A_FRAME_RATE , AUDIO_SAMPLING_RATE);
    if(ret < 0)
    {
        ERROR_LOG("fmp4 encode init failed!\n");
        goto InitFmp4Encoder_Failed;
        
    }
    
    //---debug 部分-------------------------------------------------------------------------
    #if 0   //保存找到的第一帧I帧数据到文件
        int tmp_fd = open("/jffs0/IDR_NALU.bin",O_RDWR|O_CREAT|O_TRUNC,0777);
        if(tmp_fd < 0)
        {
            ERROR_LOG("open failed !\n");
            goto other_error;
        }

        skip_len = sizeof (FRAME_HDR) + sizeof (IFRAME_INFO);
        frame_len = pack->length - skip_len;
            
        int tmp_ret = write(tmp_fd, pack->data + skip_len ,frame_len);
        if(tmp_ret != frame_len)
        {
            ERROR_LOG("write error!\n");
            close(tmp_fd);
            goto other_error;
        }
        close(tmp_fd);
    #endif

    gettimeofday(&start_record_time,NULL);
    memcpy(&tmp_time,&start_record_time,sizeof(start_record_time));
    printf("seconds:%ld  microseconds:%ld\n",start_record_time.tv_sec,start_record_time.tv_usec);
    int cucle_num = 0;
    unsigned int A_count = 0;
    unsigned int V_count = 0;
    
    IFRAME_INFO * start_info = (IFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
    unsigned long long int start_time = start_info->pts_msec;
    unsigned long long int cur_time = start_time;
    printf("start_time = (%llu) cur_time = (%llu)\n",start_time,cur_time);
     struct timeval audio_time_start = {0};
     struct timeval video_time_start = {0};
     struct timeval audio_time_current = {0};
     struct timeval video_time_current = {0};
     gettimeofday(&audio_time_start,NULL);
     gettimeofday(&video_time_start,NULL);
   // while(tmp_time.tv_sec - start_record_time.tv_sec < VIDEO_RECORD_TIME)
   while(1)//录制的时长用视频帧的帧头时间来进行计时比较准确
    {
        //找到IDR帧，开始录制
        
         gettimeofday(&audio_time_current,NULL);
        if(audio_time_current.tv_sec - audio_time_start.tv_sec >=1)//1s 打印一次
        {
            printf("count Aframe = %d\n",A_count);
            memcpy(&audio_time_start,&audio_time_current,sizeof(struct timeval));
            A_count = 0;
        }
         gettimeofday(&video_time_current,NULL);
        if(video_time_current.tv_sec - video_time_start.tv_sec >=1)//1s 打印一次
        {
            printf("count Vframe = %d\n",V_count);
            memcpy(&video_time_start,&video_time_current,sizeof(struct timeval));
            V_count = 0;
        }
        
         if (header->type == 0xFA)//0xFA-音频帧
        {
            A_count ++;
            #if 1
                skip_len = sizeof (FRAME_HDR) + sizeof (AFRAME_INFO);
                frame_len = pack->length - skip_len; 
                AFRAME_INFO * A_info = (AFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
                // printf("Audio pts : %lld\n",A_info->pts_msec);
                    
                if(Fmp4AEncode((unsigned char*)pack->data + skip_len,frame_len, A_FRAME_RATE,A_info->pts_msec))
                {
                    ERROR_LOG("Fmp4AEncode failed !\n");
                    goto other_error;
                }
            
            #endif

        }
        else if(header->type == 0xF8)   //0xF8-视频关键帧 
        {
            V_count ++;
            // printf("I frame ========\n");
            #if 1
                skip_len = sizeof (FRAME_HDR) + sizeof (IFRAME_INFO);
                frame_len = pack->length - skip_len;    
                IFRAME_INFO * V_info = (IFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
                cur_time = V_info->pts_msec;
                if(Fmp4VEncode((unsigned char*)pack->data + skip_len,frame_len,V_FRAME_RATE,V_info->pts_msec))
                {
                    ERROR_LOG("Fmp4VEncode failed !\n");
                    goto other_error;
                }
            
            #endif

        }
        else if(header->type == 0xF9)//0xF9-视频非关键帧
        {
            V_count ++;
           // printf("P frame\n");
            #if 1
                skip_len = sizeof (FRAME_HDR) + sizeof (PFRAME_INFO);
                frame_len = pack->length - skip_len;    
                PFRAME_INFO * V_info = (PFRAME_INFO*)(pack->data + sizeof(FRAME_HDR));
                cur_time = V_info->pts_msec;
                if(Fmp4VEncode((unsigned char*)pack->data + skip_len,frame_len,V_FRAME_RATE,V_info->pts_msec))
                {
                    ERROR_LOG("Fmp4VEncode failed !\n");
                    goto other_error;
                }
            
            #endif
        }
        else
        {
            ERROR_LOG("unknown frame type!\n");
        }

        encoder_release_packet(pack);
        GET_PACK:
        pack = encoder_get_packet(stream_id);
        header = (FRAME_HDR *) pack->data;
        //printf("cur_time(%llu)  start_time(%llu)\n",cur_time,start_time);
        if(cur_time - start_time >= /*VIDEO_RECORD_TIME*/ info->recode_time*1000) 
        {
            if(pack)  encoder_release_packet(pack);
            break; //录制完成
        }
            
        
    }

    DEBUG_LOG("End of recording time!\n");

    DEBUG_LOG("The fmp4_record exit !\n");
    encoder_free_stream(stream_id);
    /* 手动命令行录制视频该操作需放在这进行
    free(info.buf_mode.buf_start);
    info.buf_mode.buf_start = NULL;
    */
    Fmp4_encode_exit();
    printf("\n******END fmp4_record************************************************************************\n\n");
    return info;
    
other_error:
    if(pack)  encoder_release_packet(pack);
    encoder_free_stream(stream_id);
InitFmp4Encoder_Failed:
    //encode part
    Fmp4_encode_exit();
    
    /* 手动命令行录制视频该操作需放在这进行
    free(info.buf_mode.buf_start);
    info.buf_mode.buf_start = NULL;
    */
   
    printf("\n******ERR fmp4_record !!!************************************************************************\n\n");
    return NULL;
}


void fmp4_record_exit(fmp4_out_info_t *info)
{
    if(info != NULL)
    {
        if(info->buf_mode.buf_start != NULL)
        {
            free(info->buf_mode.buf_start);
            info->buf_mode.buf_start = NULL;
        }

        if(info->file_mode.file_name != NULL)
        {
            //没有申请内存
        }
    }

    //free(info);
    //info = NULL;
    
}
























































