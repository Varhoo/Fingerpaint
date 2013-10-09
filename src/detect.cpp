/*
 *  Created on: 27.11.2010
 *      author: Pavel Studeník
 *      email: studenik@varhoo.cz
 */

#include "detect.h"

Detect::Detect(int w, int h){
   width = w;
   height = h;

   g_gray = cvCreateImage( cvSize(w, h), 8, 1 );
   g_tmp = cvCreateImage( cvGetSize(g_gray), 8, 1 );
   g_mask = cvCreateImage( cvGetSize(g_gray), 8, 1 );
   label_img = cvCreateImage( cvGetSize(g_gray), IPL_DEPTH_LABEL, 1);
   rgb_img = cvCreateImage( cvGetSize(g_gray), IPL_DEPTH_LABEL, 3);
   tmp_paint = cvCreateImage( cvGetSize(g_gray), IPL_DEPTH_LABEL, 3);

   // default value
   value_thresh = 120;
   value_erode = 2;
   value_dilate = 2;
   value_filter_from = 500;
   value_filter_to = 10000;

   is_mask_set = false;
   //events = d_events();
}

Detect::~Detect(){
   printf("end ... ok\n");
   // free all iplImage
   /* cvReleaseImage(&g_gray);
   cvReleaseImage(&g_tmp);
   cvReleaseImage(&label_img);
   cvReleaseImage(&rgb_img);*/
}

void Detect::NextFrame(IplImage * depth, IplImage * rgb){
   
   cvCvtColor( depth, g_gray, CV_BGR2GRAY );
   cvThreshold( g_gray, g_tmp, value_thresh, 255, CV_THRESH_BINARY );

   cvErode(g_tmp, g_gray, NULL, value_erode);
   cvDilate(g_gray, g_tmp, NULL, value_dilate);

   if(is_mask_set){
      cvZero(g_gray);
      cvCopy(g_tmp, g_gray, g_mask);
   } else {
      cvCopy(g_tmp,g_gray);
   }
   cvCopy(rgb,rgb);

   //cvCvtColor( g_mask, rgb, CV_GRAY2RGB );

   cvb::CvBlobs blobs;
   int bold = 0;

   unsigned int result = cvLabel(g_gray, label_img, blobs);

   // basic value for filter by area
   // value filter from 100
   // value f1lter to 100000
   cvb::cvFilterByArea(blobs, value_filter_from, value_filter_to);
   // distance
   // time of active 
   cvb::cvUpdateTracks(blobs, tracks, 20, 5);

   // only for debug
   cvb::cvRenderTracks(tracks, rgb, rgb, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);
   int w;

   double mictime = microtime();

   if(tracks.size()>0)
   {
	   cvb::CvTracks::const_iterator it=tracks.begin();
      // temporary value of time
      double mictime = microtime();
		bold = 0;
		//for(unsigned int i=0;i<tracks.size();i++){
		for ( it=tracks.begin() ; it != tracks.end(); it++ ) {
			//printf("%d\n",i);
			w = ( (it->second->maxx - it->second->minx) + (it->second->maxy-it->second->miny) )/4 ;

         // center point and radius
//#ifdef  DEBUG
         cvCircle(rgb,	cvPoint((int) it->second->centroid.x, (int) it->second->centroid.y), w,      
              cvScalar(0, 255, 0, 0),    /* the color; red */
              2, 8, 0);     
         cvCircle(depth,	cvPoint((int) it->second->centroid.x + 4, (int) it->second->centroid.y + 4), w,      
              cvScalar(0, 255, 0, 255),    /* the color; red */
              2, 8, 0);     
//#endif

         events[it->first].x = (it->second->centroid.x )/(float) (mask[1].x-mask[0].x);
         events[it->first].y = (it->second->centroid.y )/(float) (mask[2].y-mask[0].y);
         events[it->first].size = w;

         //printf("%f %f \n", events[it->first].x, events[it->first].y);
         // is alive 

         events[it->first].time = mictime;
         events[it->first].live = true;

         //printf("time: %f %f %f \n",mictime - events[it->first].time, mictime, events[it->first].time);
         //printf("active %d \n", it->second->inactive);

         // save history of positions
         for(short i=LAST_POSITION-1; i>=0; i--){
            if(i==0){
               events[it->first].last[i].x = events[it->first].x;
               events[it->first].last[i].y = events[it->first].y;
            } else {
               events[it->first].last[i].x = events[it->first].last[i-1].x;
               events[it->first].last[i].y = events[it->first].last[i-1].y;
            }
         }

         // only for debug
         #ifdef DEBUG
			printf("%d: (%f %f) %d\n",
					it->second->id,
					it->second->centroid.x,
					it->second->centroid.y,
					(it->second->maxx-it->second->minx)*(it->second->maxy-it->second->miny));
         #endif
      }
   }

   d_events::iterator it_events;

   for(it_events = events.begin(); it_events!= events.end(); it_events++){ 
       if((mictime - it_events->second.time) > 1.) events.erase(it_events);
       //counter_people++;
   //    printf("time: %f \n",mictime - it_events->second.time);
   }

   if(is_mask_set){
      int arr[1];
      arr[0] = 4;
      CvPoint * rmask[1];
      rmask[0] = mask;

      cvPolyLine(rgb, rmask, arr, 1, true, cvScalar(255,255,255,0), 1);
   }
}


void Detect::SetMask(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4){
   mask[0].x = x1;
   mask[1].x = x2;
   mask[2].x = x3;
   mask[3].x = x4;

   mask[0].y = y1;
   mask[1].y = y2;
   mask[2].y = y3;
   mask[3].y = y4;

   is_mask_set = true;

   int arr[1];
   arr[0] = 4;
   CvPoint * rmask[1];
   rmask[0] = mask;

   cvZero(g_mask); 
   cvFillPoly(g_mask, rmask, arr, 1, cvScalar(255), 1);
}

// set value for threshold
void Detect::SetThreshold(short thresh){
   value_thresh = thresh;
}

d_events * Detect::GetEvents(){
   return &events;
}


