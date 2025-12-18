mkdir ../bin
cp -r fonts ../bin/
cp -r ../src/shaders ../bin/

./fonts/atlas ./fonts/Iceland-Regular.ttf ../bin/fonts/font_small.cmfont 64 > /dev/null
./fonts/atlas ./fonts/Iceland-Regular.ttf ../bin/fonts/font_large.cmfont 108 > /dev/null

g++ -g -o ../bin/battleship_4d \
	../src/game/main.cpp ../src/window/xlib/xlib_window.cpp ../src/time/unix/unix_time.cpp ../src/renderer/opengl/opengl.cpp \
	../src/renderer/opengl/GL/gl3w.c \
	-I ../src/ \
	-lX11 -lX11-xcb -lGL -lm -lxcb -lXfixes
