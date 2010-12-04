/*
 * main.cpp
 *
 *  Created on: 26.11.2010
 *      Author: Pavel Studeník, Bc.
 */

#include "main.h"
		
Color color = {1.0,1.0,1.0,1.0};

int start = false;

cairo_surface_t * surface;
cairo_surface_t * image;
//cairo_t *ci;
bool isFullSreen = 0;
MousePosition finger = {0,0,0,0,.0,.0};

int window_w, window_h;
IplImage* cv_image;

int treshold = 120;

gboolean timeout2(gpointer data){
	if(!start) return true;

	Detection * detec = ((TimerAction *) data)->det;

	cv_image = detec->DebugImage();
	//GtkWidget* widget = ((TimerAction *) data)->canvas_set;

	return true;
}

gboolean timeout(gpointer data){
	if(!start) return true;

	Detection * detec = ((TimerAction *) data)->det;
	MousePosition * finger = ((TimerAction *) data)->fin;
	GtkWidget     *canvas = ((TimerAction *) data)->canvas;

	detec->SetTreshold(treshold);

	detec->Next();

	detec->SetMouseRang(&(finger->f_x),&(finger->f_y),&(finger->bold));

	if(finger->x!=0 && finger->y!=0){
		finger->prev_x = finger->x;
		finger->prev_y = finger->y;
	}

	gtk_widget_queue_draw (canvas);

	return true;
}

gint
main (gint    argc,
      gchar **argv)
{
  GtkWidget     *window;
  GtkWidget     *settings;

  GtkWidget     *canvas;
  GtkWidget     *canvas_settings;

  //číslo zařízení
  int device = 0;
  if(argc > 2){
	  if(std::string(argv[1])==std::string("-cam")){
		  device = atoi(argv[2]);
	  }
  }

  MousePosition  mouse = {0,0,0,0,0,0,0};

  gtk_init (&argc, &argv);

  //vytvoření okna
  window   = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  settings   = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  /* signál pro konec programu */
  g_signal_connect (G_OBJECT (window), "delete-event",
                    G_CALLBACK (gtk_main_quit), NULL);

  /* vytvoření kanvasu pro okno */
  canvas = gtk_drawing_area_new ();
  gtk_widget_set_size_request (canvas, DEFAULT_WIDTH, DEFAULT_HEIGHT);


  canvas_settings = gtk_drawing_area_new ();
  gtk_widget_set_size_request (canvas_settings, 640, 480);


  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, DEFAULT_WIDTH, DEFAULT_HEIGHT);

  /* signál pro pro vykreslování na plochu */
  g_signal_connect (G_OBJECT (canvas), "expose-event",
                    G_CALLBACK (paint),
                    &mouse
                  );

  g_signal_connect (G_OBJECT (canvas_settings), "expose_event",
                      G_CALLBACK (video), NULL);

  /* signál pro stisknuté tlačítko */
  g_signal_connect (G_OBJECT (window), "key-press-event",
                    G_CALLBACK (event_keypress),
                    NULL
                   );

  g_signal_connect (G_OBJECT (settings), "key-press-event",
                      G_CALLBACK (event_keypress_set),
                      NULL
                     );

   g_signal_connect(G_OBJECT(window),"configure-event", //"expose-event", //, 
        G_CALLBACK(resize_window), G_OBJECT(window)); 

  /* add additional events the canvas widget will listen for
   */
  gtk_widget_add_events (canvas,
		  GDK_POINTER_MOTION_HINT_MASK | 
		  GDK_STRUCTURE_MASK |
		  GDK_POINTER_MOTION_MASK );

  gtk_widget_add_events (canvas_settings,
		  GDK_POINTER_MOTION_HINT_MASK |
		  GDK_STRUCTURE_MASK |
		  GDK_POINTER_MOTION_MASK );

  /* connect our rubber banding callbacks, like the paint callback
   * we pass the selection as userdata
   */

  g_signal_connect (G_OBJECT (canvas), "motion_notify_event",
                    G_CALLBACK (event_motion),
                    &mouse
                  );

  image = cairo_image_surface_create_from_png ("data/brush.png");

  gtk_container_add (GTK_CONTAINER (window), canvas);

  gtk_container_add (GTK_CONTAINER (settings), canvas_settings);

  //CvCapture * capture = cvCaptureFromAVI("data/video2.avi");
  CvCapture * capture = cvCaptureFromCAM(device);
  Detection * detec = new Detection(capture);

  TimerAction timer_action = { detec, &mouse, canvas, canvas_settings};

  gtk_widget_show (canvas_settings);

  gtk_widget_show_all (window);
  gtk_widget_show_all (settings);

  /* Nastavení časovače */
  g_timeout_add(200, timeout, (gpointer) &timer_action);

  g_timeout_add(500, timeout2, (gpointer) &timer_action);
  
  gtk_main ();

  return 0;
}



