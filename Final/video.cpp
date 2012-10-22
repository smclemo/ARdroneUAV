/*
 *  Derived from V4L2 video capture example
 *  http://linuxtv.org/downloads/v4l-dvb-apis/index.html
 *
 *  Steven Clementson 2012
 * 
 *************************************************************************
 *  Usage example: (from external program)
#include "video.h"
...
while(video_init((char*) "/dev/video1", 640, 480, 30)){
	printf("Camera initialisation failed. Retry in 2 seconds...\n");
	sleep(2);
}
...
unsigned char* frame = (unsigned char*)malloc(WIDTH*HEIGHT*2*sizeof(char));
get_frame(frame);
...
video_close();
 *************************************************************************
 *  TODO: 
 *  LN: 43		Set io = IO_METHOD_MMAP or IO_METHOD_READ or IO_METHOD_USERPTR
 *	LN:	495		Set fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY
 * That's it! 
 */

#include "video.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define PAGE_SIZE	4096

pthread_t video_thread;
pthread_mutex_t video_access_mutex = PTHREAD_MUTEX_INITIALIZER;
bool save = false;	// save video to file for debugging if set
FILE *file_fd;

static unsigned int WIDTH, HEIGHT, FPS;
static unsigned long image_size;
static char *dev_name;

static enum io_method io = IO_METHOD_MMAP;
static int fd = -1;
struct buffer *buffers;
static unsigned int n_buffers;
void* current_frame;
int frame_ready = 0;

static void errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n",
		 s, errno, strerror(errno));

	exit(EXIT_FAILURE);
}

static int xioctl(int fd, int request, void *arg)
{
	int r;
	do
		r = ioctl(fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}

static void process_image(const void *p)
{
	pthread_mutex_lock(&video_access_mutex);
	
	if(save)
	{
		fwrite(p, image_size, 1, file_fd);
		printf("frame saved\n");
	}
	current_frame = (void*) p;
	frame_ready = 1;
	
	pthread_mutex_unlock(&video_access_mutex);
}

static int read_frame(void)
{
	struct v4l2_buffer buf;
	unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
		if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */

			default:
				errno_exit("read");
			}
		}

		process_image(buffers[0].start);

		break;

	case IO_METHOD_MMAP:
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;

		if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */

			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}
		assert(buf.index < n_buffers);

		process_image(buffers[buf.index].start);

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");

		break;

	case IO_METHOD_USERPTR:
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */
				/* fall through */

			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}

		for (i = 0; i < n_buffers; ++i)
			if (buf.m.userptr == (unsigned long)buffers[i].start
			    && buf.length == buffers[i].length)
				break;

		assert(i < n_buffers);

		process_image((void *) buf.m.userptr);

		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");

		break;
	}

	return 1;
}

void get_frame(unsigned char* image_out)
{
	pthread_mutex_lock(&video_access_mutex);
	
	memcpy(image_out, current_frame, WIDTH*HEIGHT*2);
	frame_ready = 0;
	
	pthread_mutex_unlock(&video_access_mutex);
}

void get_frame_grey(unsigned char* image_out)
{
	unsigned int numberPixels = WIDTH*HEIGHT;
	
	pthread_mutex_lock(&video_access_mutex);
	
	char* char_pointer = (char*) current_frame;
	unsigned int y1, y2;
	while(numberPixels > 0) 
	{
	      char_pointer++;
	      y1=*(char_pointer++);
	      char_pointer++;
	      y2=*(char_pointer++);
	      *(image_out++)=y1;
	      *(image_out++)=y2;
	      numberPixels-=2;
	}
	
	frame_ready = 0;
	
	pthread_mutex_unlock(&video_access_mutex);
}

void uyvyToGrey(unsigned char *dst, unsigned char *src,  unsigned int numberPixels)
{
	while(numberPixels > 0) 
	{
	      src++;
	      unsigned int y1=*(src++);
	      src++;
	      unsigned int y2=*(src++);
	      *(dst++)=y1;
	      *(dst++)=y2;
	      numberPixels-=2;
	}
}

void drawDot(unsigned char* data_pointer, unsigned int x, unsigned int y, unsigned int radius)
{
	unsigned int i, j;
	x-=radius/2;
	y-=radius/2;
	unsigned int topLeft = y*(2*WIDTH) + x*2;
	
	for(i = 0; i < radius*2; i++)
		for(j = 0; j < radius; j++)
			*(data_pointer + (topLeft+i+(j*2*WIDTH))) = 255;
	
}

void drawBox(unsigned char* data_pointer, unsigned int x, unsigned int y, unsigned int size)
{
	unsigned int i;
	x-=size/2;
	y-=size/2;
	unsigned int topLeft = y*WIDTH + x;
	unsigned int topRight = y*WIDTH + x + size;
	unsigned int bottomLeft = (y+size)*WIDTH + x;
	
	for(i = 0; i < size; i++)
	{
		*(data_pointer + topLeft + i) = 255;
		*(data_pointer + bottomLeft + i) = 255;
		*(data_pointer + topLeft + i*WIDTH) = 255;
		*(data_pointer + topRight + i*WIDTH) = 255;
	}
}

