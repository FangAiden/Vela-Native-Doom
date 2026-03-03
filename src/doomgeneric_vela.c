#include "vela_doom.h"
#include "vela_fb.h"
#include "doomgeneric.h"
#include "doomkeys.h"

typedef int (*lseek_t)(int fd, int offset, int whence);
#define lseek ((lseek_t)(0x00604891))

typedef int *(*errno_func_t)(void);
#define __errno_func ((errno_func_t)(0x00604ef9))

typedef int (*circbuf_overwrite_t)(void *circbuf, const void *data, int nbytes);
#define circbuf_overwrite ((circbuf_overwrite_t)(0x006541d1))

typedef int (*circbuf_write_t)(void *circbuf, const void *data, int nbytes);
#define circbuf_write ((circbuf_write_t)(0x00654145))

struct fb_paninfo_s {
    uint8_t  reserved[24];
    uint32_t yoffset;
};

static int    fb_fd = -1;
static void  *fb_mem = 0;
static int    fb_width = 466;
static int    fb_height = 466;
static int    fb_bpp = 32;
static int    fb_stride = 1864;
static struct fb_planeinfo_s fb_pinfo;

static void *gpu_fb_struct = (void *)0x4040c8a0;

typedef void (*goldfish_gpu_commit_t)(void *priv, void *fb_data);
#define goldfish_gpu_commit ((goldfish_gpu_commit_t)(0x0062BBD9))

static void *paninfo_circbuf = (void *)0x4040c630;

#define LV_INDEV_TYPE_POINTER  1

typedef void *(*lv_indev_get_next_t)(void *indev);
typedef int   (*lv_indev_get_type_t)(void *indev);

#define lv_indev_get_next  ((lv_indev_get_next_t)(0x00926FBD))
#define lv_indev_get_type  ((lv_indev_get_type_t)(0x00927029))

typedef void* (*lv_display_get_default_t)(void);
typedef void  (*lv_display_enable_invalidation_t)(void *disp, int en);
typedef void* (*lv_display_get_refr_timer_t)(void *disp);
typedef void* (*lv_display_get_screen_active_t)(void *disp);
typedef void  (*lv_obj_clean_t)(void *obj);
typedef void  (*lv_timer_pause_t)(void *timer);
typedef void  (*lv_timer_resume_t)(void *timer);

#define lv_display_get_default          ((lv_display_get_default_t)(0x0091F3B5))
#define lv_display_enable_invalidation  ((lv_display_enable_invalidation_t)(0x009200E1))
#define lv_display_get_refr_timer       ((lv_display_get_refr_timer_t)(0x00920189))
#define lv_display_get_screen_active    ((lv_display_get_screen_active_t)(0x0091F529))
#define lv_obj_clean                    ((lv_obj_clean_t)(0x0091D4E9))
#define lv_timer_pause                  ((lv_timer_pause_t)(0x009333F5))
#define lv_timer_resume                 ((lv_timer_resume_t)(0x00933441))

static void *lvgl_disp = 0;
static void *lvgl_refr_timer = 0;

struct lv_indev_data_t {
    int point_x;
    int point_y;
    unsigned int key;
    unsigned int btn_id;
    short enc_diff;
    unsigned char state;
    unsigned char continue_reading;
};

typedef void (*touchpad_wheel_read_t)(void *indev, struct lv_indev_data_t *data);
#define touchpad_wheel_read  ((touchpad_wheel_read_t)(0x01027A95))

static void *touch_indev = 0;

static void *vela_find_pointer_indev(void) {
    void *indev = lv_indev_get_next(0);
    while (indev) {
        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_POINTER)
            return indev;
        indev = lv_indev_get_next(indev);
    }
    return 0;
}

static void disable_lvgl_rendering(void) {
    lvgl_disp = lv_display_get_default();
    if (!lvgl_disp) return;

    lv_display_enable_invalidation(lvgl_disp, 0);
    lvgl_refr_timer = lv_display_get_refr_timer(lvgl_disp);
    if (lvgl_refr_timer) lv_timer_pause(lvgl_refr_timer);

    void *scr = lv_display_get_screen_active(lvgl_disp);
    if (scr) lv_obj_clean(scr);

    printf("[DOOM] LVGL rendering disabled\n");
}

#define KEYQUEUE_SIZE 32
static uint16_t key_queue[KEYQUEUE_SIZE];
static int key_queue_read = 0;
static int key_queue_write = 0;

static void add_key(int pressed, unsigned char key) {
    key_queue[key_queue_write] = (uint16_t)((pressed << 8) | key);
    key_queue_write = (key_queue_write + 1) % KEYQUEUE_SIZE;
}

typedef uint32_t (*clock_systime_ticks_t)(void);
#define clock_systime_ticks ((clock_systime_ticks_t)(0x0061c9f9))

static uint32_t basetime = 0;

static unsigned char touch_to_key(int x, int y) {
    if (y > 396) {
        if (x < 116) return KEY_UPARROW;
        if (x < 232) return KEY_FIRE;
        if (x < 348) return KEY_USE;
        return KEY_DOWNARROW;
    }
    if (y < 70) {
        if (x < 233) return KEY_ENTER;
        return KEY_ESCAPE;
    }
    if (x < 50)  return KEY_LEFTARROW;
    if (x > 416) return KEY_RIGHTARROW;

    return KEY_FIRE;
}

