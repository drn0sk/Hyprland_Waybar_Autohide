.DELETE_ON_ERROR:

.PHONY : all clean

all : waybar_autohide.so

waybar_autohide.so : waybar_autohide.cpp
	$(CXX) -shared -fPIC -std=c++2b -O3 -flto -o waybar_autohide.so waybar_autohide.cpp `pkg-config --cflags pixman-1 libdrm hyprland pangocairo libinput libudev wayland-server xkbcommon`

clean :
	-rm waybar_autohide.so