void* video_thread_main(void* data)
{
	fd_set fds;
	struct timeval tv;
	int r;
		
	while(1)
	{
		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		r = select(fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) 
		{
			if (EINTR == errno)
				continue;
			errno_exit("select");
		}

		if (0 == r) 
		{
			fprintf(stderr, "select timeout\n");
			exit(EXIT_FAILURE);
		}

		if (read_frame())
			usleep(FPS*1000);

		/* EAGAIN - continue select loop. */
	}
}

static void stop_capturing(void)
{
	enum v4l2_buf_type type;

	switch (io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
			errno_exit("VIDIOC_STREAMOFF");

		break;
	}
}

static void start_capturing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	switch (io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);

			buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory      = V4L2_MEMORY_MMAP;
			buf.index       = i;

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");

		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i) {
			struct v4l2_buffer buf;

			CLEAR(buf);

			buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory	= V4L2_MEMORY_USERPTR;
			buf.m.userptr	= (unsigned long)buffers[i].start;
			buf.length	= buffers[i].length;
			buf.index	= i;

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");

		break;
	}
}

static void uninit_device(void)
{
	unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
		free(buffers[0].start);
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i)
			if (-1 == munmap(buffers[i].start, buffers[i].length))
				errno_exit("munmap");
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i)
			free(buffers[n_buffers].start);
		break;
	}

	free(buffers);
}

static void init_read(unsigned int buffer_size)
{
	buffers = (buffer*) calloc(1, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	buffers[0].length = buffer_size;
	buffers[0].start = malloc(buffer_size);

	if (!buffers[0].start) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
}

static void init_mmap(void)
{
	struct v4l2_requestbuffers req;
	CLEAR(req);

	req.count	    = 4;
	req.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory	    = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support "
				 "memory mapping\n", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf(stderr, "Insufficient buffer memory on %s\n",
			 dev_name);
		exit(EXIT_FAILURE);
	}

	buffers = (buffer*) calloc(req.count, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;

		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
			errno_exit("VIDIOC_QUERYBUF");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start =
			mmap(NULL /* start anywhere */,
			      buf.length,
			      PROT_READ | PROT_WRITE /* required */,
			      MAP_SHARED /* recommended */,
			      fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start)
			errno_exit("mmap");
	}
}

static void init_userp(unsigned int buffer_size)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s does not support "
				 "user pointer i/o\n", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	buffers = (buffer*) calloc(4, sizeof(*buffers));

	if (!buffers) {
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
		buffers[n_buffers].length = buffer_size;
		posix_memalign(&buffers[n_buffers].start, PAGE_SIZE,
			       buffers[n_buffers].length);

		if (!buffers[n_buffers].start) {
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
}

static void init_device(void)
{
	struct v4l2_capability cap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;
	
	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n",
				 dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n",
			 dev_name);
		exit(EXIT_FAILURE);
	}

	switch (io) {
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			fprintf(stderr, "%s does not support read i/o\n",
				 dev_name);
			exit(EXIT_FAILURE);
		}
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			fprintf(stderr, "%s does not support streaming i/o\n",
				 dev_name);
			exit(EXIT_FAILURE);
		}
		break;
	}
	
	CLEAR(fmt);
	fmt.type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = WIDTH;
	fmt.fmt.pix.height      = HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
		errno_exit("VIDIOC_S_FMT");

	/* Note VIDIOC_S_FMT may change width and height. */

	image_size = fmt.fmt.pix.width * fmt.fmt.pix.height * 2;
	
	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	switch (io) {
	case IO_METHOD_READ:
		init_read(fmt.fmt.pix.sizeimage);
		break;

	case IO_METHOD_MMAP:
		init_mmap();
		break;

	case IO_METHOD_USERPTR:
		printf("capture: fmt.fmt.pix.sizeimage = %d\n",
							fmt.fmt.pix.sizeimage);
		init_userp(fmt.fmt.pix.sizeimage);
		break;
	}
}

static void close_device(void)
{
	if (-1 == close(fd))
		errno_exit("close");

	fd = -1;
}

static void open_device(void)
{
	struct stat st;

	if (-1 == stat(dev_name, &st)) {
		fprintf(stderr, "Cannot identify '%s': %d, %s\n",
			 dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode)) {
		fprintf(stderr, "%s is no device\n", dev_name);
		exit(EXIT_FAILURE);
	}

	fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);

	if (-1 == fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n",
			 dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

int video_init(char* videoName, int cam_width, int cam_height, int cam_fps) 
{
	dev_name = videoName;
	WIDTH = cam_width;
	HEIGHT = cam_height;
	FPS = cam_fps;
	
	open_device();

	init_device();

	start_capturing();

	//start video thread 
	int rc = pthread_create(&video_thread, NULL, video_thread_main, NULL);
	if (rc) {
		printf("ctl_Init: Return code from pthread_create(video_thread) is %d\n", rc);
		return 1;
	}

	return 0;
}

int video_frame_ready()
{
	return frame_ready;
}

void video_start_recording()
{
	file_fd = fopen("video_output.yuv", "wb");
	save = true;
}

void video_close()
{
	pthread_cancel(video_thread);
	stop_capturing();
	uninit_device();
	close_device();
	
	if(save)
		fclose(file_fd);
}