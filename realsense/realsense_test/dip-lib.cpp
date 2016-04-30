/** @brief The implementations of DIP are collected in this DIP library.
 * @author : <Thomas Tsai, d04922009@ntu.edu.tw>
 *
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>             /* getopt_long() */
#include <errno.h>
#include <cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <string>
#include <algorithm>    // std::sort
#include <vector>       // std::vector

using namespace cv;
using namespace std;

#include "dip.h"

int SCR_X_OFFSET=0, SCR_Y_OFFSET=0;

/** @brief contrast power transform
 * d = s^y = pow(s, y)
 * pow = 2: square law, similar to exponential
 * pwo = 1/3: cubic root,similar to logarithmic
 */
void power_transform(uint8_t *src, uint8_t *dst, int size, float pow, int level)
{
	for(int i = 0 ; i < size; i++){
		float p = powf(src[i]/(float)level, pow);
		if(p > 1.0f ) p = 1.0f;
		dst[i]=p*level;
	}
}

/** @breif log transform
 * Stretch dark region, suppress bright region
 * log_n(b) = log_c (b) / log_c(n) = ln(b)/ln(n)
 * here n=2.0
 * G = ln(1+a*F)/ln(2), 0 <= F <=1
 */
void log_transform(uint8_t *src, uint8_t *dst, int size, float a, int level)
{
	for(int i = 0 ; i < size; i++){
		float l = logf(1.0f + a * src[i]/(float)level);
		l /= logf(2.0f);
		if(l > 1.0f) l= 1.0f;
		dst[i]=l*level;
	}
}

/** @breif inverse log transform
 * expand bright region
 *
 * G = b * (e^(aF) - 1.0), 0 <= F <=1.0
 *
 */
void invLog_transform(uint8_t *src, uint8_t *dst, int size, float a, int level)
{
	for(int i = 0 ; i < size; i++){
		//F = src[i]/(float)level
		float p = powf(M_E, a * src[i]/(float)level) - 1.0f;
		if(p > 1.0f) p= 1.0f;
		else if(p < 0.0f ) p = 0.0f;
		dst[i]=level*p;
	}
}

/** @brief flipping the image
 * org : input
 * flipped : output
 * fv : 'v' for vertically, otherwise for horizontally
 */
int flip(uint8_t *org, uint8_t *flipped, char fv)
{
	if(fv == 'v'){//vertically
		for(int i = 0 ; i < HEIGHT ; i ++)
			for(int j = 0 ; j < WIDTH ; j ++)
				flipped[(HEIGHT -1 - i) * WIDTH + j]=org[ i * WIDTH + j];
	}else{//swap 
		for(int i = 0 ; i < HEIGHT ; i ++)
			for(int j = 0 ; j < WIDTH ; j ++)
				flipped[i * WIDTH + j]=org[ i * WIDTH + (WIDTH - 1 - j)];
	}
	return 0;
}

/** @brief boundary extension of the src image to a new image by padding some rows and columns
 * around the border.
 * pad : odd extension
 */
uint8_t *boundary_ext(uint8_t *src, int width, int height, int pad, const string ename)
{
	assert(src);

	int pw=width + 2*pad;
	int ph=height + 2*pad;
	uint8_t *pad_buf=(uint8_t *)malloc( pw * ph);
	memset(pad_buf, 0, pw * ph);

	//padding row border
	for(int y = 1; y <= pad ; y++)
		for(int x = 0; x < width;x++){
			//padding upper row border
			pad_buf[x+pad + (pad-y) * pw]=src[x + y * width];
			//padding lower row border
			pad_buf[x+pad + (height-1+y+pad) * pw]=src[x + (height -1 - y) * width];
		}
	//padding column borders
	for(int y = 0; y < height ; y++)
		for(int x = 1; x <= pad;x++){
			//padding left column border
			pad_buf[pad-x + (pad+y)*pw]=src[x + y * width];
			//padding right column border
			pad_buf[width - 1 + x + pad + (pad + y ) * pw]=src[width -1 - x + y * width];
		}

	//copy source buffer to working buffer
	for(int y = 0 ; y < height; y++)
		for(int x= 0 ; x < width ;x++)
			pad_buf[ x+pad +(y+pad)*pw]=src[x+y*width];
/*
	//show the padded image
	IplImage* imgMedia = cvCreateImageHeader(cvSize(pw, ph), IPL_DEPTH_8U, 1);
	cvSetData(imgMedia, pad_buf, pw);
	//show the median filtered image
	namedWindow(ename, WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED );	// Create a window for display.
	moveWindow(ename, 1000,400+SCR_Y_OFFSET);
	cvShowImage(ename.c_str(), imgMedia );                   // Show our image inside it.
	cout << ename <<endl;
	*/
	return pad_buf;
}

