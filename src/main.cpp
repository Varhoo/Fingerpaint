/*
 * main.cpp
 *
 *  Created on: 27.11.2010
 *      author: Pavel Studeník
 *      email: studenik@varhoo.cz
 */

#include "main.h"
		
Color color = {1.0,1.0,1.0,1.0};

int start = false;
int TIMER = 1000/25;

//static int currently_drawing = 0;
static int currently_framing = 0;

double timer[4];

cairo_surface_t * surface;
cairo_surface_t * image = NULL;
//cairo_t *ci;
bool isFullSreen = 0;
bool paint_circle = false;
static int brush_type = 0;
MousePosition finger = {0,0,0,0,.0,.0};

static int window_w, window_h;
static IplImage* cv_image;
static int treshold = 180;
static int fps = 0, __fps = 0, fpstime;
static bool show_logo_bool = true;

char color_key[10][16];

gboolean timeout2(gpointer data){
	if(!start) return true;

	Detection * detec = ((TimerAction *) data)->det;

	cv_image = detec->DebugImage();
	//cv_image = detec->GetFrame();
	GtkWidget* widget = ((TimerAction *) data)->canvas_set;

   gtk_widget_queue_draw (widget);

	return true;
}

gboolean timeout(gpointer data){
	if(!start or currently_framing ) return true;

    //time debug
	double t = Microtime();	

    //lock process
	currently_framing = 1;

    //fps counter for debug
	__fps++;

	if(time(NULL)-fpstime > 0) {  fps = __fps;  __fps = 0; fpstime=time(NULL);}

    // get value from universe structur
	Detection * detec = ((TimerAction *) data)->det;
	MousePosition * finger = ((TimerAction *) data)->fin;
	GtkWidget     *canvas = ((TimerAction *) data)->canvas;

    //set trashold value
	detec->SetTreshold(treshold);

	timer[2] =  Microtime();
	detec->Next();
	timer[2] =  Microtime()-timer[2];
	timer[3] =  detec->GetTimer();

	detec->SetMouseRang(&(finger->f_x),&(finger->f_y),&(finger->bold));

	if(finger->x!=0 && finger->y!=0){
		finger->prev_x = finger->x;
		finger->prev_y = finger->y;
	}

	gtk_widget_queue_draw (canvas);

	timer[0] =  Microtime() - t;
	
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

  char * path_mask = NULL;

  //číslo zařízení
  int device = 0;
  for(int i = 1; i <argc; i++){
	  if(std::string(argv[i])==std::string("--cam")){
		  device = atoi(argv[i+1]);
	  }
	  if(std::string(argv[i])==std::string("--mask")){
		  path_mask = argv[i+1];
	  }
  }

  printf("Number of device: %d \n",device);

  MousePosition  mouse = {0,0,0,0,0,0,0};

  gtk_init (&argc, &argv);

  //vytvoření okna
  window   = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  settings   = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  /* signál pro konec programu */
  g_signal_connect (G_OBJECT (window), "delete-event",
                    /*G_CALLBACK (gtk_main_quit), NULL);*/
                    G_CALLBACK (main_quit), NULL);

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
                      G_CALLBACK (event_keypress),
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

  if(path_mask != NULL)
      image = cairo_image_surface_create_from_png (path_mask);

  gtk_container_add (GTK_CONTAINER (window), canvas);
  gtk_container_add (GTK_CONTAINER (settings), canvas_settings);

  CvCapture * video_capture = cvCaptureFromCAM(device);
  Detection * Detect = new Detection(video_capture);
  //Set parametr for detection
  //max area size, min area size, delate/erode filter
  Detect->SetParameter(100000,100,5);         

  TimerAction timer_action = { Detect, &mouse, canvas, canvas_settings};

  gtk_widget_show (canvas_settings);

  gtk_widget_show_all (window);
  gtk_widget_show_all (settings);

  preload(timer_action);
  gtk_main ();

  return 0;
}

static void
preload (TimerAction timer_action) 
{

	/* show logo */
  show_logo_bool = true;

	printf("loading...\n");
	CvFileStorage* fs=cvOpenFileStorage("data/config.xml", 0,CV_STORAGE_READ);
	//save color
	char buffer[64];
   for(int i=0; i <=10; i++) {
		sprintf(buffer,"color_key_%d",i);
		char * tmp = (char *) cvReadStringByName(fs, NULL, buffer);
		memcpy(color_key[i],tmp,strlen(tmp)+1);
		printf("read color %d: #%s\n",i,color_key[i]);
	}
	treshold = cvReadIntByName( fs, NULL, "treshold");
	cvReleaseFileStorage( &fs);

  /* loading timer */
  g_timeout_add(TIMER, timeout, (gpointer) &timer_action);
  g_timeout_add(TIMER, timeout2, (gpointer) &timer_action);

}

