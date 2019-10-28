#ifndef __OLED_DRIVER__
#define __OLED_DRIVER__

#define GMEM_WIDTH 128
#define GMEM_HEIGHT 64
#define GMEM_SIZE (GMEM_WIDTH * GMEM_HEIGHT / 8)

#define DRAW_BLACK_NO 0x00000001
#define DRAW_WHITE_NO 0x00000002
#define DRAW_BLACK_INVERT 0x00000004
#define DRAW_WHITE_INVERT 0x00000008
#define DRAW_BLACK_WHITE 0x00000010
#define DRAW_WHITE_BLACK 0x00000020
#define DRAW_FLIP_SPRITE 0x00000040

#define CHECK_BLACK_BLACK 0x00000001
#define CHECK_WHITE_WHITE 0x00000002
#define CHECK_BLACK_WHITE 0x00000004
#define CHECK_WHITE_BLACK 0x00000008
#define CHECK_FLIP_SPRITE 0x00000040

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

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t **black_costumes_bank;
    uint8_t **white_costumes_bank;
    uint32_t n_costumes;
} Sprite;

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

/*
 * Draw sprite at specified coordinates
 * with proper costume and its color
 */
void draw_sprite(Sprite sprite, uint32_t costume, int16_t x, int16_t y, uint32_t options_mask);

/*
 * Test if sprite costume collides with
 * already drawn pixels in picture
 */
uint32_t check_sprite_collision(Sprite sprite, uint32_t costume, int16_t x, int16_t y, uint32_t options_mask);

#endif
