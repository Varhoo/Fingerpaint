/*
 * detection.cpp
 *
 *  Created on: 27.11.2010
 *      Author: Pavel Studeník
 */
#include <gtk/gtk.h>
#include  <sys/time.h>
#include  <time.h>
#include <unistd.h>
#include <pthread.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>
#include "detection_class.h"

#include <vector>

#define D_TIMER_VIDEO 50
#define D_TIMER_ANIME 50

int d_fps = 1;
int dtime = time(NULL);

Detection * detec;

static int currently_drawing = 0;
static int currently_framing = 0;
static int currently_anime = 0;

static int start = 0;

//the global pixmap that will serve as our buffer
static GdkPixmap *pixmap = NULL;

static void 
on_window_destroy (GtkObject *object, gpointer user_data)
{
        gtk_main_quit();
}


class game_object {

public:
	float x;
	float y;

	game_object(float _x,float _y){ printf("%f, %f \n", _x,_y); x = _x; y = _y; };
};

 std::vector <game_object> vgames;

gboolean on_window_configure_event(GtkWidget * da, GdkEventConfigure * event, gpointer user_data){
    static int oldw = 0;
    static int oldh = 0;
    //make our selves a properly sized pixmap if our window has been resized
    if (oldw != event->width || oldh != event->height){
        //create our new pixmap with the correct size.
        GdkPixmap *tmppixmap = gdk_pixmap_new(da->window, event->width,  event->height, -1);
        //copy the contents of the old pixmap to the new pixmap.  This keeps ugly uninitialized
        //pixmaps from being painted upon resize
        int minw = oldw, minh = oldh;
        if( event->width < minw ){ minw =  event->width; }
        if( event->height < minh ){ minh =  event->height; }
        gdk_draw_drawable(tmppixmap, da->style->fg_gc[GTK_WIDGET_STATE(da)], pixmap, 0, 0, 0, 0, minw, minh);
        //we're done with our old pixmap, so we can get rid of it and replace it with our properly-sized one.
        g_object_unref(pixmap); 
        pixmap = tmppixmap;
    }
    oldw = event->width;
    oldh = event->height;
    return TRUE;
}

gboolean on_window_expose_event(GtkWidget * da, GdkEventExpose * event, gpointer user_data){
    gdk_draw_drawable(da->window,
        da->style->fg_gc[GTK_WIDGET_STATE(da)], pixmap,
        // Only copy the area that was exposed.
        event->area.x, event->area.y,
        event->area.x, event->area.y,
        event->area.width, event->area.height);
    return TRUE;
}

//do_draw will be executed in a separate thread whenever we would like to update
//our animation
void *do_draw(void * user_data){
	 //return false;
    currently_drawing = 1;

	 GtkWidget  * widget = (GtkWidget * ) user_data;

    int width, height;
    gdk_threads_enter();
    //gdk_drawable_get_size(canvas, &width, &height);
    gdk_window_get_size(widget->window,&width,&height);

    //cairo_t * cr = gdk_cairo_create (widget->window);
	 gdk_threads_leave();    

    //create a gtk-independant surface to draw on
    cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(cst);
 	 


    // all drawing here 
    cairo_set_source_rgb (cr, 0.0,0.0,0.0);
	 cairo_paint (cr);

    //do some time-consuming drawing
    static int i = 0;
    ++i; i = i % 300;   //give a little movement to our animation

    int j,k;
    for(k=0; k<100; ++k){   //lets just redraw lots of times to use a lot of proc power
        for(j=0; j < 1000; ++j){
            cairo_set_source_rgb (cr, (double)j/1000.0, (double)j/1000.0, 1.0 - (double)j/1000.0);
            cairo_move_to(cr, i,j/2); 
            cairo_line_to(cr, i+100,j/2);
            cairo_stroke(cr);
        }
    }
    cairo_destroy(cr);


    //When dealing with gdkPixmap's, we need to make sure not to
    //access them from outside gtk_main().
    gdk_threads_enter();

    cairo_t *cr_pixmap = gdk_cairo_create(pixmap);
    cairo_set_source_surface (cr_pixmap, cst, 0, 0);
    cairo_paint(cr_pixmap);
    cairo_destroy(cr_pixmap);

    gdk_threads_leave();

	 //    cairo_surface_destroy(cst);
	 printf(":%d",i);

	 if(dtime!=time(NULL)) { printf("%d %d\n",(int) time(NULL),d_fps); dtime = time(NULL) ; d_fps = 1; } else { d_fps++; }

    currently_drawing = 0;

    return NULL;
}