/* function to draw the rectangular selection
 */
static void
paint_selection (cairo_t       *ci,
                 MousePosition *mouse)
{

	double t =  Microtime();

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

	int size_brush;
	//omezení pokud jsou velké skoky
	if(test_t==0 || test_t > 100) return;

	cairo_save (ci);

   if(brush_type == 0){
		size_brush = 5+mouse->bold/15;
		cairo_set_line_width (ci, 0.);
	} else {
		size_brush = 5+mouse->bold*0.75;
	}
	int step = int (test_t/size_brush);
	int rate_rand = 10;
//printf("%d %d\n",size_brush, mouse->bold);
	//délka jednoho tahu
	double step_x = (mouse->x - mouse->prev_x)/(double) step;
	double step_y = (mouse->y - mouse->prev_y)/(double) step;

	float aplha = 1.;
	
	if(brush_type == 2) {
		aplha = 0.2;
	}
	
	  /* proložení ploch */
	  for(int i=0; i < step+1; i++){
if(brush_type == 0){
		 cairo_set_source_rgba (ci, color.r, color.g, color.b, 1.);
		 cairo_arc (ci, mouse->prev_x+step_x*i,mouse->prev_y+step_y*i,  size_brush, 0, 2 * M_PI);
		 cairo_fill_preserve (ci);
		 cairo_stroke (ci);
} else {
	  cairo_set_source_rgba (ci, color.r, color.g, color.b, aplha);
	  for(int j=0; j < 5; j++){
//		printf("%f ",10*(drand48()*2.-1.));
		cairo_arc (ci, (mouse->prev_x+step_x*i)+rate_rand*(drand48()*2.-1.), (mouse->prev_y+step_y*i)+rate_rand*(drand48()*2.-1.),  size_brush, 0, 2 * M_PI);
		cairo_fill_preserve (ci);
		cairo_stroke (ci);
	  }
}
	  }
	  //vykreslení plochy
if(brush_type == 0){
	  cairo_set_source_rgba (ci, color.r, color.g, color.b, 1.);
	  cairo_arc (ci, mouse->x, mouse->y,  size_brush, 0, 2 * M_PI);
	  cairo_fill_preserve (ci);
	  cairo_stroke (ci);
} else {
	  cairo_set_source_rgba (ci, color.r, color.g, color.b, aplha);
	  for(int j=0; j < 5; j++){
//		printf("%f ",10*(drand48()*2.-1.));
		cairo_arc (ci, mouse->x+rate_rand*(drand48()*2.-1.), mouse->y+rate_rand*(drand48()*2.-1.),  size_brush/5, 0, 2 * M_PI);
		cairo_fill_preserve (ci);
		cairo_stroke (ci);
	  }
}
//	cairo_restore (ci);
if(image!=NULL){
	int _w = cairo_image_surface_get_width (image);
	int _h = cairo_image_surface_get_height (image);
	//cairo_surface_t* test;
	//cairo_t *cb;
	//test = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, _w, _h );
    //printf("%d %d\n",_w,_h);
	//cb = cairo_create(test);
    cairo_scale  (ci, w/(double)_w, h/(double)_h);
    cairo_set_source_surface (ci, image, 0, 0);
    cairo_paint(ci);
}
	cairo_restore (ci);


	  timer[1] =  Microtime() - t;
}

/* draw to video canvas */
static void video (
		GtkWidget      *widget,
		GdkEventExpose *eev){

	//gtk_widget_queue_draw(GTK_WIDGET(widget));

	if(!start) return;

	if(cv_image!=NULL){

		double t =  Microtime();

		/* vypsání ladícího textu */
		CvFont font;
		double hScale=0.5;
		double vScale=0.5;
		int    lineWidth=1;
		char buffer[125];

		cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);
		sprintf(buffer,"Treshold %d/255 [key m,n]",treshold);
		cvPutText (cv_image,buffer,cvPoint(10,20), &font, cvScalar(0,255,0));
		cvLine(cv_image,cvPoint(260,15),cvPoint(260+treshold,15),cvScalar(0,255,0),10,4);

		sprintf(buffer,"Timer: %f | %f | %f | %f",timer[0], timer[1], timer[2],timer[3]);
		cvPutText (cv_image,buffer,cvPoint(10,40), &font, cvScalar(255,0,0));
		sprintf(buffer,"Fps: %d ", fps);
		cvPutText (cv_image,buffer,cvPoint(10,60), &font, cvScalar(255,0,0));

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
		//gtk_widget_queue_draw(widget);
		gdk_pixbuf_unref (pix);
		
		timer[2] =  Microtime() - t;
	} else {
		//printf("ERROR: Video nebylo načteno.\n");
	}

	currently_framing = 0;
}

