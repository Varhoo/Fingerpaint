#include <gtk/gtk.h>
#include  <time.h>
#include <unistd.h>
#include <pthread.h>

#define D_TIMER 50

int d_fps = 1;
int dtime = time(NULL);

static int currently_drawing = 0;
GtkWidget     		*canvas;

static void 
on_window_destroy (GtkObject *object, gpointer user_data)
{
        gtk_main_quit();
}

//do_draw will be executed in a separate thread whenever we would like to update
//our animation
void *do_draw(void *ptr){

    currently_drawing = 1;

    int width, height;
    gdk_threads_enter();
//    gdk_drawable_get_size(canvas, &width, &height);
    if(dtime!=time(NULL)) { printf("%d %d\n",(int) time(NULL),d_fps); dtime = time(NULL) ; d_fps = 1; } else { d_fps++; }

    gdk_threads_leave();

    //create a gtk-independant surface to draw on
/*    cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(cst);

    // all drawing here 

    cairo_destroy(cr);


    //When dealing with gdkPixmap's, we need to make sure not to
    //access them from outside gtk_main().
    gdk_threads_enter();

    cairo_t *cr_pixmap = gdk_cairo_create(pixmap);
    cairo_set_source_surface (cr_pixmap, cst, 0, 0);
    cairo_paint(cr_pixmap);
    cairo_destroy(cr_pixmap);*/

//    gdk_threads_leave();

//    cairo_surface_destroy(cst);

    currently_drawing = 0;

    return NULL;
}


gboolean 
timer_frame ( gpointer user_data )
{

    static gboolean first_execution = TRUE;

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
        iret = pthread_create( &thread_info, NULL, do_draw, NULL);
    }

    //tell our window it is time to draw our animation.
//    int width, height;
//    gdk_drawable_get_size(pixmap, &width, &height);
//    gtk_widget_queue_draw_area(window, 0, 0, width, height);

    first_execution = FALSE;

    return TRUE;
}


static void 
paints (GtkObject *object, gpointer user_data)
{
	gtk_widget_queue_draw(GTK_WIDGET(object));
	
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

}

int
main (int argc, char *argv[])
{
        GtkBuilder              *builder;
        GtkWidget               *window;
        
	//we need to initialize all these functions so that gtk knows
	//to be thread-aware
	if (!g_thread_supported ()){ 
		g_thread_init(NULL); 
	}
	gdk_threads_init();
//	gdk_threads_enter();


        gtk_init (&argc, &argv);
        
        builder = gtk_builder_new ();
        gtk_builder_add_from_file (builder, "gui/settings.builder", NULL);

        window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
        canvas = GTK_WIDGET (gtk_builder_get_object (builder, "canvas"));
        gtk_builder_connect_signals (builder, NULL);          
        g_object_unref (G_OBJECT (builder));

	//signals for event
	g_signal_connect (G_OBJECT (canvas), "expose-event",
                G_CALLBACK (paints), NULL
                  );
	g_signal_connect(G_OBJECT(window), "destroy", 
		G_CALLBACK(on_window_destroy), NULL
		);
      

	//we can turn off gtk's automatic painting and double buffering routines.
	gtk_widget_set_app_paintable(window, TRUE);
	gtk_widget_set_double_buffered(window, FALSE);

	g_timeout_add(D_TIMER, (GSourceFunc) timer_frame, (gpointer) NULL);

//        gtk_widget_show (window);   
	gtk_widget_show_all(window);
//	time_handler(window);    

        gtk_main ();
//	gdk_threads_leave();
        
        return 0;
}
