/*
 * detection_class.h
 *
 *  Created on: 27.11.2010
 *      author: Pavel Studeník
 *      email: studenik@varhoo.cz
 */

#ifndef DETECTION_CLASS_H_
#define DETECTION_CLASS_H_

#define MIN_SIZE 100
#define MAX_SIZE 3100
#define NUM_SIZE 20
#define DEFAULT_FPS 20

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include "lib/cvblob.h"
#include <highgui.h>

// library for timer
#include  <sys/time.h>
#include  <time.h>
#include <stdlib.h>

/**
 * Function return Microtime - using for debug
 */
static double 
Microtime()
{
     timeval tim;
     gettimeofday(&tim, NULL);
     return (tim.tv_sec+(tim.tv_usec/1000000.0));
};

class Detection{
private:
    //filter value for max area 
    int filter_max_af;
    //filter value for min area 
    int filter_min_af;
    //filter value dilate/erode
    int filter_de;

    //video capture from opencv
    CvCapture* capture;
    //char path[256];
    CvMemStorage* storage;
    //value fot treshold
    int	g_thresh;
    //size
    int width, height;
    //frame per second
    int fps;
    IplImage* paint,  *tmp, * frame, *g_gray;
    //position of mouse (brush)
    CvPoint mouse;
    //weight of brush
    int mouse_type;
    CvSeq* contours;
    //timer for debug
    double timer;

public:
    Detection(CvCapture* capture);
    //move to next frame
    IplImage* Next(void);
    //rotate frame
    IplImage * Rotate(IplImage * frame,int angle, bool mirror);
    //set parameter detection
    void SetParameter(int max_arrea_filter,int min_area_filter, int dilate_erode);
    //set mouse/brush to rate 0..1
    void SetMouseRang(float *x, float *y , int *bold);
    //get buffer image - during detection..
    IplImage* DebugImage(void);
    //set value for trashold
    void SetTreshold(int treshold);
    //get complited frame
    IplImage* GetFrame(void);
    double GetTimer();
};

#endif /* DETECTION_CLASS_H_ */