/* function to draw the rectangular selection
 */
static void
paint_selection (cairo_t       *ci,
                 MousePosition *mouse)
{
	int w = cairo_image_surface_get_width (surface);
	int h = cairo_image_surface_get_height (surface);

	/** draw brush */ /*
	cairo_surface_t* test;
	cairo_t *cb;
	double brush_w = 70, brush_h = 70;
	int w = cairo_image_surface_get_width (image);
	int h = cairo_image_surface_get_height (image);

	test = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, w, h );
	cb = cairo_create(test);
	cairo_scale (cb, brush_w/w, brush_h/h);
	cairo_set_source_surface (cb, image, 0, 0);
	cairo_mask_surface(cb, test, 0, 0);

	cairo_paint(cb);

	cairo_destroy (cb);

//    cairo_set_source_rgb (ci, 0.5,0.5,0.5);
	cairo_paint(ci);

	cairo_set_source_surface (ci, test, mouse->x-(brush_w/2),mouse->y-(brush_h/2));
	cairo_paint(ci);
	*/	
	
	/*	mouse->x = 100;
	mouse->y = 100;
	mouse->prev_x = 250;
	mouse->prev_y = 100;*/
	
	//printf("%f %f\n",mouse->f_x,mouse->f_y);
	if(!(mouse->f_x>0 && mouse->f_y > 0)) return;

	mouse->x = (int) (w-mouse->f_x*w);
	mouse->y = (int) (mouse->f_y*h);
	//printf("(%d %d) %d %d < %f %f\n",w,h,mouse->x,mouse->y,mouse->f_x,mouse->f_y);

	/* proložení vykreslování */
	float test_t = sqrt(pow(mouse->x-mouse->prev_x,2)+
	pow(mouse->y-mouse->prev_y,2));

	//omezení pokud jsou velké skoky
	if(test_t==0 || test_t > 100) return;

	int size_brush = 5+mouse->bold;
	int step = int (test_t/size_brush);

	//délka jednoho tahu
	double step_x = (mouse->x - mouse->prev_x)/(double) step;
	double step_y = (mouse->y - mouse->prev_y)/(double) step;
	
	cairo_save (ci);
	  /* proložení ploch */
	  for(int i=0; i < step+1; i++){
		 cairo_set_source_rgba (ci, color.r, color.g, color.b, 1.);
		 cairo_arc (ci, mouse->prev_x+step_x*i,mouse->prev_y+step_y*i,  size_brush, 0, 2 * M_PI);
		 cairo_fill_preserve (ci);
		 cairo_stroke (ci);
	  }
	  //vykreslení plochy
	  cairo_set_source_rgba (ci, color.r, color.g, color.b, 1.);
	  cairo_arc (ci, mouse->x, mouse->y,  size_brush, 0, 2 * M_PI);
	  cairo_fill_preserve (ci);
	  cairo_stroke (ci);

	  cairo_restore (ci);
}

