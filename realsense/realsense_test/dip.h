/** @brief All dip implementations are collected in this DIP library
 * @author : <Thomas Tsai, d04922009@ntu.edu.tw>
 *
 */
#ifndef _H_DIP_LIB_H
#define	_H_DIP_LIB_H
#include <iostream>
#include <string>
#include <algorithm>    // std::sort
#include <vector>       // std::vector

#ifndef M_E
# define M_E		2.7182818284590452354
#endif

#define	DEFAULT_BIT_DEPTH	(8)
#define	MAX_GREY_LEVEL	((1<<DEFAULT_BIT_DEPTH) -1)

#define	WIDTH	(256)
#define	HEIGHT	(256)

#define	HIST_WIN_WIDTH 	(256)
#define	HIST_WIN_HEIGHT	(256)

/** @brief contrast power transform
 * d = s^y
 * pow = 2: square law, similar to exponential
 * pwo = 1/3: cubic root,similar to logarithmic
 */
void power_transform(uint8_t *src, uint8_t *dst, int size, float pow=2.0, int level=MAX_GREY_LEVEL);

/** @breif log transform
 * Stretch dark region, suppress bright region
 * log_n(b) = log_c (b) / log_c(n) = ln(b)/ln(n)
 * here n=2.0
 * G = ln(1+a*F)/ln(2), 0 <= F <=1
 */
void log_transform(uint8_t *src, uint8_t *dst, int size, float a=1.0, int level=MAX_GREY_LEVEL);

/** @breif inverse log transform
 * expand bright region
 *
 * G = b * (e^(aF) - 1.0), 0 <= F <=1.0
 *
 */
void invLog_transform(uint8_t *src, uint8_t *dst, int size, float a=1.0f, int level=MAX_GREY_LEVEL);
/** @brief flipping the image
 * org : input
 * flipped : output
 * fv : 'v' for vertically, otherwise for horizontally
 */
int flip(uint8_t *org, uint8_t *flipped, char fv);

/** @brief generate a M x N matrix which simulates pepper and salt, impulse noise
 * impulse noise : pepper and salt noise generator
 * black_thr : black threshold <
 * white_thr : white threshold >
 */
void impulse_noise_gen(uint8_t *imp, int M, int N, uint8_t black_thr, uint8_t white_thr);
void impulse_noise_add(uint8_t *imp, uint8_t *image, int M, int N);

/** @brief white noise generator: M x N matrix
 * white noise : uniform white noise generator
 * white_thr : white threshold >
 */
void white_noise_gen(uint8_t *imp, int M, int N, uint8_t white_thr);
void white_noise_add(uint8_t *white, uint8_t *image, int M, int N );
/** @brief expanding src image to a new image by padding some rows and columns
 * around the border, so the matrix kernel can operate on the border of the Original
 * image.
 */
uint8_t *expand_border(uint8_t *src, int width, int height, int pad);

void img_diff(uint8_t *s1, uint8_t *s2, uint8_t *diff, int width, int height);

/** @brief power 2 of the diff two images : (s1[]-s2[]) * (s1[]-s2[]) = diff[]
 * I : orignal image
 * P : processed image
 */
double img_MSE(uint8_t *I, uint8_t *P, int width, int height);

/** Peak Signal to Noise Ratio
 * L : bits of the pixel, default is 8 bits
 */
float PSNR(uint8_t *I, uint8_t *P, int width, int height, int L=8);

/** @brif media filter
 * dim : dim x dim mask kernel
 *
 */
void median(uint8_t *src, uint8_t *dst, int width, int height, int dim=3, int method=0);

/** @brif media filter
 * dim : dim x dim mask kernel
 *
 */
void mean(uint8_t *src, uint8_t *dst, int width, int height, int dim=3, int method=0);

/** @brief flipping the image
 * image : 8bit grey image
 * width : width of the image
 * height : height of the image
 */
int hist(unsigned *hist_table, int h_size, uint8_t *image, int width, int height);


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
			 unsigned *cdf_table, int h_size, uint8_t *histeq_map);

/** @brief calculate cdf from hist and output the histogram mapping table
 * for histogram equalization
 * 
 */
void hist_cdf(unsigned *hist_table, unsigned *cdf_table, int h_size, 
		int pixels, uint8_t *histeq_map=NULL);

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
				    int max_grey_level, int lM );

extern int SCR_X_OFFSET, SCR_Y_OFFSET;

#endif	//_H_DIP_LIB_H