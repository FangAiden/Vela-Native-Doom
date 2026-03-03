#ifndef VELA_FB_H
#define VELA_FB_H

typedef unsigned short fb_coord_t;

struct fb_videoinfo_s {
    unsigned char  fmt;
    fb_coord_t     xres;
    fb_coord_t     yres;
    unsigned char  nplanes;
};

struct fb_planeinfo_s {
    void          *fbmem;
    unsigned int   fblen;
    fb_coord_t     stride;
    unsigned char  display;
    unsigned char  bpp;
    unsigned int   xres_virtual;
    unsigned int   yres_virtual;
    unsigned int   xoffset;
    unsigned int   yoffset;
};

#define FBIOGET_VIDEOINFO    0x2801
#define FBIOGET_PLANEINFO    0x2802
#define FBIOPAN_DISPLAY      0x2803
#define FBIOSET_POWER        0x2804
#define FBIOPUT_AREA         0x2805
#define FBIOGET_PANELINFO    0x2808
#define FBIO_UPDATE          0x280a

struct fb_area_s {
    fb_coord_t x;
    fb_coord_t y;
    fb_coord_t w;
    fb_coord_t h;
};

#define FB_FMT_RGB16_565     11
#define FB_FMT_RGB24         12
#define FB_FMT_RGB32         13
#define FB_FMT_RGBA32        21

#define PROT_READ   1
#define PROT_WRITE  2
#define MAP_SHARED  1
#define MAP_FAILED  ((void *)-1)

#endif
