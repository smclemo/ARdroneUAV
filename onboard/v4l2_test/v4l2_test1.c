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

#define CLEAR(x) memset (&(x), 0, sizeof (x))

struct buffer {
    void * start;
    size_t length;
};

static char * dev_name = "/dev/video0";		//AR.Drone front camera
//static char * dev_name = "/dev/video1";	//AR.Drone bottom camera
static int fd = -1;
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
	
	/* two loops = 39.3ms
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
	
	// One loop = 26.7ms
	util_timestamp_int();
	for(i = 0; i < image_size/4; i++)
	{
		// (x,y) = AVERAGE[(2x,4y)+(2x+1,4y)+(2x,4y+1)+(2x+1,4y+1)]
		// i = x + 640y
		// x = i%640, y = i/640
		unsigned int fact = 2*(i%640) + 2560*((int)(i/640));
		*((unsigned char*)data_pointer + i) = (	*((unsigned char*)data_pointer + (fact)) + 
												*((unsigned char*)data_pointer + (fact + 1)) +
												*((unsigned char*)data_pointer + (fact + 640)) + 
												*((unsigned char*)data_pointer + (fact + 641)))/4;							
	}
	printf ("Time to downsample frame = %d microseconds\n", util_timestamp_int());
	
    fwrite(data_pointer, image_size/4, 1, file_fd);

    if (ioctl(fd, VIDIOC_QBUF, &buf) < 0) 
	{
		printf("ioctl() VIDIOC_QBUF failed.\n");
		return -1;
    }

    return 1;
}

//return timestamp in microseconds since first call to this function
int util_timestamp_int()
{
  static struct timeval tv1;
  struct timeval tv;
  if(tv1.tv_usec==0 && tv1.tv_sec==0) gettimeofday(&tv1, NULL); 
  gettimeofday(&tv, NULL); 
  return (int)(tv.tv_sec-tv1.tv_sec)*1000000+(int)(tv.tv_usec-tv1.tv_usec);
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

	/* Ignore if cropping is not supported (EINVAL). */

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
    fmt.fmt.pix.width = 640;	//for AR.Drone front camera
    fmt.fmt.pix.height = 480;
    //fmt.fmt.pix.width = 176;	//for AR.Drone bottom camera
    //fmt.fmt.pix.height = 144;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) < 0) 
	{
		printf("ioctl() VIDIOC_S_FMT failed.\n");
		return -1;    
    }

    image_size = fmt.fmt.pix.width * fmt.fmt.pix.height *3/2;
	
    struct v4l2_requestbuffers req;

    CLEAR(req);
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) < 0) 
	{
		printf("ioctl() VIDIOC_REQBUFS failed.\n");
		return -1;    
    }

    printf("Buffer count = %d\n", req.count);

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
