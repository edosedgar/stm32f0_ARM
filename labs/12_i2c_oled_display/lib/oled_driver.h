#ifndef __OLED_DRIVER__
#define __OLED_DRIVER__

#define GMEM_WIDTH 128
#define GMEM_HEIGHT 64
#define GMEM_SIZE (GMEM_WIDTH * GMEM_WIDTH / 8)

/*
 * Simple strucutre to define fonts
 * Refer to font5x7.c for example
 */
typedef struct {
    const uint8_t *font;
    uint8_t width;
    uint8_t height;
    uint8_t (*get_pix)(uint8_t, uint8_t, uint8_t);
} font_desc_t;

enum color_t {
    clBlack = 0x00,
    clWhite = 0xFF
};

/*
 * initialize display
 */
void oled_config(void);

/*
 * clear graphic memory
 */
void oled_clr(enum color_t);

/*
 * flush graphic mem to display
 */
void oled_update(void);

/*
 * Put a pixel with a specified color
 */
void oled_set_pix(uint8_t x, uint8_t y, enum color_t color);

/*
 * Put a character
 */
void oled_putc(char ch);

/*
 * Set current cursor
 */
void oled_set_cursor(uint8_t x, uint8_t y);

/*
 * Print picture with a given threshold for b/w
 */
void oled_pic(const uint8_t *im, uint8_t thrsh);

/*
 * Print picture using dithering algorithm for
 * adaptive b/w
 */
void oled_pic_dithering(const uint8_t *im);

#endif
