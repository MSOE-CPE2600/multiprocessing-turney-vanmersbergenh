/**********************************************
*  Filename: mandel.c
*  Description: Uses multiprocessing and multithreading to create mandelbrot images
*  Compile: make
*  Modified by: Hunter Van Mersbergen
*  Date: 11/24/2025
*  Class: CPE2600 121
***********************************************/

/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "jpegrw.h"
#include <sys/wait.h>
#include <pthread.h>

typedef struct thread_data {
	imgRawImage* img;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	int max;
	int start_row;
	int end_row;
} thread_data_t;

// local routines
static int iteration_to_color( int i, int max );
static int iterations_at_point( double x, double y, int max );
static void compute_image( imgRawImage *img, double xmin, double xmax,
									double ymin, double ymax, int max, int threads);
static void show_help();
static void* thread_image(void* thread_struct);


int main( int argc, char *argv[] )
{
	char c;

	int num_procs = 1;
	int num_threads = 1;

	// These are the default configuration values used
	// if no command line arguments are given.
//	const char *outfile = "mandel.jpg";
	double xcenter = 0;
	double ycenter = 0;
	double xscale = 4;
	double yscale = 0; // calc later
	int    image_width = 1000;
	int    image_height = 1000;
	int    max = 1000;

	// For each command line argument given,
	// override the appropriate configuration value.

	while((c = getopt(argc,argv,"x:y:s:W:H:m:n:t:h"))!=-1) {
		switch(c) 
		{
			case 'x':
				xcenter = atof(optarg);
				break;
			case 'y':
				ycenter = atof(optarg);
				break;
			case 's':
				xscale = atof(optarg);
				break;
			case 'W':
				image_width = atoi(optarg);
				break;
			case 'H':
				image_height = atoi(optarg);
				break;
			case 'm':
				max = atoi(optarg);
				break;
			case 'h':
				show_help();
				exit(1);
				break;
			case 'n':
				num_procs = atoi(optarg);
				break;
			case 't':
				num_threads = atoi(optarg);
				break;
		}
	}

	// Calculate y scale based on x scale (settable) and image sizes in X and Y (settable)

	// Display the configuration of the image.
	printf("mandel: x=%lf y=%lf xscale=%lf yscale=%1f max=%d outfile=%s\n",xcenter,ycenter,xscale,yscale,max, "mandel##.jpeg");
	
	int procs = 0;
	int images = 0;

	while (images < 50)
	{
		if (procs >= num_procs)
		{
			wait(NULL);
			procs--;
		}
		if (procs < num_procs)
		{
			int pid = fork();
			if (pid == 0)
			{
				xcenter = 0.01 * images;
				xscale = images * 0.1 + 0.1;
				yscale = xscale / image_width * image_height;

				char filename[12] = {"mandel##.jpg"};
				filename[6] = (images / 10) + '0';
				filename[7] = (images % 10) + '0';
				char *outfile = filename;
				// Create a raw image of the appropriate size.
				imgRawImage* img = initRawImage(image_width,image_height);

				// Fill it with a black
				setImageCOLOR(img,0);

				// Compute the Mandelbrot image
				compute_image(img,xcenter-xscale/2,xcenter+xscale/2,ycenter-yscale/2,ycenter+yscale/2,max,num_threads);

				// Save the image in the stated file.
				storeJpegImageFile(img,outfile);

				// free the mallocs
				freeRawImage(img);
				exit(0);
			}
			else 
			{
				procs++;
				printf("Procs: %d\n", procs);
				printf("Images: %d\n", images);
				images++;
			}
		}
	}
	while (procs > 0)
	{
		wait(NULL);
		procs--;
		printf("Procs: %d\n", procs);
	}

	return 0;
}

/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) {

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max, int threads)
{
	pthread_t tids[threads];
	thread_data_t thread_data[threads];
	
	int height = img->height;

	int thread_rows = height / threads;
	// For every pixel in the image...

	//split work among threads
	for (int t = 0; t < threads; t++) {
		thread_data[t].img = img;
		thread_data[t].xmin = xmin;
		thread_data[t].xmax = xmax;
		thread_data[t].ymin = ymin;
		thread_data[t].ymax = ymax;
		thread_data[t].max = max;
		thread_data[t].start_row = t * thread_rows;
		if (t == threads - 1) {
			thread_data[t].end_row = height;
		} else {
			thread_data[t].end_row = (t + 1) * thread_rows;
		}

		pthread_create(&tids[t], NULL, (void*)thread_image, (void*)&thread_data[t]);
	}

	for (int t = 0; t < threads; t++) {
		pthread_join(tids[t], NULL);
	}
}

/**
* Coomputes a portion of the image in a thread
*
* @param thread_struct Pointer to thread_data_t struct
* @return NULL
*/
void* thread_image(void* thread_struct) 
{
	thread_data_t* data = (thread_data_t*)thread_struct;
	imgRawImage* img = data->img;
	double xmin = data->xmin;
	double xmax = data->xmax;
	double ymin = data->ymin;
	double ymax = data->ymax;
	int max = data->max;
	int start_row = data->start_row;
	int end_row = data->end_row;

	int width = img->width;
	int height = img->height;

	int i,j;

	// same code from compute_image but for only the
	// part of the image the thread is computing
	for(j=start_row;j<end_row;j++) {

		for(i=0;i<width;i++) {

			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			setPixelCOLOR(img,i,j,iteration_to_color(iters,max));
		}
	}

	return NULL;
}


/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0xFFFFFF*iters/(double)max;
	return color;
}


// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}
