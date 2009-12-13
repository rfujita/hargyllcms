AC_INIT([hargyll], [1.1.0])
AM_INIT_AUTOMAKE([1.9 foreign])

AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_INSTALL

AM_PROG_LIBTOOL
AC_PROG_LIBTOOL

AC_PATH_XTRA

AC_CHECK_LIB(m, sqrt)
AC_CHECK_LIB(pthread, pthread_create)

AC_CHECK_LIB(tiff, TIFFOpen, TIFF_LIBS="-ltiff", , -lm)
AC_SUBST([TIFF_LIBS])

AC_CHECK_LIB(X11, XOpenDisplay, X_LIBS="$X_LIBS -lX11")
AC_CHECK_LIB(Xext, XextFindDisplay, X_LIBS="$X_LIBS -lXext",,-lX11)
AC_CHECK_LIB(Xss, XScreenSaverSuspend, X_LIBS="$X_LIBS -lXss",,-lXext -lX11)
AC_CHECK_LIB(Xrandr, XRRRootToScreen, X_LIBS="$X_LIBS -lXrandr",,-LXext -lX11)
AC_CHECK_LIB(Xinerama, XineramaQueryScreens, X_LIBS="$X_LIBS -lXinerama",,-LXext -lX11)
AC_CHECK_LIB(Xxf86vm, XF86VidModeGetGamma, X_LIBS="$X_LIBS -lXxf86vm",,-lXext -lX11)
AC_SUBST([X_LIBS])

AC_CHECK_LIB(usb, usb_open, USB_LIBS="-lusb", , -lusb)
AC_SUBST([USB_LIBS])

# udev rules go in /lib, not /usr/lib
if test "$prefix" = "/usr" -o "$prefix" = "/usr/local" ; then
  slashlibdir=/lib
else
  slashlibdir=$prefix/lib
fi
AC_SUBST(slashlibdir)

AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 cgats/Makefile
                 doc/Makefile
                 gamut/Makefile
                 h/Makefile
                 icc/Makefile
                 libusb/Makefile
                 link/Makefile
                 numlib/Makefile
                 plot/Makefile
                 profile/Makefile
                 ref/Makefile
                 render/Makefile
                 rspl/Makefile
                 scanin/Makefile
                 spectro/Makefile
                 target/Makefile
                 tweak/Makefile
                 xicc/Makefile
                 tools/Makefile])
AC_OUTPUT

