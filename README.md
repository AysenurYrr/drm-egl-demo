


EGL + OpenGL ES + GBM desteği yükler

sudo apt update
sudo apt install libdrm-dev libgbm-dev libegl1-mesa-dev libgles2-mesa-dev


CFLAGS = -Wall -I/usr/include/libdrm
koydum öünkü 
/usr/include/xf86drm.h:40:10: fatal error: drm.h: No such file or directory
   40 | #include <drm.h>
      |          ^~~~~~~

      sebebiyle

Run the following command to check available DRM devices:
ls /dev/dri 

Add Your User to the video Group: Run the following command to add your user to the video group, for permissions on card0
sudo usermod -aG video $USER


Ensure your GPU supports EGL and GBM. Use tools like eglinfo or glxinfo to verify:
sudo apt install mesa-utils
sudo apt install mesa-utils-extra
eglinfo //


Ensure your GPU supports the required GBM and EGL features. Use eglinfo to confirm supported configurations:
eglinfo | grep -i gbm
If GBM support is missing, you may need to update your drivers or use a different GPU.

Missing seperator on Makefile,
Remove spaces and make indented with a tab

tty ekranına CTRL+F3 ile geçiş yapıp windowless bir sistemde görüntüyü bastırabilirsin.
sonrasında örneği kapatmak için CTRL + C yap ve sonra CTRL + F2 yapman gerekmektedir. Uyarı, Eğer CTRL + C öncesinde direk CTRL + F2 yaparsan ekran donabilir.