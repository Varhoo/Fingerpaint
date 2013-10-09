#include "freenect.h"

freenect_context *f_ctx;
freenect_device *f_dev;

freenect_video_format requested_format = FREENECT_VIDEO_RGB;
freenect_video_format current_format = FREENECT_VIDEO_RGB;

// led and angle for mechanic motor
int freenect_angle = 30;
int freenect_led;
int nr_devices;
bool die;
pthread_mutex_t gl_backbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gl_frame_cond = PTHREAD_COND_INITIALIZER;

uint8_t *rgb_back;
uint8_t *depth_mid, *depth_front;
uint8_t *rgb_mid, *rgb_front;
int got_rgb;
int got_depth;
uint16_t t_gamma[2048];

int filter_distance(IplImage * img, int distance){
    printf("%u ",img->imageData[1]);
    for(int i = 0; i < img->height; i++){
        for(int j = 0; j < img->width; j++){
           if( //img->imageData[(i*img->width + j)*3+0 ] > 0 ||
              img->imageData[(i*img->width + j)*3+2 ] > 0
             ){
                img->imageData[(i*img->width+j)*3+0] = 0; 
                img->imageData[(i*img->width+j)*3+1] = 0; 
                img->imageData[(i*img->width+j)*3+2] = 0; 
           }
        }
    } 
    return 0;
}


int freenect::filter_from = 512;
int freenect::filter_to = 768;

