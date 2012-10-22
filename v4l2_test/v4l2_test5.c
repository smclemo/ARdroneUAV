/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 */

#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>	     /* getopt_long() */

#include <fcntl.h>	      /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/types.h>	  /* for videodev2.h */

#include <linux/videodev2.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define PAGE_SIZE	4096

FILE *file_fd;
static unsigned long image_size;

enum io_method {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
};

struct buffer {
	void *start;
	size_t length;
};

static char *dev_name;
static enum io_method io = IO_METHOD_MMAP;
static int fd = -1;
struct buffer *buffers;
static unsigned int n_buffers;

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
	fwrite(p, image_size, 1, file_fd);
	printf("frame saved\n");
	sleep(1);
	//fputc('.', stdout);
	//fflush(stdout);
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

static void mainloop(void)
{
	unsigned int count;

	count = 30;

	while (count-- > 0) {
		for (;;) {

			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			/* Timeout. */
			tv.tv_sec = 2;
			tv.tv_usec = 0;

			r = select(fd + 1, &fds, NULL, NULL, &tv);

			if (-1 == r) {
				if (EINTR == errno)
					continue;

				errno_exit("select");
			}

			if (0 == r) {
				fprintf(stderr, "select timeout\n");
				exit(EXIT_FAILURE);
			}

			if (read_frame())
				break;

			/* EAGAIN - continue select loop. */
		}
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
	buffers = calloc(1, sizeof(*buffers));

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

	req.count	       = 4;
	req.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory	      = V4L2_MEMORY_MMAP;

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

	buffers = calloc(req.count, sizeof(*buffers));

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

	buffers = calloc(4, sizeof(*buffers));

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

static void init_device(void)
{
	struct v4l2_capability cap;
	//struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	/**************
	
	struct v4l2_cropcap cropcap;
	struct v4l2_format format;

	reset_cropping_parameters ();

	// Scale down to 1/4 size of full picture. 

	memset (&format, 0, sizeof (format)); // defaults 

	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	format.fmt.pix.width = cropcap.defrect.width >> 1;
	format.fmt.pix.height = cropcap.defrect.height >> 1;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;

	if (-1 == ioctl (fd, VIDIOC_S_FMT, &format)) {
			perror ("VIDIOC_S_FORMAT");
			exit (EXIT_FAILURE);
	}

	****************/
	
	
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


	/* Select video input, video standard and tune here. */

/*
	CLEAR(cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; // reset to default 

		if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
			switch (errno) {
			case EINVAL:
				// Cropping not supported. 
				break;
			default:
				printf("Error in VIDIOC_S_CROP\n");
				break;
			}
		}
	} else
		printf("Error in VIDIOC_CROPCAP\n");
	*/
	
	CLEAR(fmt);

	fmt.type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = 1280;
	fmt.fmt.pix.height      = 720;
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

	fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

	if (-1 == fd) {
		fprintf(stderr, "Cannot open '%s': %d, %s\n",
			 dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp,
		 "Usage: %s [options]\n\n"
		 "Options:\n"
		 "-d | --device name   Video device name [/dev/video]\n"
		 "-h | --help	  Print this message\n"
		 "-m | --mmap	  Use memory mapped buffers\n"
		 "-r | --read	  Use read() calls\n"
		 "-u | --userp	 Use application allocated buffers\n"
		 "",
		 argv[0]);
}

static const char short_options[] = "d:hmru";

static const struct option
long_options [] = {
	{ "device",     required_argument,      NULL,	   'd' },
	{ "help",       no_argument,	    NULL,	   'h' },
	{ "mmap",       no_argument,	    NULL,	   'm' },
	{ "read",       no_argument,	    NULL,	   'r' },
	{ "userp",      no_argument,	    NULL,	   'u' },
	{ 0, 0, 0, 0 }
};


/*
 *  ATComSerialProxyServer
 *  v0.2.2
 *  ARDroneTools
 *
 *  Created by nosaari on 25.02.11.
 *

 Very basic proxy server that reads any lines in given tty device and sends it
 via UDP to given server.
 Any line thats not starting with 'AT' is considered as debug output and will
 be ignored!

 Based on server code from
 http://www.gamedev.net/topic/310343-udp-client-server-echo-example/

 For detailed infos read
 http://www.linuxhowtos.org/C_C++/socket.htm

 *
 */



// Address of control server on drone, should be 192.168.1.1
// Test the server locally by setting address to "127.0.0.1" and
// typing "nc -l -u 127.0.0.1 5556" in a terminal window!
#define REMOTE_SERVER_ADDRESS   "192.168.1.1"
// Port of control server for AT commands, leave at 5556
#define REMOTE_SERVER_PORT      5556

int connectionSocket = 0;
struct sockaddr_in remoteAddress;
uint addressLength = 0;	
	
int init_command()
{
    // Create socket with UDP setting (SOCK_DGRAM)
    connectionSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (connectionSocket < 0)
    {
        fprintf(stderr, "ERROR: Cannot open socket!\n");
        return 1;
    }

    // Initialise remote server address
    memset(&remoteAddress, 0, sizeof(struct sockaddr_in));
    remoteAddress.sin_family = AF_INET;
    remoteAddress.sin_addr.s_addr = inet_addr(REMOTE_SERVER_ADDRESS);
    remoteAddress.sin_port = htons(REMOTE_SERVER_PORT);
    addressLength = sizeof(remoteAddress);

	return 0;
}

int send_command(char* message)
{
	int flags = 0;   // Send flags, 0 should be good

	// CONVENTION: Only send messages that begin with the chars 'AT'!
	// Every AT command must begin with these, everything else is ignored!
	if (message[0] == 'A' && message[1] == 'T')
	{
		// Add line feed to the end
		strncat(message, "\r", 2);

		// Send message to remote server
		int returnNo = sendto(connectionSocket,
						  message,
						  strlen(message),
						  flags,
						  (struct sockaddr *)&remoteAddress,
						  addressLength);

		if (returnNo < 0)
		{
			fprintf(stderr, "ERROR: Cannot send data!\n");
			return 1;
		}
		return 0;
	}

	return 1;
}



int main(int argc, char **argv)
{
	dev_name = "/dev/video1";
	file_fd = fopen("test.yuv", "wb");

	for (;;) {
		int index;
		int c;

		c = getopt_long(argc, argv,
				 short_options, long_options,
				 &index);

		if (-1 == c)
			break;

		switch (c) {
		case 0: /* getopt_long() flag */
			break;

		case 'd':
			dev_name = optarg;
			break;

		case 'h':
			usage(stdout, argc, argv);
			exit(EXIT_SUCCESS);

		case 'm':
			io = IO_METHOD_MMAP;
			break;

		case 'r':
			io = IO_METHOD_READ;
			break;

		case 'u':
			io = IO_METHOD_USERPTR;
			break;

		default:
			usage(stderr, argc, argv);
			exit(EXIT_FAILURE);
		}
	}

	open_device();

	init_device();

	start_capturing();

	int rc = system("(../../bin/program.elf ${PELF_ARGS}; gpio 181 -d ho 1) & > /dev/null 2>&1");
	sleep(100);
	printf("return code from program.elf = %i\n", rc);

	init_command();
	sleep(2);
	
	char data[100];
	memset(data, 0x0, 100);
	sprintf(data, "AT*FTRIM=%d\r", 1);
	send_command(data);
	sleep(2);
	
	memset(data, 0x0, 100);
	sprintf(data, "AT*LED=%d,2,1073741824,1\r", 2);
	send_command(data);
	sleep(2);
	
	float roll_move = 0;
	float pitch_move = 0;
	float throttle_move = 0;
	float yaw_move = 0;
	
	memset(data, 0x0, 100);
	sprintf(data, "AT*PCMD=%d,0,%ld,%ld,%ld,%ld\r",
              3,
              roll_move,
              pitch_move,
              throttle_move,
              yaw_move);
	send_command(data);
	sleep(2);
	
	memset(data, 0x0, 100);
	sprintf(data, "AT*REF=%d,290718208\r", 4);
	send_command(data);
	sleep(2);
	
	memset(data, 0x0, 100);
	sprintf(data, "AT*PCMD=%d,1,%ld,%ld,%ld,%ld\r",
              5,
              roll_move,
              pitch_move,
              throttle_move,
              yaw_move);
	send_command(data);
	sleep(2);
	
	memset(data, 0x0, 100);
	sprintf(data, "AT*PCMD=%d,0,0,0,0,0\r", 1);
	send_command(data);
	sleep(2);
	
	memset(data, 0x0, 100);
	sprintf(data, "AT*REF=%d,290717696\r", 1);
	send_command(data);
	sleep(2);
	
	mainloop();

	stop_capturing();

	uninit_device();

	close_device();

	fclose(file_fd);
	
	exit(EXIT_SUCCESS);

	return 0;
}
