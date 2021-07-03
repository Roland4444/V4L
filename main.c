#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <SDL2/SDL.h>
int SDLStart=0;
int main(){
    char *device_name= "/dev/video0";
    int  file_device = open(device_name, O_RDWR, 0);
    if (file_device == -1){
        printf ("%s error %d, %s\n",device_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    };
    SDL_Window *wnd;
    SDL_Renderer *render;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
                return SDL_GetError();
    printf("Init...\n");
    printf("Init random mass...\n");
    wnd = SDL_CreateWindow("Title",  800,  0, 640, 480, SDL_WINDOW_SHOWN);
    render = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED);
    while (1)
{
//setting by default 640 X 480 X2 bytes YUV2  30 fps;;
/* prepare buffer on mmap*/
        struct v4l2_requestbuffers   RQBUFF;
        RQBUFF.count = 1;
        RQBUFF.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        RQBUFF.memory = V4L2_MEMORY_MMAP;
        ioctl(file_device,VIDIOC_REQBUFS, &RQBUFF);
        /* start capturing */
        printf("@@@@ start capturing@@@@");
        struct v4l2_buffer LOADBUFF;
        memset(&LOADBUFF, 0, sizeof(LOADBUFF));
        LOADBUFF.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        LOADBUFF.memory      = V4L2_MEMORY_MMAP;
        LOADBUFF.index       = 0;
/* loop of capturing each frame */
        ioctl(file_device, VIDIOC_QUERYBUF, &LOADBUFF);
        printf("out buffer ready");
        printf("buff length %d \n", LOADBUFF.length);
        void* MYBUFF=mmap(NULL,LOADBUFF.length,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            file_device,
            LOADBUFF.m.offset);
//extract und render
        int8_t RAWBUFF[640*480];
        int8_t * o=RAWBUFF;
        int8_t * q;
        memset(&LOADBUFF, 0, sizeof(LOADBUFF));
        LOADBUFF.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        LOADBUFF.memory = V4L2_MEMORY_MMAP;
        LOADBUFF.index = 0;
        ioctl(file_device, VIDIOC_QBUF, &LOADBUFF);
        enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(file_device, VIDIOC_STREAMON, &type);
            /**************STREAM********/
        LOADBUFF.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        LOADBUFF.memory = V4L2_MEMORY_MMAP;
        while (ioctl(file_device, VIDIOC_DQBUF, &LOADBUFF) == EAGAIN)
        {   }
        q = MYBUFF;
        int x;
        int y;
        for (x=0;x<640;x++){
            for (y=0;y<480;y++){
                *o=*q;
                o++;q++;q++;
            }
        }

        int i = 0;
        int j = 0;
        int8_t* pointer=RAWBUFF;
        for (j = 0; j<y; j++)
        {
            for (i = 0; i<x; i++)
            {
                SDL_SetRenderDrawColor(render, *(pointer), *(pointer), *(pointer++), 255);
                SDL_RenderDrawPoint(render, i, j);
            }
        }
        while (1)
        {
            SDL_Event e;
            if (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT)
				break;
            SDL_RenderPresent(render);
            SDL_Delay(5);
		//SDL_RenderClear(render);
//SDL_free(render);
         //   SDLStart=1;
/*  stop capturing */
            ioctl(file_device, VIDIOC_STREAMOFF, &type);
 /* release buffers */
            ioctl(file_device,VIDIOC_REQBUFS, &RQBUFF);
            munmap(NULL,LOADBUFF.length);
 /* close the device */
            break;
        }
}
}
