// arm-none-linux-gnueabi-gcc -o v4l2_test v4l2_test.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h> 
#include <linux/videodev2.h>

#include "fast.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct buffer {
    void * start;
    size_t length;
};

struct point {
	unsigned int x;
	unsigned int y;
};

static char * dev_name = "/dev/video1";		//AR.Drone front camera
//static char * dev_name = "/dev/video2";	//AR.Drone bottom camera
static int fd = -1;
static unsigned int WIDTH = 1280;
static unsigned int HEIGHT = 720;
struct buffer * buffers = NULL;
static unsigned int n_buffers = 0;
FILE *file_fd;
static unsigned long image_size;
static unsigned char *file_name;

// Gets a frame. Executes in 6.7ms for front camera
static int read_frame(void)
{
    struct v4l2_buffer buf;
    unsigned int i;

    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) 
	{
		printf("ioctl() VIDIOC_DQBUF failed.\n");
		return -1;    
    }

    assert(buf.index < n_buffers);
    printf ("Buffer: index = %d\n", buf.index);

    fwrite(buffers[buf.index].start, image_size, 1, file_fd);

    if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) 
	{
		printf("ioctl() VIDIOC_QBUF failed.\n");
		return -1;    
    }

    return 1;
}

/* 	generate a half size image by averaging 2x2 blocks of pixels together
	resulting value is guaranteed to be correct to within +/- 0.5 but note:
	1 2                        1 1
	1 2  averages to 1         2 2  averages to 2

void halfsize(unsigned char* halfim, unsigned char* im, int width, int height)
{
	int ix, iy;
	for(iy=0; iy<height/2; iy++)
	{
		for(ix=0; ix<width/16; ix++)
		{
			uint8x16_t r1pixels = vld1q_u8((uint8_t*)im+(16*ix+2*iy*width)); // load 16 8-bit pixels from row 1
			uint8x16_t r2pixels = vld1q_u8((uint8_t*)im+(16*ix+2*iy*width+width)); // load 16 8-bit pixels from row 2
			
			uint8x16_t avpixels = vrhaddq_u8(r1pixels,r2pixels); // average the rows into a 8x16 (rounding up here)
			uint16x8_t averaged_16 = vpaddlq_u8(avpixels);        //average the columns into a 16x8
			uint8x8_t averaged = vshrn_n_u16(averaged_16,1);  // divide by two and narrow into 8x8 (rounding down here)

			vst1_u8((uint8_t*)(halfim+8*ix+(width/2)*iy),averaged); // store 8 8-bit values into destination image
		}
	}
}*/

//return timestamp in microseconds since first call to this function
int util_timestamp_int()
{
  static struct timeval tv1;
  struct timeval tv;
  if(tv1.tv_usec==0 && tv1.tv_sec==0) gettimeofday(&tv1, NULL); 
  gettimeofday(&tv, NULL); 
  return (int)(tv.tv_sec-tv1.tv_sec)*1000000+(int)(tv.tv_usec-tv1.tv_usec);
}

void drawDot(void* data_pointer, unsigned int x, unsigned int y, unsigned int radius)
{
	unsigned int i, j;
	x-=radius/2;
	y-=radius/2;
	if(x<0) x=0;
	if(y<0) y=0;
	unsigned int topLeft = y*(2*WIDTH) + x*2;
	for(i = 0; i < radius*2; i++)
	{
		for(j = 0; j < radius; j++)
		{			
			/*if((radius%2 == 1) && (j == radius-1 || i == radius-1))	// account for odd radius
			{
				*((unsigned char*)data_pointer + ((x+i+1)+WIDTH*(y+j))) = 255;
				*((unsigned char*)data_pointer + ((x+i)+WIDTH*(y+j+1))) = 255;
				if(j == radius-1 && i == radius-1)
					*((unsigned char*)data_pointer + ((x+i+1)+WIDTH*(y+j+1))) = 255;
			}
			
			*((unsigned char*)data_pointer + ((x+i)+WIDTH*(y+j))) = 255;
			*((unsigned char*)data_pointer + (WIDTH*HEIGHT + (x+i)/2+(WIDTH/2)*(int)((y+j)/2))) = 0;
			*((unsigned char*)data_pointer + (WIDTH*HEIGHT+(WIDTH/2)*(HEIGHT/2)+(x+i)/2+(WIDTH/2)*(int)((y+j)/2))) = 255;*/
			
			*((unsigned char*)data_pointer + (topLeft+i+(j*2*WIDTH))) = 255;
		}
	}
}