static unsigned char last_key = 0;
static unsigned char last_pressed = 0;

static void handle_touch(void) {
    if (!touch_indev) return;

    struct lv_indev_data_t td;
    memset(&td, 0, sizeof(td));
    touchpad_wheel_read(touch_indev, &td);

    if (td.state == 1) {
        unsigned char key = touch_to_key(td.point_x, td.point_y);
        if (key != last_key && last_key != 0) {
            add_key(0, last_key);
        }
        add_key(1, key);
        last_key = key;
        last_pressed = 1;
    } else {
        if (last_pressed && last_key != 0) {
            add_key(0, last_key);
            last_key = 0;
        }
        last_pressed = 0;
    }
}

void DG_Init(void) {
    printf("[DOOM] DG_Init: opening /dev/fb0...\n");

    fb_fd = open("/dev/fb0", 3 | 0x40);
    if (fb_fd < 0) {
        printf("[DOOM] FATAL: cannot open /dev/fb0, fd=%d\n", fb_fd);
        _exit(1);
    }

    memset(&fb_pinfo, 0, sizeof(fb_pinfo));
    int ret = ioctl(fb_fd, FBIOGET_PLANEINFO, &fb_pinfo);
    if (ret >= 0 && fb_pinfo.fbmem) {
        fb_mem = fb_pinfo.fbmem;
        fb_stride = fb_pinfo.stride > 0 ? fb_pinfo.stride : 1864;
        fb_bpp = fb_pinfo.bpp > 0 ? fb_pinfo.bpp : 32;
    } else {
        fb_mem = (void *)0x4048e058;
    }

    struct fb_videoinfo_s vinfo;
    memset(&vinfo, 0, sizeof(vinfo));
    ioctl(fb_fd, FBIOGET_VIDEOINFO, &vinfo);
    if (vinfo.xres > 0) fb_width = vinfo.xres;
    if (vinfo.yres > 0) fb_height = vinfo.yres;

    printf("[DOOM] FB: %dx%d, bpp=%d, stride=%d, mem=%p, virt=%dx%d\n",
           fb_width, fb_height, fb_bpp, fb_stride, fb_mem,
           fb_pinfo.xres_virtual, fb_pinfo.yres_virtual);

    memset(fb_mem, 0, fb_stride * fb_height);

    disable_lvgl_rendering();

    touch_indev = vela_find_pointer_indev();
    printf("[DOOM] Touch: LVGL indev=%p\n", touch_indev);

    printf("[DOOM] Time system ready\n");

    printf("[DOOM] Stopping miwear watchface...\n");
    system("am stop com.xiaomi.miwear.watchface");

    usleep(200000);
    printf("[DOOM] Watchface stopped\n");

    printf("[DOOM] DG_Init complete\n");
}

void DG_DrawFrame(void) {
    static int frame_count = 0;

    static int x_lut[466];
    static int y_lut[291];
    static int lut_init = 0;

    if (!lut_init) {
        int i;
        for (i = 0; i < 466; i++) x_lut[i] = (i * 320) / 466;
        for (i = 0; i < 291; i++) y_lut[i] = (i * 200) / 291;
        lut_init = 1;
    }

    handle_touch();

    if (!fb_mem || !DG_ScreenBuffer) return;

    uint32_t *fb = (uint32_t *)fb_mem;
    int y;

    for (y = 0; y < 291; y++) {
        uint32_t *src = DG_ScreenBuffer + y_lut[y] * 320;
        uint32_t *dst = fb + (y + 87) * 466;
        int x;
        for (x = 0; x < 466; x++) {
            dst[x] = src[x_lut[x]] | 0xFF000000;
        }
    }

    frame_count++;

    goldfish_gpu_commit(gpu_fb_struct, fb_mem);

    if (frame_count <= 3) {
        printf("[DOOM] Frame %d rendered and committed\n", frame_count);
    }
}

void DG_SleepMs(uint32_t ms) {
    usleep(ms * 1000);
}

uint32_t DG_GetTicksMs(void) {
    static uint32_t base_ticks = 0;
    uint32_t ticks = clock_systime_ticks();

    if (base_ticks == 0) {
        base_ticks = ticks;
    }

    return (ticks - base_ticks) / 10;
}

int DG_GetKey(int *pressed, unsigned char *doomKey) {
    if (key_queue_read == key_queue_write) return 0;

    uint16_t val = key_queue[key_queue_read];
    key_queue_read = (key_queue_read + 1) % KEYQUEUE_SIZE;

    *pressed = (val >> 8) & 1;
    *doomKey = val & 0xFF;
    return 1;
}

void DG_SetWindowTitle(const char *title) {
    syslog(6, "[DOOM] %s", title);
}

int main(void) {
    syslog(6, "[DOOM] VelaDoom starting...");

    static char arg0[] = "doom";
    static char arg1[] = "-iwad";
    static char arg2[] = "/data/freedoom1.wad";
    static char *argv[] = { arg0, arg1, arg2, 0 };

    doomgeneric_Create(3, argv);

    while (1) {
        doomgeneric_Tick();
    }

    return 0;
}