gboolean 
timer_frame ( gpointer user_data )
{

   //printf("currently_framing %d\n",currently_framing);

   if(currently_framing==0){
		//lock painting
		currently_framing = 1;

		detec->SetTreshold(180);
   	
		detec->Next();
		//printf("currently_framing %d\n",currently_framing);
   	GtkWidget  * widget = (GtkWidget * ) user_data;
		gtk_widget_queue_draw(GTK_WIDGET(widget));		
   }
}




double Microtime(){
     timeval tim;
     gettimeofday(&tim, NULL);
     return (tim.tv_sec+(tim.tv_usec/1000000.0));
}

static float gtime = Microtime();

gboolean 
timer_anime2 ( gpointer user_data )
{

   //printf("currently_framing %d\n",currently_framing);
	printf("time %f %f %f\n", Microtime()-gtime,Microtime(),gtime);

	if((Microtime()-gtime) > 2.){
		float y = drand48();
		game_object gm = game_object(0,y);
		vgames.push_back(gm);
		gtime = Microtime();
	}

	for(int k = 0; k < vgames.size(); k++) {
		vgames[k].x += 0.01;
	}

   if(currently_anime==0){
		//lock painting
		currently_anime = 1;

		//printf("currently_framing %d\n",currently_framing);
   	GtkWidget  * widget = (GtkWidget * ) user_data;
		gtk_widget_queue_draw(GTK_WIDGET(widget));		
   }
}

gboolean 
timer_anime ( gpointer user_data )
{

    static gboolean first_execution = TRUE;
	 GtkWidget  * widget = (GtkWidget * ) user_data;

    //use a safe function to get the value of currently_drawing so
    //we don't run into the usual multithreading issues
    int drawing_status = g_atomic_int_get(&currently_drawing);

    //if we are not currently drawing anything, launch a thread to 
    //update our pixmap
    if(drawing_status == 0){
        static pthread_t thread_info;
        int  iret;
        if(first_execution != TRUE){
            pthread_join(thread_info, NULL);
        }
        iret = pthread_create( &thread_info, NULL, do_draw, user_data);
    }

    //tell our window it is time to draw our animation.
    int width, height;
    gdk_drawable_get_size(pixmap, &width, &height);
    gtk_widget_queue_draw_area(widget, 0, 0, width, height);

    first_execution = FALSE;

    return TRUE;
}

void click_fullcreen (GtkWidget *widget, gpointer data)
{
	 GtkWidget * window = (GtkWidget *) data;
    if (GTK_TOGGLE_BUTTON (widget)->active) 
    {
        /* If control reaches here, the toggle button is down */
        printf("ok\n");
		 gdk_window_fullscreen(window->window);
		  start = 1;
    } else {
    	  printf("ko\n");
        /* If control reaches here, the toggle button is up */
		  gdk_window_unfullscreen(window->window);
    }
	 //int w, h;
	 //gdk_window_get_size(window->window,&w,&h);
	 //gtk_widget_set_size_request(window,w,h);
}


static void 
paints_anime (GtkObject *object, gpointer user_data)
{

   if(!start) return;

	GtkWidget * widget = (GtkWidget *) object;

   cairo_t *cr;
	cairo_t *ci;

	cr = gdk_cairo_create (widget->window);

	// nastavení pozadí kreslící plochy
	cairo_set_source_rgb (cr, 0.0,0.0,0.0);

	cairo_paint (cr);
   
   static int i = 0;
	i++;

	//cairo_set_source_surface(cr, surface, 0, 0);
	//cairo_mask_surface(cr, surface, 0, 0);
	int h,w;
	gdk_window_get_size(widget->window,&w,&h);

  // printf("%d %d \n",w,h);

	for(int k = 0; k < vgames.size(); k++) {

		 cairo_set_source_rgba (cr, 1.,1.,0., 1.);
		 cairo_arc (cr, vgames[k].x*w,vgames[k].y*h,  15, 0, 2 * M_PI);
		 cairo_fill_preserve (cr);
		 cairo_stroke (cr);
   }

	cairo_destroy(cr);


	currently_anime = 0;
}

