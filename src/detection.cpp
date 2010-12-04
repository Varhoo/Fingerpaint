/*
 * detection.cpp
 *
 *  Created on: 27.11.2010
 *      Author: Pavel Studeník
 */

#include "detection_class.h"
using namespace cvb;

int
main(	int argc,
		char *argv[])
{

	//CvCapture * capture = cvCaptureFromAVI("data/video2.avi");
	CvCapture * capture = cvCaptureFromCAM(1);

	Detection * detec = new Detection(capture);

	while(1) {
			detec->Next();
			cvShowImage( "Detection", detec->DebugImage() );

			char c = cvWaitKey(1000/25);
			if( c == 27 ) break;
		}

	return false;
}