/* funkce která vykesluje */
static void video (
		GtkWidget      *widget,
		GdkEventExpose *eev){

	gtk_widget_queue_draw(GTK_WIDGET(widget));

	if(!start) return;

	if(cv_image){

		/* vypsání ladícího textu */
		CvFont font;
		double hScale=0.5;
		double vScale=0.5;
		int    lineWidth=1;
		char buffer[125];

		cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);
		sprintf(buffer,"Treshold %d/255 [key m,n]",treshold);
		cvPutText (cv_image,buffer,cvPoint(10,20), &font, cvScalar(0,255,0));
		cvLine(cv_image,cvPoint(260,15),cvPoint(260+treshold,15),cvScalar(0,255,0),10,4,NULL);

		//převedení opencv do gtk
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
						 0, 0, 0, 0, 640, 480, GDK_RGB_DITHER_NONE, 0, 0 );
		gtk_widget_queue_draw(widget);
		gdk_pixbuf_unref (pix);
	} else {
		printf("ERROR: Video nebylo načteno.\n");
	}

}

static void
paint (GtkWidget      *widget,
       GdkEventExpose *eev,
       MousePosition  *mouse)
{
	gint width, height;

	cairo_t *cr;
	cairo_t *ci;

	width  = widget->allocation.width;
	height = widget->allocation.height;

	// vykreslení kreslící plochy
	ci = cairo_create (surface);
	paint_selection (ci, mouse);
	cairo_destroy (ci);

	cr = gdk_cairo_create (widget->window);

	// nastavení pozadí kreslící plochy
	cairo_set_source_rgb (cr, 0.0,0.0,0.0);

	cairo_paint (cr);

	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_mask_surface(cr, surface, 0, 0);

	cairo_destroy(cr);
}

static gboolean
event_keypress_set (GtkWidget     *widget,
				GdkEventKey *event )
{
	switch(event->keyval){
			exit(0);
	}

	return true;
}
/**
 * Funkce na zpracování událostí myši
 */
static gboolean
event_keypress (GtkWidget     *widget,
				GdkEventKey *event )
{

	switch(event->keyval){
	// nastavení barev
	case 'r':
		color.b = 0.0; color.r = 1.0; color.g = 0.0;
		break;
	case 'b':
		color.b = 1.0; color.r = 0.0; color.g = 0.0;
		break;
	case 'g':
		color.b = 0.0; color.r = 0.0; color.g = 1.0;
		break;
	// vymazání plochy
	case 'c':
		{
		cairo_t * ci = cairo_create (surface);
	    cairo_set_source_rgb (ci, 0.0,0.0,0.0);
	    cairo_paint (ci);
		cairo_destroy (ci);
		}
		break;
	// spuštění režimu ve fullsreenmodu
	case 'f':
		isFullSreen = !isFullSreen;
		if(isFullSreen){
			gdk_window_fullscreen(widget->window);
		}
		else {
			gdk_window_unfullscreen(widget->window);
			window_w = DEFAULT_WIDTH;
			window_h = DEFAULT_HEIGHT;
		}
		break;
	/* spuštění programu */
	case 's':
		start = true;
	break;
	/* nastavení treshold pro upravu obrázku */
	case 'n':
		if(treshold<255) treshold++; break;
	/* nastavení treshold pro upravu obrázku */
	case 'm':
		if(treshold>0) treshold--; break;

	//escape
	case 65307:
	case 'q':
		exit(0);
		break;
	}

	return TRUE;
}

static gboolean
event_motion (GtkWidget      *widget,
              GdkEventMotion *mev,
              MousePosition  *mouse)
{
  mouse->prev_x = mouse->x;
  mouse->prev_y = mouse->y;

  mouse->x = mev->x;
  mouse->y = mev->y;

  /* tell the canvas widget that it needs to redraw itself */
  gtk_widget_queue_draw (widget);
  return TRUE;
}

static gboolean
resize_window (GtkWidget      *widget,
              GdkEventMotion *mev)
{
	int w,h;
	if(isFullSreen)
		gdk_window_get_size(widget->window,&w,&h);
	else {
		w = DEFAULT_WIDTH;	
		h = DEFAULT_HEIGHT;	
	}
		
	if(w!=window_w && h!=window_h){
		window_w = w;
		window_h = h;

		surface = cairo_surface_create_similar (
          surface, 
          CAIRO_CONTENT_COLOR_ALPHA,
          w, h);
		gtk_widget_set_size_request(widget,w,h);
	}

  /* tell the canvas widget that it needs to redraw itself */
 	 gtk_widget_queue_draw (widget);
  return FALSE;
}