// takes about 1.1ms. 
// Known issue: outliers on the right or bottom of the image have a greater effect when the blob
// is in the top of left of the image. Ideally should use median rather than mean but this would be too slow.
void findBlob(void* data_pointer, unsigned char Ymin, unsigned char Ymax, unsigned char Umin, unsigned char Umax, unsigned char Vmin, unsigned char Vmax)
{
	// pixel at location (x,y) has:
	// Y value i=x+320*y
	// U value i=320*240 + x/2+160*(int)(y/2)
	// V value i=320*240+160*120+x/2+160*(int)(y/2)
	
	struct point centre;
	centre.x = 0;
	centre.y = 0;
	unsigned int i, x, y, count = 0;
	unsigned char Y, U, V;
	
	/*for(i = 0; i < 320*240; i++)
	{
		x = i%320;
		y = i/320;
		//Y = *((unsigned char*)data_pointer + (x+320*y));
		//U = *((unsigned char*)data_pointer + (320*240 + x/2+160*(int)(y/2)));
		V = *((unsigned char*)data_pointer + (320*240+160*120+x/2+160*(int)(y/2)));
		
		/*testing
		if(Y>=Ymin && Y<=Ymax)
			*((unsigned char*)data_pointer + (x+320*y)) = 0;
		
		if(U>=Umin && U<=Umax)
			*((unsigned char*)data_pointer + (320*240 + x/2+160*(int)(y/2))) = 0;
		
		if(V>=Vmin && V<=Vmax)
			*((unsigned char*)data_pointer + (320*240+160*120+x/2+160*(int)(y/2))) = 0;
		
		
		if(Y>=Ymin && Y<=Ymax && U>=Umin && U<=Umax && V>=Vmin && V<=Vmax)
		{
			centre.x += x;
			centre.y += y;
			count++;
		}			
	}*/
	
		/* YUV420
	for(i = 96000; i < 115200; i++)
	{
		V = *((unsigned char*)data_pointer + i);

		if(V>=Vmin && V<=Vmax)
		{
			*((unsigned char*)data_pointer + i) = 0;
			centre.x += 2*((i-96000)%160);
			centre.y += (i-96000)/80;
			count++;
		}
	}*/
	
	// UYVY
	for(i = 2; i < image_size; i+=4)
	{
		V = *((unsigned char*)data_pointer + i);

		if(V>=Vmin && V<=Vmax)
		{
			*((unsigned char*)data_pointer + i) = 0;
			centre.x += (i%(WIDTH*2))/2;
			centre.y += i/(WIDTH*2);
			count++;
		}
	}
	
	if(count > 5)
	{
		centre.x /= count;
		centre.y /= count;
	}
	else
	{
		centre.x = WIDTH/2;
		centre.y = HEIGHT/2;
	}
	
	drawDot(data_pointer, centre.x, centre.y, 10);
	//return centre;
}

void convert2grey(void* outputImage, void* inputImage)
{
	unsigned int i;
	for(i = 0; i < image_size/2-1; i++)
	{								
		*((unsigned char*)outputImage + i) = *((unsigned char*)inputImage + (2*i+1));							
	}
	
}

static int read_frame_small(void)
{
    struct v4l2_buffer buf;
    unsigned int i;

    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_DQBUF, &buf) < 0) 
	{
		printf("ioctl() VIDIOC_DQBUF failed.\n");
		return -1;    
    }

    assert(buf.index < n_buffers);
    printf ("Buffer: index = %d\n", buf.index);
	void* data_pointer = buffers[buf.index].start;
	
	/* two loops = 13.5ms
	util_timestamp_int();
	for(i = 0; i < image_size/2; i++)
	{								
		*((unsigned char*)data_pointer + i) = (	*((unsigned char*)data_pointer + (2*i)) + 
												*((unsigned char*)data_pointer + (2*i + 1)))/2;							
	}
	
	for(i = 0; i < image_size/4; i++)
	{
		unsigned int ind = i+320*(int)(i/320);												
		*((unsigned char*)data_pointer + i) = (	*((unsigned char*)data_pointer + (ind)) + 
												*((unsigned char*)data_pointer + (ind + 320)))/2;							
	}*/
	
	// One loop = 6.8ms
	//util_timestamp_int();
	/*for(i = 0; i < image_size/4; i++)
	{
		// (x,y) = AVERAGE[(2x,4y)+(2x+1,4y)+(2x,4y+1)+(2x+1,4y+1)]
		// i = x + 640y
		// x = i%640, y = i/640
		unsigned int fact = 2*(i%640) + 2560*((int)(i/640));
		*((unsigned char*)data_pointer + i) = (	*((unsigned char*)data_pointer + (fact)) + 
												*((unsigned char*)data_pointer + (fact + 1)) +
												*((unsigned char*)data_pointer + (fact + 640)) + 
												*((unsigned char*)data_pointer + (fact + 641)))/4;							
	}*/
	
	/* NEON Optimised = XX.Xms
	util_timestamp_int();
	
	struct buffer* output_image = NULL;
	output_image = calloc(1, sizeof(*output_image));
	output_image[0].length = buf.length;
	output_image[0].start = mmap(NULL, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
			
	unsigned char* output_image_pointer = (char*) output_image[0].start;
	halfsize(output_image_pointer, (unsigned char*)data_pointer, 640, 480);
	*/
	//printf("Time to downsample frame = %d microseconds\n", util_timestamp_int());
	//printf("Y = %i U = %i V = %i\n", *((unsigned char*)data_pointer + 38560), *((unsigned char*)data_pointer + 86480), *((unsigned char*)data_pointer + 105680));
	/**((unsigned char*)data_pointer + 38560) = 0;
	*((unsigned char*)data_pointer + 86480) = 0;
	*((unsigned char*)data_pointer + 105680) = 0;*/
	
	//findBlob(data_pointer, 91, 100, 130, 136, 160, 240);
	
	void* greyscale_pointer = malloc(WIDTH*HEIGHT*sizeof(char));
	convert2grey(greyscale_pointer, data_pointer);
	
	int numcorners;
	
	//util_timestamp_int();
	/*
		FAST9 b=40: 460@21.7ms
		FAST10 b=35: 474@21.6ms
		FAST11 b=30: 446@20.1ms
		FAST12 b=27: 470@20.2ms
	*/
	xy* corners = fast12_detect_nonmax((const byte*) greyscale_pointer, WIDTH, HEIGHT, WIDTH, 27, (int*)&numcorners);
	printf("Corners found = %i\n", numcorners);
	
	//printf("Time to find FAST corners = %d microseconds\n", util_timestamp_int());
	
	unsigned int j;
	for (j=0; j<numcorners; j++)
	{
		drawDot(data_pointer, corners[j].x, corners[j].y, 6);
	}	
	
    fwrite(/*(void*)output_image_pointer*/data_pointer, image_size, 1, file_fd);

    if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) 
	{
		printf("ioctl() VIDIOC_QBUF failed.\n");
		return -1;
    }

    return 1;
}


