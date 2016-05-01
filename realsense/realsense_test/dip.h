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

/** @brief flipping the image
 * image : 8bit grey image
 * width : width of the image
 * height : height of the image
 */
void hist(unsigned *hist_table, int h_size, uint8_t *image, int width, int height);


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