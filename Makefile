
PROG     =  magoo
VER      =  0.1
DEFS     =  -DPROGRAM_NAME=${PROG} -DPROGRAM_VER=${VER}
DEPS     =  x11 cairo freetype2 gdk-2.0 gdk-pixbuf-2.0
CFLAGS   += -g $(shell pkg-config --cflags ${DEPS}) ${DEFS}
LDLIBS   += $(shell pkg-config --libs ${DEPS}) -lm -lreadline
PREFIX   ?= /usr
MODULES  =  cairo commands console config magoo xlib
HEADERS  =  magoo.h
#MANPAGES =
VPATH    =  src

${PROG}: ${MODULES:%=%.o}

%.o: %.c ${HEADERS}

install: ${PROG}
	@install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}
	@install -Dm644 share/${PROG}.png ${DESTDIR}${PREFIX}/share/pixmaps/${PROG}.png
	@install -Dm644 share/${PROG}.desktop ${DESTDIR}${PREFIX}/share/applications/${PROG}.desktop

clean:
	@rm -f ${PROG}-${VER}.tar.gz ${MODULES:%=%.o}

distclean: clean
	@rm -f ${PROG}

dist: distclean
	@tar -czf ${PROG}-${VER}.tar.gz *

.PHONY: clean dist distclean
