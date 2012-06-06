#include <opencv/cv.h>
#include <libfreenect.h>


int filter_distance(IplImage * img, int distance);


class freenect{
private:
   // size of frame
   short max_width;
   short max_height;

   short error;
   // freenect value
   // thread
   pthread_t freenect_thread;
   IplImage * frame_rgb;
   IplImage * frame_dph;

   // alloc memory
   static void * freenect_threadfunc(void * arg);
   static void rgb_cb(freenect_device *dev, void *rgb, uint32_t timestamp);
   static void depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp);
public:
   static int filter_from, filter_to;

   void init(short user_device_number);
   int reload();
   
   int get_error();
   void set_filter(int from, int to);
   IplImage * get_image_rgb();
   IplImage * get_image_depth_rgb();
   freenect();
   ~freenect();
};
