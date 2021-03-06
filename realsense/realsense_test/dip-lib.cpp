/** @brief The implementations of DIP are collected in this DIP library.
 * @author : <Thomas Tsai, d04922009@ntu.edu.tw>
 *
 */
#include <windows.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

 //opencv
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <conio.h>

#include <string>
#include <algorithm>    // std::sort
#include <vector>       // std::vector

using namespace cv;
using namespace std;

#include "dip.h"

int SCR_X_OFFSET=0, SCR_Y_OFFSET=0;

/** @brief boundary extension of the src image to a new image by padding some rows and columns
* around the border.
* pad : odd extension
*/
uint8_t *boundary_ext(uint8_t *src, int width, int height, int pad, const string ename)
{
	assert(src);

	int pw = width + 2 * pad;
	int ph = height + 2 * pad;
	uint8_t *pad_buf = (uint8_t *)malloc(pw * ph);
	memset(pad_buf, 0, pw * ph);

	//padding row border
	for (int y = 1; y <= pad; y++)
		for (int x = 0; x < width;x++) {
			//padding upper row border
			pad_buf[x + pad + (pad - y) * pw] = src[x + y * width];
			//padding lower row border
			pad_buf[x + pad + (height - 1 + y + pad) * pw] = src[x + (height - 1 - y) * width];
		}
	//padding column borders
	for (int y = 0; y < height; y++)
		for (int x = 1; x <= pad;x++) {
			//padding left column border
			pad_buf[pad - x + (pad + y)*pw] = src[x + y * width];
			//padding right column border
			pad_buf[width - 1 + x + pad + (pad + y) * pw] = src[width - 1 - x + y * width];
		}

	//copy source buffer to working buffer
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width;x++)
			pad_buf[x + pad + (y + pad)*pw] = src[x + y*width];
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

/** @brief histogram table
 * image : 8bit grey image
 * width : width of the image
 * height : height of the image
 */
void hist(unsigned *hist_table, int h_size, uint8_t *image, int width, int height)
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
	uint8_t *window= new uint8_t[pixels];
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