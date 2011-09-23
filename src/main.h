/*
 * main.h
 *
 *  Created on: 27.11.2010
 *      author: Pavel Studeník
 *      email: studenik@varhoo.cz
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>

#include <gtk/gtk.h>
#include <cairo.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <string>

#include "detection_webcam.h"

#define DEFAULT_WIDTH  800
#define DEFAULT_HEIGHT 640

/* a structure keeping information about
 * the selection */
typedef struct
{
  gboolean active;   /* whether the selection is active or not */
  gdouble  x, y;
  gdouble  w, h;
}
SelectionInfo;

typedef struct
{
	/* aktuální pozice */
	gint x,y;
	/* předchozí pozice */
	gint prev_x, prev_y;

	float f_x, f_y;

	int bold;
}
MousePosition;

typedef struct
{
	gdouble r,g,b,a;
}Color;

typedef struct
{
	Detection * det;
	MousePosition * fin;
	GtkWidget     *canvas;
	GtkWidget     *canvas_set;

}TimerAction;

typedef struct
{
	CvCapture * capture;
	gint width;
	gint height;
	float fps;
	MousePosition mouse;
}Camera;

static void paint (GtkWidget      *widget,
		   GdkEventExpose *eev,
           MousePosition  *mouse);

/* forward definitions of handler for mouse events
 */
static void video (
		GtkWidget      *widget,
		GdkEventExpose *eev);

static gboolean 
	event_motion  (GtkWidget      *widget,
                               GdkEventMotion *mev,
                               MousePosition  *mouse);

static gboolean 
	event_keypress (GtkWidget     *widget,
					GdkEventKey *event );

static gboolean
	resize_window (GtkWidget      *widget,
              GdkEventMotion *mev);

gboolean timeout2(gpointer data);
#endif /* MAIN_H_ */
