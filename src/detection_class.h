/*
 * detection_class.h
 *
 *  Created on: 27.11.2010
 *      Author: Pavel Studeník
 */

#ifndef DETECTION_CLASS_H_
#define DETECTION_CLASS_H_

#define MIN_SIZE 100
#define MAX_SIZE 3100
#define NUM_SIZE 20

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <cvblob.h>

class Detection{
	CvCapture* capture;
	char path[256];
	CvMemStorage* storage;
	int	g_thresh;
	int width, height;
	int fps;
	IplImage* paint,  *tmp, * frame, *g_gray;
	CvPoint mouse;
	int mouse_type;
	CvSeq* contours;

public:
	Detection(CvCapture* capture);
	IplImage* Next(void);
	IplImage * Rotate(IplImage * frame,int angle, bool mirror);
	void SetMouseRang(float *x, float *y , int *bold);
	IplImage* DebugImage(void);
	void SetTreshold(int treshold);
};

#endif /* DETECTION_CLASS_H_ */