int reset_cropping_parameters()
{
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;

	memset(&cropcap, 0, sizeof (cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == ioctl(fd, VIDIOC_CROPCAP, &cropcap)) 
	{
		printf("ioctl() VIDIOC_CROPCAP failed.\n");
		return -1;    
	}

	memset(&crop, 0, sizeof (crop));
	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c = cropcap.defrect; 

	//Ignore if cropping is not supported (EINVAL). 

	if (-1 == ioctl(fd, VIDIOC_S_CROP, &crop) && errno != EINVAL) 
	{
		printf("ioctl() VIDIOC_S_CROP failed.\n");
		return -1;    
	}
	
	return 0;
}

int main(int argc,char ** argv)
{
    struct v4l2_capability cap;
    unsigned int i;
    enum v4l2_buf_type type;

    file_fd = fopen("test.yuv", "wb");
    fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) 
	{
		printf("ioctl() VIDIOC_QUERYCAP failed.\n");
		return -1;    
    }
	
    printf("driver = %s, card = %s, version = %d, capabilities = 0x%x\n",
    			cap.driver, cap.card, cap.version, cap.capabilities);		
	
	struct v4l2_format fmt;
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;	
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
	
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) 
	{
		printf("ioctl() VIDIOC_S_FMT failed.\n");
		return -1;    
    }

    image_size = fmt.fmt.pix.width * fmt.fmt.pix.height *2;
	
    struct v4l2_requestbuffers req;

    CLEAR(req);
    req.count = 5;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) 
	{
		printf("ioctl() VIDIOC_REQBUFS failed.\n");
		return -1;    
    }

    //printf("Buffer count = %d\n", req.count);

    buffers = calloc(req.count, sizeof(*buffers));
		
    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) 
	{
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;

		if (ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) 
		{
			printf("ioctl() VIDIOC_QUERYBUF failed.\n");
			return -1;    
		}

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = mmap(NULL, buf.length, 
			PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start) 
		{
			printf ("mmap() failed.\n");
			return -1;
		}
    }

    for (i = 0; i < n_buffers; ++i) 
	{
		struct v4l2_buffer buf;

		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) 
		{
			printf("ioctl() VIDIOC_QBUF failed.\n");
			return -1;    
		}
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type)< 0) 
	{
		printf("ioctl() VIDIOC_STREAMON failed.\n");
		return -1;    
    }

    for (;;) 
	{
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		tv.tv_sec = 2;
		tv.tv_usec = 0;
		r = select(fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) 
		{
			if (EINTR == errno) continue; 
			printf("select err\n");
		}

		if (0 == r) 
		{
			fprintf(stderr, "select timeout\n");
			exit(EXIT_FAILURE);
		}
		
		if (read_frame_small()) 
		{
			break;
		}
    }

    for (i = 0; i < n_buffers; ++i) 
	{
    	if (-1 == munmap(buffers[i].start, buffers[i].length)) printf("munmap() failed.\n");
    }

    close(fd);
    fclose(file_fd);
    return 0;
}
