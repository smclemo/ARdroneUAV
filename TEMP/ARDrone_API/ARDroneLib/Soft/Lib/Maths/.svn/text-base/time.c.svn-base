#include <Maths/time.h>
#include <sys/time.h>

float32_t time_in_ms_f(void)
{
  float32_t time_milli_sec, f_sec, f_usec;

  struct timeval  tv;

  gettimeofday(&tv, NULL);

  f_sec   = tv.tv_sec;
  f_usec  = tv.tv_usec;

  time_milli_sec = (float32_t)(1000.0f*f_sec + f_usec/1000.0f);

  return time_milli_sec;
}