static void 
paints_video (GtkObject *object, gpointer user_data)
{
	//gtk_widget_queue_draw(GTK_WIDGET(object)); only for timer
	
	// image of opencv to gtk pixmap
/*	GdkPixbuf * pix = gdk_pixbuf_new_from_data(
			(guchar*) cv_image->imageData,
			GDK_COLORSPACE_RGB,
			FALSE,
			cv_image->depth,
			cv_image->width,
			cv_image->height,
			cv_image->widthStep,
			NULL,
			NULL);

	 //gtkImg = gtk_image_new_from_pixbuf (pix);
	gdk_draw_pixbuf (object->window,
					 object->style->fg_gc[GTK_WIDGET_STATE (object)],
					 pix,
					 0, 0, 0, 0, 640, 480, GDK_RGB_DITHER_NONE, 0, 0 );
	gtk_widget_queue_draw(object);
	gdk_pixbuf_unref (pix);
*/
	//printf(".");

   IplImage* cv_image = detec->getFrame();
   GtkWidget  * widget = (GtkWidget * ) object;

	GdkPixbuf * pix = gdk_pixbuf_new_from_data(
				(guchar*)cv_image->imageData,
				GDK_COLORSPACE_RGB,
				FALSE,
				cv_image->depth,
				cv_image->width,
				cv_image->height,
				cv_image->widthStep,
				NULL,
				NULL);
		 //gtkImg = gtk_image_new_from_pixbuf (pix);
		gdk_draw_pixbuf (widget->window,
						 widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
						 pix,
						 0, 0, 0, 0, 640, 480, /*GDK_RGB_DITHER_NORMAL*/ GDK_RGB_DITHER_NONE , 0, 0 );
		//gtk_widget_queue_draw(widget);
		
		gdk_pixbuf_unref (pix);
		currently_framing = 0;
}

int
main (int argc, char *argv[])
{
        GtkBuilder              *builder;
        GtkWidget               *window, *window_paint, * fullscreen;
		  GtkWidget     			  *canvas, *canvas_screen;
        GdkGLConfig *gl_config;

	//we need to initialize all these functions so that gtk knows
	//to be thread-aware
	if (!g_thread_supported ()){ 
		g_thread_init(NULL); 
	}
	gdk_threads_init(); 
	gdk_threads_enter();


        gtk_init (&argc, &argv);

		  //opengl
//        gtk_gl_init (&argc, &argv);

//		  gl_config = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB | GDK_GL_MODE_ALPHA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);

        builder = gtk_builder_new ();
        gtk_builder_add_from_file (builder, "gui/settings.builder", NULL);

        window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
        canvas = GTK_WIDGET (gtk_builder_get_object (builder, "canvas"));
		  window_paint = GTK_WIDGET (gtk_builder_get_object (builder, "windowscreen"));
		  canvas_screen = GTK_WIDGET (gtk_builder_get_object (builder, "canvasscreen")); 

		  fullscreen = GTK_WIDGET (gtk_builder_get_object (builder, "fullscreen"));
        gtk_builder_connect_signals (builder, NULL);          
        g_object_unref (G_OBJECT (builder));

	//signals for event
	g_signal_connect (G_OBJECT (canvas), "expose-event",
                G_CALLBACK (paints_video), NULL
                  );


	g_signal_connect (G_OBJECT (canvas_screen), "expose-event",
                G_CALLBACK (paints_anime), NULL
                  );

	g_signal_connect(G_OBJECT(window), "destroy", 
		G_CALLBACK(on_window_destroy), NULL
		);

   //g_signal_connect(G_OBJECT(window_paint), "expose_event", G_CALLBACK(on_window_expose_event), NULL);
   // g_signal_connect(G_OBJECT(window_paint), "configure_event", G_CALLBACK(on_window_configure_event), NULL);

	g_signal_connect(G_OBJECT(fullscreen), "clicked", 
		G_CALLBACK(click_fullcreen ), (gpointer) window_paint
		);

   gtk_widget_set_size_request(canvas_screen,500,500);

// opencv 
	CvCapture * capture = cvCaptureFromCAM(-1);      
   detec = new Detection(capture);

	//we can turn off gtk's automatic painting and double buffering routines.
	//gtk_widget_set_app_paintable(window_paint, TRUE);
	//gtk_widget_set_double_buffered(window_paint, FALSE);

	g_timeout_add(D_TIMER_ANIME, (GSourceFunc) timer_anime2, (gpointer) canvas_screen);
	g_timeout_add(D_TIMER_VIDEO, (GSourceFunc) timer_frame, (gpointer) canvas);

   gtk_widget_show (window_paint);   
   gtk_widget_show(window);
//	time_handler(window);    

	//pixmap = gdk_pixmap_new(window_paint->window,500,500,-1);

        gtk_main ();
	gdk_threads_leave();
        
        return 0;
}