void SavePaint(){
	char buffer[64];
	sprintf(buffer,"paints/img_%d.png",(int) time(NULL));		
	cairo_surface_write_to_png (surface, buffer);
}

void CleanPaint(){
	cairo_t * ci = cairo_create (surface);
	cairo_set_source_rgb (ci, 0.0,0.0,0.0);
	cairo_paint (ci);
	cairo_destroy (ci);
}

static void
paint (GtkWidget      *widget,
       GdkEventExpose *eev,
       MousePosition  *mouse)
{
	//gint width, height;

	cairo_t *cr;
	cairo_t *ci;

	//width  = widget->allocation.width;
	//height = widget->allocation.height;

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


/**
	keyevents for paint windows thet is show by dataprojector
*/
static gboolean
event_keypress (GtkWidget     *widget,
				GdkEventKey *event )
{
	unsigned int r,g,b;
	//printf("debug color %d %c\n", event->keyval, event->keyval);
	switch(event->keyval){
	// key set color for brush
	case '65456':
	case '0':
		sscanf(color_key[0],"%2X%2X%2X",&r,&g,&b);
		color.b = b/255.0; color.r = r/255.0; color.g = g/255.0;
		break;
	case 65457:
	case '1':
		sscanf(color_key[1],"%2X%2X%2X",&r,&g,&b);
		color.b = b/255.0; color.r = r/255.0; color.g = g/255.0;
		break;
	case 65458:
	case '2':
		sscanf(color_key[2],"%2X%2X%2X",&r,&g,&b);
		color.b = b/255.0; color.r = r/255.0; color.g =  g/255.0;
		break;
	case 65459:
	case '3':
		sscanf(color_key[3],"%2X%2X%2X",&r,&g,&b);
		color.b = b/255.0; color.r = r/255.0; color.g = g/255.0;
		break;
	case 65460:
	case '4':
		sscanf(color_key[4],"%2X%2X%2X",&r,&g,&b);
		color.b = b/255.0; color.r = r/255.0; color.g = g/255.0;
		break;
	case 65461:
	case '5':
		sscanf(color_key[5],"%2X%2X%2X",&r,&g,&b);
		color.b = b/255.0; color.r = r/255.0; color.g = g/255.0;
		break;
	case 65462:
	case '6':
		sscanf(color_key[6],"%2X%2X%2X",&r,&g,&b);
		color.b = b/255.0; color.r = r/255.0; color.g = g/255.0;
		break;
	case 65463:
	case '7':
		sscanf(color_key[7],"%2X%2X%2X",&r,&g,&b);
		color.b = b/255.0; color.r = r/255.0; color.g = g/255.0;
		break;
	case 65464:
	case '8':
		sscanf(color_key[8],"%2X%2X%2X",&r,&g,&b);
		color.b = b/255.0; color.r = r/255.0; color.g = g/255.0;
		break;
	case 65465:
	case '9':
		sscanf(color_key[9],"%2X%2X%2X",&r,&g,&b);
		color.b = b/255.0; color.r = r/255.0; color.g = g/255.0;
		break;

	case 65455:
	case '/':
		brush_type = 0;
		break;
	case '+':
		brush_type = 1;
		break;
	case '*':
	case 'a':
		brush_type = 2;
		break;
	case 65535:
		SavePaint();
		CleanPaint();
		break;

	// vymazání plochy0
	case 'c':
		{
			SavePaint();
			CleanPaint();
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
	case 'm':
		if(treshold<255) treshold++; break;
	/* nastavení treshold pro upravu obrázku */
	case 'n':
		if(treshold>0) treshold--; break;

	//escape
	case 65307:
	case 'q':
		main_quit ();
		break;
	}

	return true;
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

/**
 * windows resize funtion
*/
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
		
		CleanPaint();
	}

  /* tell the canvas widget that it needs to redraw itself */
 	 gtk_widget_queue_draw (widget);
  return FALSE;
}

/** function save data to xml file
  * xml file  is in path data/config.xml
  */
void save_settings(void){
	CvFileStorage* fs=cvOpenFileStorage("data/config.xml", 0,CV_STORAGE_WRITE);
	//save color
	char buffer[64];
   for(int i=0; i <=10; i++) {
		sprintf(buffer,"color_key_%d",i);
		cvWriteString( fs, buffer, color_key[i] );
	}
	cvWriteInt( fs, "treshold",treshold);
	cvReleaseFileStorage( &fs);
}

/** exit function */
void main_quit(void){
	printf("save config ... \t");
	save_settings();
	printf("[ok]\n");
	printf("exit..\n");
	gtk_main_quit();
}
