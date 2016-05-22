The OpenGL Extension Wrangler (GLEW) is used to access the modern OpenGL API functions(version 3.2 up to latest version).If we use an ancient version of OpenGL then we can access the OpenGL functions simply including as #include <GL/gl.h>.But in modern OpenGL, the API functions are determined at run time, not compile time. GLEW will handle the run time loading of the OpenGL API.About GLEW see here

GLFW or freeglut will allow us to create a window, and receive mouse and keyboard input in a cross-platform way. OpenGL does not handle window creation or input, so we have to use these library for handling window, keyboard, mouse, joysticks, input and other purpose.

GLFW and freeglut are alternative for us according to our need we can choose any one but GLEW is different from them which is used for run time loading of the OpenGL API.