/** @brief generate a M x N matrix which simulates pepper and salt, impulse noise
 * impulse noise : pepper and salt noise generator
 * black_thr : black threshold <
 * white_thr : white threshold >
 */
void impulse_noise_gen(uint8_t *imp, int M, int N, uint8_t black_thr, uint8_t white_thr)
{
	//generate random matrix
	int b=0,w=0;
	srand(time(NULL));
	memset(imp, 127, M*N);
	printf("%s:black_thr=%u, white_thr=%u\n", __func__, black_thr, white_thr);
	for(int i = 0; i < M*N; i++) {
		int r =  rand()%256;//0-255
		//printf("r=%d\n",r);
		if(r < black_thr){
			imp[i]=0;//black
			b++;
		}else if(r > white_thr){
			imp[i] = 255;//white
			w++;
		}
	}
	printf("%s:black=%d, white=%d\n", __func__,b,w);
}

void impulse_noise_add(uint8_t *imp, uint8_t *image, int M, int N)
{
	for(int i = 0; i < M*N; i++) {
		if(imp[i]==0){
			image[i]=0;//black
		}else if(imp[i] == 255){
			image[i] = 255;//white
		}
	}
}

/** @brief white noise generator: M x N matrix
 * white noise : uniform white noise generator
 * white_thr : white threshold >
 */
void white_noise_gen(uint8_t *imp, int M, int N, uint8_t white_thr)
{
	//generate random matrix
	int w=0;
	srand(time(NULL));
	memset(imp, 0, M*N);
	printf("%s:white_thr=%u\n", __func__, white_thr);
	for(int i = 0; i < M*N; i++) {
		int r =  rand()%256;//0-255
		if(r >= white_thr){
			imp[i] = r;//white
			w++;
		}
	}
	printf("%s: white=%d\n", __func__,w);
}

void white_noise_add(uint8_t *white, uint8_t *image, int M, int N )
{
	for(int i = 0; i < M*N; i++) {
		if(white[i])
			image[i] = 255;//white
	}

}

/** @brief power 2 of the diff two images : (s1[]-s2[]) * (s1[]-s2[]) = diff[]
 * 
 */
void img_diff(uint8_t *s1, uint8_t *s2, uint8_t *diff, int width, int height)
{
	for(int y = 0 ; y < height; y++)
		for(int x = 0 ; x < width;x++){
			int d = s1[x+y*width] - s2[x+y*width];
			diff[x+y*width]= d*d;
		}
}

/** @brief power 2 of the diff two images : (s1[]-s2[]) * (s1[]-s2[]) = diff[]
 * I : orignal image
 * P : processed image
 */
double img_MSE(uint8_t *I, uint8_t *P, int width, int height)
{
	double mse=0.0;
	for(int y = 0 ; y < height; y++)
		for(int x = 0 ; x < width;x++){
			int d = I[x+y*width] - P[x+y*width];
			mse += d*d;
		}
	printf("%s:mse=%f, MSE=%f\n",__func__, mse, mse/(height * width));
	return mse/(height * width);
}

/** Peak Signal to Noise Ratio
 * L : bits of the pixel
 */
float PSNR(uint8_t *I, uint8_t *P, int width, int height, int L)
{
	float mse = img_MSE(I, P, width, height);
	unsigned level = (0x1 << L ) -1;
	
	printf("%s:L=%d, level=%d\n",__func__, L, level);
	float psnr = 10.0 * logf( (level * level) / mse );
	printf("%s:psnr=%f\n",__func__, psnr);
	
	return psnr;
}

