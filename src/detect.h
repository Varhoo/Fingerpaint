

#ifndef __DETECT__
#define __DETECT__

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <math.h>
#include "lib/cvblob.h"
#include "utils.h"
#include <map>
#define LAST_POSITION 3

typedef struct s_pnt{
   float x;
   float y;
}pnt;

typedef struct s_pos{
   float x;
   float y;
   double time;
   pnt last[LAST_POSITION];
   short size;
   bool live;
} pos; 

typedef std::map<int, pos> d_events;

class Detect {
   IplImage * label_img, * rgb_img;
   IplImage *	g_gray, * g_tmp, * tmp_paint, * g_mask;

   // default value for trashold
   short value_thresh;
   bool is_mask_set;

   unsigned int width, height;
   short value_dilate, value_erode; 

   unsigned int value_filter_from;
   unsigned int value_filter_to;
   CvPoint mask[4];
   cvb::CvTracks tracks;


   public:
      d_events events;

      Detect(int w, int h);
      ~Detect();

      void NextFrame(IplImage * depth, IplImage * rgb);

      void SetThreshold(short tresh);
      void SetMask(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
      d_events * GetEvents();
};

#endif