freenect::freenect(void){
    // size of buffer
    max_width = 640;
    max_height = 480;
    // init freenect

    // init error   
    error = 0;

    // init size of depth
    depth_mid = (uint8_t*)malloc(640*480*3);
    depth_front = (uint8_t*)malloc(640*480*3);
    rgb_back = (uint8_t*)malloc(640*480*3);
    rgb_mid = (uint8_t*)malloc(640*480*3);
    rgb_front = (uint8_t*)malloc(640*480*3);


    frame_rgb = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);
    frame_dph = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);

    got_rgb = 0;
    got_depth = 0;

	if (freenect_init(&f_ctx, NULL) < 0) {
      error = 1;
		printf("freenect_init() failed\n");
		return;
	}

   // debug information on stdio
	// freenect_set_log_level(f_ctx, FREENECT_LOG_DEBUG);
	freenect_select_subdevices(f_ctx, (freenect_device_flags)(FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

   // get number od kinect devices
	nr_devices = freenect_num_devices (f_ctx);
	printf ("Number of devices found: %d\n", nr_devices);

   // if device not found that nothing to do
	if (nr_devices < 1) {
		freenect_shutdown(f_ctx);
      error = 2;
	   printf ("Device don't found\n");
		return;
	}

	for (int i=0; i<2048; i++) {
		float v = i/2048.0;
		v = powf(v, 3)* 6;
		t_gamma[i] = v*6*256;
	}
}

int freenect::reload(){

    pthread_mutex_lock(&gl_backbuf_mutex);

	// When using YUV_RGB mode, RGB frames only arrive at 15Hz, so we shouldn't force them to draw in lock-step.
	// However, this is CPU/GPU intensive when we are receiving frames in lockstep.
	if (current_format == FREENECT_VIDEO_YUV_RGB) {
		while (!got_depth && !got_rgb) {
			pthread_cond_wait(&gl_frame_cond, &gl_backbuf_mutex);
		}
	} else {
		while ((!got_depth || !got_rgb) && requested_format != current_format) {
			pthread_cond_wait(&gl_frame_cond, &gl_backbuf_mutex);
		}
	}

	if (requested_format != current_format) {
		pthread_mutex_unlock(&gl_backbuf_mutex);
		return 2;
	}

	uint8_t *tmp;
	//uint32_t *tmp32;

	if (got_depth) {
		tmp = depth_front;
		depth_front = depth_mid;
		depth_mid = tmp;
		got_depth = 0;
	}
	if (got_rgb) {
		tmp = rgb_front;
		rgb_front = rgb_mid;
		rgb_mid = tmp;
		got_rgb = 0;
	}

	pthread_mutex_unlock(&gl_backbuf_mutex);

    memcpy(frame_rgb->imageData,
         (char *)rgb_front,
         frame_rgb->width * frame_rgb->height * frame_rgb->nChannels);
    memcpy(frame_dph->imageData,
         (char *)depth_front,
         frame_dph->width * frame_dph->height * frame_dph->nChannels);

    //cvCvtColor(frame_dph,frame_dph,CV_BGR2RGB);
    //cvCvtColor(frame_rgb,frame_rgb,CV_BGR2RGB);
    return 0;
}

void freenect::set_filter(int from, int to){
   filter_from = from;
   filter_to = to;
}

void freenect::depth_cb(freenect_device *dev, void *v_depth, uint32_t timestamp)
{
	int i;
	uint16_t *depth = (uint16_t*)v_depth;
	pthread_mutex_lock(&gl_backbuf_mutex);
	for (i=0; i<640*480; i++) {
		int pval = t_gamma[depth[i]];
		int lb = pval & 0xff;
      int tt = pval >> 8;
      if (pval < filter_from || pval > filter_to) tt = 6; // temporary
		switch (tt /* >>8 */ ) {
			case 0:
				depth_mid[3*i+0] = 255;
				depth_mid[3*i+1] = 255-lb;
				depth_mid[3*i+2] = 255-lb;
				break;
			case 1:
				depth_mid[3*i+0] = 255;
				depth_mid[3*i+1] = lb;
				depth_mid[3*i+2] = 0;
				break;
			case 2:
				depth_mid[3*i+0] = 255-lb;
				depth_mid[3*i+1] = 255;
				depth_mid[3*i+2] = 0;
				break;
			case 3:
				depth_mid[3*i+0] = 0;
				depth_mid[3*i+1] = 255;
				depth_mid[3*i+2] = lb;
				break;
			case 4:
				depth_mid[3*i+0] = 0; //0;
				depth_mid[3*i+1] = 255-lb;
				depth_mid[3*i+2] = 255;
				break;
			case 5:
				depth_mid[3*i+0] = 0;
				depth_mid[3*i+1] = 0;
				depth_mid[3*i+2] = 255-lb;
				break;
			default:
				depth_mid[3*i+0] = 0;
				depth_mid[3*i+1] = 0;
				depth_mid[3*i+2] = 0;
				break;
		}
	}
	got_depth++;
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

void freenect::rgb_cb(freenect_device *dev, void *rgb, uint32_t timestamp)
{
	pthread_mutex_lock(&gl_backbuf_mutex);

	// swap buffers
	assert (rgb_back == rgb);
	rgb_back = rgb_mid;
	freenect_set_video_buffer(dev, rgb_back);
	rgb_mid = (uint8_t*)rgb;

	got_rgb++;
	pthread_cond_signal(&gl_frame_cond);
	pthread_mutex_unlock(&gl_backbuf_mutex);
}

//static void* startThread(void* pVoid);
void * freenect::freenect_threadfunc(void * arg){
   int accelCount = 0;

	//freenect_set_tilt_degs(f_dev,freenect_angle);
	freenect_set_led(f_dev,LED_RED);
	freenect_set_depth_callback(f_dev, freenect::depth_cb);
	freenect_set_video_callback(f_dev, freenect::rgb_cb);
	freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, current_format));
	freenect_set_depth_mode(f_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
	freenect_set_video_buffer(f_dev, rgb_back);

	freenect_start_depth(f_dev);
	freenect_start_video(f_dev);

	printf("'w'-tilt up, 's'-level, 'x'-tilt down, '0'-'6'-select LED mode, 'f'-video format\n");

	while (!die && freenect_process_events(f_ctx) >= 0) {
		//Throttle the text output
		if (accelCount++ >= 2000)
		{
			accelCount = 0;
			freenect_raw_tilt_state* state;
			freenect_update_tilt_state(f_dev);
			state = freenect_get_tilt_state(f_dev);
			double dx,dy,dz;
			freenect_get_mks_accel(state, &dx, &dy, &dz);
			printf("\r raw acceleration: %4d %4d %4d  mks acceleration: %4f %4f %4f", state->accelerometer_x, state->accelerometer_y, state->accelerometer_z, dx, dy, dz);
			fflush(stdout);
		}

		if (requested_format != current_format) {
			freenect_stop_video(f_dev);
			freenect_set_video_mode(f_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, requested_format));
			freenect_start_video(f_dev);
			current_format = requested_format;
		}
	}

	printf("\nshutting down streams...\n");

	freenect_stop_depth(f_dev);
	freenect_stop_video(f_dev);

	freenect_close_device(f_dev);
	freenect_shutdown(f_ctx);

	printf("-- done!\n");
	return NULL;

}


void freenect::init(short user_device_number){
   if (error!=0) {
      printf("No init kinect. Error %d\n",error);
      return;
   }
	if (freenect_open_device(f_ctx, &f_dev, user_device_number) < 0) {
		printf("Could not open device\n");
		freenect_shutdown(f_ctx);
      error = 3;
		return;
	}

   int res = pthread_create(&freenect_thread, NULL, freenect::freenect_threadfunc, NULL);
	if (res) {
		printf("pthread_create failed\n");
      error = 4;
		freenect_shutdown(f_ctx);
		return;
	}
}

freenect::~freenect(void){

   pthread_join(freenect_thread, NULL);

   free(depth_mid);
   free(depth_front);
   free(rgb_back);
   free(rgb_mid);
   free(rgb_front);
}

/**
 * Get opencv image from camera
 * @return pointer to IplImage
 * */
IplImage * freenect::get_image_rgb(){
   return frame_rgb;
}

/**
 * Get opencv image from depth camera
 * @return pointer to IplImage
 * */
IplImage * freenect::get_image_depth_rgb(){
   return frame_dph;
}

int freenect::get_error(){
   return error;
}
