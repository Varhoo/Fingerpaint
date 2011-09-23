/*
 * detection_class.cpp
 *
 *  Created on: 27.11.2010
 *      author: Pavel Studeník
 *      email: studenik@varhoo.cz
 */

#include "detection_webcam.h"

Detection::Detection(CvCapture * capture_tmp){
        //bakcup caputure of video
		capture = capture_tmp;

		if(!cvGrabFrame(capture)){              
          // capture a frame
		  printf("FAIL: Could not grab a frame\n");
		}

        //get information about video
	    height    = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
	    width    = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
	    fps       = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);

        //get frame from video capture
	    frame = cvQueryFrame(capture);

        //sometime cature don't set infomration
        //get information from frame
	    if(!(width>0 && height>0)){
	    	width = frame->width;
	    	height = frame->height;
	    	printf("SET: size camera: %d %d\n", width, height);
	    }
        //set fps - manualy
	    if(!(fps>0)){
	    	fps = DEFAULT_FPS;
	    	printf("SET: psf camera: %d\n", fps);
	    }

		storage= cvCreateMemStorage();

        //temporary image
		g_gray = cvCreateImage( cvGetSize( frame ), 8, 1 );
		paint = cvCreateImage( cvGetSize( frame ), 8, 3 );
		tmp = cvCreateImage( cvGetSize( frame ), 8, 1 );

		contours = 0;
		g_thresh = 150; //default trashold

	}

/**
 * Set value for trashold
 */
void Detection::SetTreshold(int treshold){
	g_thresh = treshold;
}

/**
 * Move to next frame
 * @return - actual frame
 */
IplImage*	Detection::Next(void){

		frame = cvQueryFrame( capture );

		cvb::CvTracks tracks;

        // rotate frame thanks mirror and degree transformation
		//Rotate(frame,180, true);

		cvCvtColor( frame, g_gray, CV_BGR2GRAY );

        timer = Microtime();
		cvThreshold( g_gray, tmp, g_thresh, 255, CV_THRESH_BINARY );
        timer = Microtime() - timer;

        //filter for better detection
        //using dilate and erode
		cvDilate(tmp,g_gray,NULL,filter_de);
		cvErode(g_gray,tmp,NULL,filter_de);
		
		cvCopy(tmp,g_gray);

        //detection
        //using blob - external library
		cvb::CvBlobs blobs;
		int bold = 0;

		IplImage *labelImg = cvCreateImage(cvGetSize(g_gray), IPL_DEPTH_LABEL, 1);

		unsigned int result = cvLabel(g_gray, labelImg, blobs);

		cvCvtColor( g_gray, paint, CV_GRAY2BGR );

        //basic value 100, 100000
		cvb::cvFilterByArea(blobs, filter_min_af, filter_max_af);
		cvb::cvUpdateTracks(blobs, tracks, 200., 5);
		cvb::cvRenderTracks(tracks, paint, paint, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);


#ifdef DEBUG
		printf("size: %d\n",tracks.size());
#endif
		if(tracks.size()>0){
			cvb::CvTracks::const_iterator it=tracks.begin();
			cvb::CvTracks::const_iterator it2;

			bold = 0;
			//for(unsigned int i=0;i<tracks.size();i++){
			for ( it2=tracks.begin() ; it2 != tracks.end(); it2++ ) {
				//printf("%d\n",i);
				int _bold= (it2->second->maxx-it2->second->minx)*(it2->second->maxy-it2->second->miny);
				if (_bold > bold) {
					bold = _bold;
					it = it2;
				}
			}
#ifdef DEBUG
			printf("%d: (%f %f) %d\n",
					it->second->id,
					it->second->centroid.x,
					it->second->centroid.y,
					(it->second->maxx-it->second->minx)*(it->second->maxy-it->second->miny));
#endif
			bold= (it->second->maxx-it->second->minx)*(it->second->maxy-it->second->miny);

			mouse.x = it->second->centroid.x;
			mouse.y = it->second->centroid.y;

			if(bold < MIN_SIZE)
				mouse_type =1;
			else if(bold > MAX_SIZE)
				mouse_type =NUM_SIZE;
			else
				mouse_type = (bold-MIN_SIZE)/(MAX_SIZE/NUM_SIZE);

	#ifdef DEBUG
			printf("mouse: bold %d %d \n", mouse_type, bold);
	#endif

		}

		cvReleaseImage(&labelImg);
		cvReleaseBlobs(blobs);
		return frame;
	}

IplImage * Detection::Rotate(IplImage * frame, int angle, bool mirror){
    //angle = angle % 360;

    IplImage * t = cvCloneImage(frame);

    if(mirror){
    	cvConvertImage(t,frame,CV_CVTIMG_FLIP); //vertical
    	cvCopy(frame,t);
    }

    if(angle>0){
    	double factor = 1.0;

		float m[6];
		CvMat M = cvMat(2, 3, CV_32F, m);
		m[0] = (float)(factor*cos(-angle*CV_PI/180.));
		m[1] = (float)(factor*sin(-angle*CV_PI/180.));
		m[3] = -m[1];
		m[4] = m[0];
		m[2] = width*0.5;
		m[5] = height*0.5;

		cvGetQuadrangleSubPix( t, frame, &M);
    }

    cvReleaseImage(&t);

    return frame;
}

void 
Detection::SetParameter(
    int max_arrea_filter, int min_arrea_filter, int dilate_erode){

    this->filter_max_af = max_arrea_filter;
    this->filter_min_af = min_arrea_filter;
    this->filter_de = dilate_erode;
}

IplImage* 
Detection::DebugImage(
    void){

	return paint;
}

double
Detection::GetTimer(
    void){

    return timer;
}

/**
 * Get frame and convert it
 */
IplImage* 
Detection::GetFrame(
    void){

    //convert data RGB -> BRG due to gtk canvas 
    cvCvtColor( frame, frame, CV_RGB2BGR );
    return frame;
}

/**
 * Set data for mouse/brush
 */
void Detection::SetMouseRang(float *x, float *y, int *bold ){
	*x = mouse.x/float(width);
	*y = mouse.y/float(height);
	*bold = mouse_type;
}