/** @brif media filter
 * dim : dim x dim mask kernel
 *
 */
void median(uint8_t *src, uint8_t *dst, int width, int height, int dim, int method)
{
	//padding border
	uint8_t window[dim*dim];
	int pad = (dim / 2);
	uint8_t *pad_buf=NULL;
	//expand source image by padding borders
	pad_buf = boundary_ext(src, width, height, pad, "median");

	for(int y = pad;  y < (height+pad); y++)
       for(int x = pad ; x < (width+pad); x++){
           int i = 0;
           for(int fx = 0; fx < dim;fx++)
               for(int fy = 0; fy < dim; fy++){
                   window[i++] = pad_buf[(x + fx - pad) + (y + fy - pad)*(width + 2*pad)];
//				   printf("[%d]=%d ", i-1, window[i-1]);
			   }
			//sorting window
			std::vector<uint8_t> myvector (window, window+dim*dim);
			// using default comparison (operator <):
			std::sort (myvector.begin(), myvector.end());
//			for(int i=0;i<dim*dim;i++)
//				printf("[%d]=%d ", i, myvector[i]);
//			printf("\nm=%d\n", myvector[ (dim * dim) / 2]);
//			cout << endl;
			//using the median element in the sorting list
			dst[(y-pad)*width+(x-pad)] = myvector[ (dim * dim) / 2];
	   }

	 free(pad_buf);
}

/** @brif median filter
 * dim : dim x dim mask kernel, dim is "odd"
 *
 */
void mean(uint8_t *src, uint8_t *dst, int width, int height, int dim, int method)
{
	//padding border
	uint8_t window[dim*dim];
	int pad = (dim / 2);
	uint8_t *pad_buf=NULL;
	//expand source image by padding borders
	pad_buf = boundary_ext(src, width, height, pad, "mean");
	
	if(method == 0){
		for(int fx = 0; fx < dim;fx++)
			for(int fy = 0; fy < dim; fy++){
				window[fx + fy * dim] = 1;
			}

		for(int y = pad;  y < (height+pad); y++)
		for(int x = pad ; x < (width+pad); x++){
			unsigned sum=0;
			//convolution with filter
			for(int fx = 0; fx < dim;fx++)
				for(int fy = 0; fy < dim; fy++){
					sum += window[fx + fy*dim] * pad_buf[(x + fx - pad) + (y + fy - pad)*(width + 2*pad)];
				}
				sum /= dim*dim;//normal kernel/filter to 1
				//using the median element in the sorting list
				dst[(y-pad)*width+(x-pad)] = sum;
		}
	}else {
		for(int fx = 0; fx < dim;fx++)
			for(int fy = 0; fy < dim; fy++){
				window[fx + fy * dim] = 1;
			}
		window[dim*dim/2]=2;
		
		for(int y = pad;  y < (height+pad); y++)
		for(int x = pad ; x < (width+pad); x++){
			unsigned sum=0;
			//convolution with filter
			for(int fx = 0; fx < dim;fx++)
				for(int fy = 0; fy < dim; fy++){
					sum += window[fx + fy*dim] * pad_buf[(x + fx - pad) + (y + fy - pad)*(width + 2*pad)];
				}
				sum /= (dim*dim + 1);//normal kernel/filter to 1
				//using the median element in the sorting list
				dst[(y-pad)*width+(x-pad)] = sum;
		}
	}
	free(pad_buf);
}

/** @brief flipping the image
 * image : 8bit grey image
 * width : width of the image
 * height : height of the image
 */
int hist(unsigned *hist_table, int h_size, uint8_t *image, int width, int height)
{
	for(int i = 0; i < h_size; i++) hist_table[i]=0;
	for(int i = 0 ; i < height*width ; i ++){
		//printf("0x%x ", image[i]);
		hist_table[image[i]]++;
	}
#ifdef DEBUG
	for(int i = 0 ; i < MAX_GREY_LEVEL;i++){
		printf("%3d: %u\n", i, hist_table[i]);
	}
#endif
}

