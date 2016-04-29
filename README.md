# README.md markdown #
<https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet>
# How to install opencv 3.1 in visual studio 2015: #
<https://www.youtube.com/watch?v=l4372qtZ4dc> 

##setup environment variables in win10:##
1. OPENCV_BUILD=d:\opencv-3.1.0\build\x64\vc14\    
2. OPENCV_DIR=d:\opencv-3.1.0\  

##setup include path in vs2015##
VS2015 include: $(OPENCV_DIR)\build\include  
VS2015 lib path :$(OPENCV_BUILD)\lib\  
input lib: opencv_world310d.lib  

#intel realsense #
env variables : $(RSSDK_DIR)  
vs2015 c/c++  code generation: /MTD  
vs2015 include : $(RSSDK_DIR)\include\;$(RSSDK_DIR)\sample\common\include\  
vs2015 lib path : $(RSSDK_DIR)\lib\x64;$(RSSDK_DIR)\sample\common\lib\x64\v140\  
input lib : libpxcmd_d.lib libpxc_d.lib libpxcutils_d.lib libpxcutilsmd_d.lib

#command line
-listio
-nframes: total frames to capture
-sdname
-csize : %dx%dx%d => color image :width x height x fps
-dsize : %dx%dx%d => depth image :width x height x fps
-isize : %dx%dx%d => irda image :width x height x fps
-lsize : %dx%dx%d => left image :width x height x fps
-rsize : %dx%dx%d => right image :width x height x fps
-file
-record
-noRender
-mirror: mirror the display, so capture is like a morror

detail : $(RSSDK_DIR)\sample\common\src\util_cmdline.cpp