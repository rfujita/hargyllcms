## Process this file with automake to produce Makefile.in

# Common CFLAGS for Argyll components
AM_CFLAGS = -DUNIX -I $(top_srcdir)/cgats -I$(top_srcdir)/gamut		\
    -I$(top_srcdir)/h -I$(top_srcdir)/icc				\
    -I$(top_srcdir)/numlib -I$(top_srcdir)/plot -I$(top_srcdir)/profile	\
    -I$(top_srcdir)/render -I$(top_srcdir)/rspl -I$(top_srcdir)/spectro	\
    -I$(top_srcdir)/target -I$(top_srcdir)/xicc				\
    $(TIFF_CFLAGS)

noinst_LIBRARIES = libargyll.a

libargyll_a_SOURCES = ../h/llist.h ../h/sort.h ../h/counters.h ../h/xlist.h ../h/copyright.h ../h/config.h

libargyll_a_SOURCES += ../cgats/pars.c ../cgats/pars.h ../cgats/cgats.c ../cgats/cgats.h ../cgats/parsstd.c	\
	../cgats/cgatsstd.c

libargyll_a_SOURCES += ../gamut/gamut.h ../gamut/gamut.c

libargyll_a_SOURCES += ../gamut/gammap.h ../gamut/gammap.c ../gamut/nearsmth.c ../gamut/nearsmth.h

libargyll_a_SOURCES += ../plot/plot.h ../plot/plot.c

libargyll_a_SOURCES += ../plot/vrml.h ../plot/vrml.c

libargyll_a_SOURCES += ../profile/prof.h ../profile/profin.c ../profile/profout.c

libargyll_a_SOURCES += ../render/render.h ../render/render.c

libargyll_a_SOURCES += ../rspl/rspl.h ../rspl/rspl_imp.h ../rspl/mlbs.h ../rspl/rspl.c ../rspl/scat.c ../rspl/rev.c	\
	../rspl/rev.h ../rspl/gam.c ../rspl/gam.h ../rspl/spline.c ../rspl/opt.c

libargyll_a_SOURCES += ../scanin/scanrd.h ../scanin/scanrd_.h ../scanin/scanrd.c

libargyll_a_SOURCES += ../spectro/insttypes.h ../spectro/insttypes.c ../spectro/insttypeinst.h

libargyll_a_SOURCES += ../spectro/inst.h ../spectro/inst.c ../spectro/insttypes.c ../spectro/dtp20.c ../spectro/dtp20.h ../spectro/dtp22.c	\
	../spectro/dtp22.h ../spectro/dtp41.c ../spectro/dtp41.h ../spectro/dtp51.c ../spectro/dtp51.h ../spectro/dtp92.c ../spectro/dtp92.h		\
	../spectro/i1disp.c ../spectro/i1disp.h ../spectro/i1pro.c ../spectro/i1pro.h ../spectro/i1pro_imp.c ../spectro/i1pro_imp.h	\
	../spectro/munki.c ../spectro/munki_imp.c ../spectro/ss.c ../spectro/ss.h ../spectro/ss_imp.c ../spectro/ss_imp.h ../spectro/hcfr.c ../spectro/hcfr.h	\
	../spectro/spyd2.c ../spectro/spyd2.h ../spectro/spyd2setup.h ../spectro/spyd2PLD.h ../spectro/huey.c ../spectro/huey.h ../spectro/unixio.c	\
	../spectro/usbio.c ../spectro/hidio.c ../spectro/pollem.c ../spectro/pollem.h ../spectro/icoms.h ../spectro/conv.h ../spectro/usbio.h	\
	../spectro/hidio.h ../spectro/munki.h ../spectro/munki_imp.h

libargyll_a_SOURCES += ../spectro/dispsup.c ../spectro/dispwin.c ../spectro/dispwin.h ../spectro/dispsup.h

libargyll_a_SOURCES += ../spectro/conv.c ../spectro/pollem.c 

libargyll_a_SOURCES += ../target/alphix.c ../target/alphix.h ../target/randix.c ../target/randix.h


libargyll_a_SOURCES += ../xicc/xicc.h ../xicc/xicc.c ../xicc/xlutfix.c ../xicc/xspect.c ../xicc/xspect.h ../xicc/xsep.c	\
	../xicc/xsep.h ../xicc/xdevlin.c ../xicc/xdevlin.h ../xicc/xcam.c ../xicc/xcam.h ../xicc/cam97s3.c ../xicc/cam97s3.h	\
	../xicc/cam02.c ../xicc/cam02.h ../xicc/mpp.c ../xicc/xfit.c ../xicc/xfit.h ../xicc/moncurve.c ../xicc/moncurve.h	\
	../xicc/mpp.h ../xicc/xdgb.c ../xicc/xdgb.h ../xicc/xcal.c ../xicc/xcal.h ../xicc/xcolorants.c ../xicc/xcolorants.h

libargyll_a_SOURCES += ../xicc/xutils.h ../xicc/xutils.c

libargyll_a_SOURCES += ../numlib/numlib.h ../numlib/numsup.c ../numlib/numsup.h ../numlib/dnsq.c ../numlib/dnsq.h	\
	../numlib/powell.c ../numlib/powell.h ../numlib/dhsx.c ../numlib/dhsx.h ../numlib/ludecomp.c ../numlib/ludecomp.h ../numlib/svd.c	\
	../numlib/svd.h ../numlib/zbrent.c ../numlib/zbrent.h ../numlib/rand.c ../numlib/rand.h ../numlib/sobol.c ../numlib/sobol.h ../numlib/aatree.c ../numlib/aatree.h

EXTRA_DIST= ../xicc/xmono.c ../xicc/xmatrix.c ../xicc/xlut.c