/** histogram equlization
 * mapping original hist_table to new table histeq_map
 * input :
 * src[] : 8 bit grey image
 * dst[] : histogram equlized destination buffer
 * pixels : total pixels of the 8 bit grey image
 * hist_table[] : histogram
 * cdf_table[] : cdf of the hist_table
 * h_size : size of histogram, it's usually 256 for 8 bit grey image
 * histeq_map : the histogram equalization mapping table
 */
void hist_eq(uint8_t *src, uint8_t *dst, int pixels, unsigned *hist_table,
			 unsigned *cdf_table, int h_size, uint8_t *histeq_map)
{
	unsigned cdf=0;
	//calculate CDF without normalization from 0 to 1.0, but in 0 to pixels
	for(int i=0;i < h_size ; i++ ){
		cdf += hist_table[i];
		cdf_table[i] = cdf;
		histeq_map[i] = (MAX_GREY_LEVEL * cdf)  / pixels ;//mapping grey level i to new grey level
		//printf("%d->%d\n", i, histeq_map[i]);
	}

	//histogram equlization
	for(int i = 0; i < pixels ; i++)
		dst[i] = histeq_map[src[i]];	//mapping original grey level to new level
}

/** @brief calculate cdf from hist and output the histogram mapping table
 * for histogram equalization
 * 
 */
void hist_cdf(unsigned *hist_table, unsigned *cdf_table, int h_size, 
		int pixels, uint8_t *histeq_map)
{
	unsigned cdf=0;
	//calculate CDF without normalization from 0 to 1.0, but in 0 to pixels
	for(int i=0;i < h_size ; i++ ){
		cdf += hist_table[i];
		cdf_table[i] = cdf;
		if(histeq_map)
			histeq_map[i] = (MAX_GREY_LEVEL * cdf)  / pixels ;//mapping grey level i to new grey level
		//printf("%d->%d\n", i, histeq_map[i]);
	}
}
/** @brief local histogram equlization
 * http://angeljohnsy.blogspot.com/2011/06/local-histogram-equalization.html
 * For every pixel, based on the neighbor hood value the histogram equalization is done. 
 * Here I used 3 by 3 window matrix for explanation. By changing the window matrix size, 
 * the histogram equalization can be enhanced. By changing the values of M and N the window 
 * size can be changed in the code given below.
 * 
 * mapping original hist_table to new table histeq_map
 * input :
 * src[] : grey image
 * dst[] : the destination buffer after local histogram equalization
 * width, height : dimension of the image
 * max_grey_level : max_grey_level, default is 0-255
 * lM: the local matrix MxM to perform local H.E., odd is better!
 * name : The name of window to show
 */
void local_hist_eq(uint8_t *src, uint8_t *dst, int width, int height,
				    int max_grey_level, int lM )
{
	unsigned pixels=lM*lM;
	unsigned cdf=0;
	unsigned *hist_table=new unsigned[max_grey_level+1];
	//unsigned *cdf_table=new unsigned[max_grey_level+1];
	//padding borders
	uint8_t window[pixels];
	int pad = (lM / 2);
	uint8_t *pad_buf=NULL;
	//expand source image by padding borders
	int pw=width + 2*pad;
	int ph=height + 2*pad;
	pad_buf = boundary_ext(src, width, height, pad, "lhe");

	for(int y=0; y < height;y++)
		for(int x = 0 ; x < width ; x++){
			//clear tables
			std::fill_n(hist_table, max_grey_level+1, 0);
			//std::fill_n(cdf_table, max_grey_level+1, 0);

			//local histogram : lMxlM
			for(int ly = 0; ly < lM;ly++)
				for(int lx = 0;lx < lM; lx++)
					hist_table[ pad_buf[ (lx+x) + (ly+y) * pw] ]++;
			//cdf
			int k = src[x + y * width];
			cdf = 0;
			for(int i=0;i <= k ; i++ ){
				cdf += hist_table[i];
				//cdf_table[i] = cdf;
			}

			//local histogram equlization
			//dst[x + y * width] = cdf_table[src[x + y * width]] * max_grey_level / pixels;
			dst[x + y * width] = cdf * max_grey_level / pixels;
		}
	if(hist_table)
		delete [] hist_table;
	//if(cdf_table)
	//	delete [] cdf_table;
	if(pad_buf)
		free(pad_buf);
}