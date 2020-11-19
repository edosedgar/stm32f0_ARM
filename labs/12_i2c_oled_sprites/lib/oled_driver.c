#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_i2c.h"

#include "oled_driver.h"
#include <string.h>

static uint8_t gmem[GMEM_SIZE] = {0};
static uint8_t curX = 0;
static uint8_t curY = 0;

extern font_desc_t font_desc;

static void oled_hw_config(void)
{
    /*
     * Clock on the I2C port and configure it
     */
    // SCL - GPIOB6
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_6,
                       LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_6,
                             LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_6, LL_GPIO_AF_1);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_6,
                        LL_GPIO_SPEED_FREQ_HIGH);

    // SDA - GPIOB7
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7,
                       LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_7,
                             LL_GPIO_OUTPUT_OPENDRAIN);
    LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_7, LL_GPIO_AF_1);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_7,
                        LL_GPIO_SPEED_FREQ_HIGH);
    /*
     * Clock on the I2C peripheral and set it up
     */
    LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_SYSCLK);
    LL_I2C_Disable(I2C1);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    LL_I2C_DisableAnalogFilter(I2C1);
    LL_I2C_SetDigitalFilter(I2C1, 1);
    /*
     * Set I2C speed to 400 kHz, for further details refer
     * to lecture
     */
    LL_I2C_SetTiming(I2C1, 0x50330309);
    LL_I2C_DisableClockStretching(I2C1);
    LL_I2C_SetMasterAddressingMode(I2C1, LL_I2C_ADDRESSING_MODE_7BIT);
    LL_I2C_SetMode(I2C1, LL_I2C_MODE_I2C);
    LL_I2C_Enable(I2C1);

    return;
}

static uint8_t oled_cmd_send(uint8_t byte)
{
    /*
     * Initiate transmission
     * Display address = 0x78
     */
    LL_I2C_HandleTransfer(I2C1, 0x78, LL_I2C_ADDRSLAVE_7BIT,
                          2, LL_I2C_MODE_AUTOEND,
                          LL_I2C_GENERATE_START_WRITE);
    /*
     * Send Control byte (Co = 0, D/C# = 0)
     */
    while (!LL_I2C_IsActiveFlag_TXIS(I2C1));
    LL_I2C_TransmitData8(I2C1, 0x00);
    /*
     * Send cmd
     */
    while (!LL_I2C_IsActiveFlag_TXIS(I2C1));
    LL_I2C_TransmitData8(I2C1, byte);
    /*
     * Check for end of transmission
     */
    while (LL_I2C_IsActiveFlag_TC(I2C1));
    return 0;
}

static uint8_t oled_data_send(uint8_t *byte, uint8_t size)
{
    uint8_t i;

    /*
     * Initiate transmission
     * Display address = 0x78
     */
    LL_I2C_HandleTransfer(I2C1, 0x78, LL_I2C_ADDRSLAVE_7BIT,
                          size + 1, LL_I2C_MODE_AUTOEND,
                          LL_I2C_GENERATE_START_WRITE);
    /*
     * Send Control byte (Co = 0, D/C# = 1)
     */
    while (!LL_I2C_IsActiveFlag_TXIS(I2C1));
    LL_I2C_TransmitData8(I2C1, 0x40);
    /*
     * Send data
     */
    for (i = 0; i < size; i++) {
        while (!LL_I2C_IsActiveFlag_TXIS(I2C1));
        LL_I2C_TransmitData8(I2C1, byte[i]);
    }
    /*
     * Check for end of transmission
     */
    while (LL_I2C_IsActiveFlag_TC(I2C1));
    return 0;
}

void oled_clr(enum color_t color)
{
    memset(gmem, color, GMEM_SIZE);
    return;
}

void oled_update(void)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        // Set page start address (number of vertical byte)
        oled_cmd_send(0xB0 + i);
        // Set lower column number
        oled_cmd_send(0x00);
        // Set higher column number
        oled_cmd_send(0x10);
        // Send 128 pixels
        oled_data_send(&gmem[GMEM_WIDTH * i], GMEM_WIDTH);
    }
}

void oled_config(void)
{
    uint32_t delay = 2000000;
    // Config hardware
    oled_hw_config();

    // Wait a bit
    while (delay--);

    // Set display OFF
    oled_cmd_send(0xAE);

    // Set addressing mode
    // Vertical addressing mode
    oled_cmd_send(0x20);
    oled_cmd_send(0x10);

    // Vertical flip: 0xC0 - on, 0xC8 - off
    oled_cmd_send(0xC8);

    // Set start line address 0-63
    oled_cmd_send(0x40);

    // Set contrast level: 0-255
    oled_cmd_send(0x81);
    oled_cmd_send(0xFF);

    // Horizontal flip: 0xA1 - on, 0xA0 - off
    oled_cmd_send(0xA1);

    // Normal colo - 0xA6, Inverse - 0xA7
    oled_cmd_send(0xA6);

    // Number of active lines: 16 - 64
    oled_cmd_send(0xA8);
    oled_cmd_send(0x3F);

    // Render GRAM: 0xA4 - render, 0xA5 - black screen
    oled_cmd_send(0xA4);

    // Set display offset: 0-63
    oled_cmd_send(0xD3);
    oled_cmd_send(0x00);

    // Set display refresh rate:
    // 7-4 bits set osc freq, 0-3 sets resfresh ratio
    oled_cmd_send(0xD5);
    oled_cmd_send(0xF0);

    // Set flipping config
    oled_cmd_send(0xDA);
    oled_cmd_send(0x12);

    // Enable charge pump
    oled_cmd_send(0x8D);
    oled_cmd_send(0x14);

    // Turn on display
    oled_cmd_send(0xAF);

    oled_clr(clBlack);
    oled_update();
    return;
}

void oled_set_pix(uint8_t x, uint8_t y, enum color_t color)
{
    if (x >= GMEM_WIDTH || y >= GMEM_HEIGHT)
        return;

    if (color != clBlack)
        gmem[GMEM_WIDTH * (y / 8) + x] |= 1 << (y % 8);
    else
        gmem[GMEM_WIDTH * (y / 8) + x] &= ~(1 << (y % 8));
    return;
}

void oled_set_cursor(uint8_t x, uint8_t y)
{
    curX = x;
    curY = y;
    return;
}

void oled_putc(char ch)
{
    uint8_t i, j;
    uint8_t color;

    /*
     * NOTE: historically \r and \n are used together
     * in many instances as \r is used to move the carriage back
     * to the left side of terminal, \n moves the carriage down
     * Thereby, we must implement both of them
     */
    if (ch == '\n') {
        curY++;
        return;
    }

    if (ch == '\r') {
        curX = 0;
        return;
    }

    for (j = 0; j < font_desc.height; j++) {
        for (i = 0; i < font_desc.width; i++) {
            color = font_desc.get_pix(ch, i, j);
            oled_set_pix(curX * (font_desc.width + 1) + i,
                         curY * (font_desc.height + 2) + j, color);
        }
    }
    curX++;
    return;
}

/*
 * Additional functionality
 */

void oled_pic(const uint8_t *im, uint8_t thrsh)
{
    uint8_t i, j;

    /*
     * Render picture from im array:
     * if value is above thrsh then it is white
     * otherwise it is black
     */
    for (j = 0; j < GMEM_HEIGHT; j++) {
        for (i = 0; i < GMEM_WIDTH; i++) {
            oled_set_pix(i, j, im[j * GMEM_WIDTH + i] > thrsh);
        }
    }
    return;
}

/*
 * Set of helper functons to mitigate work
 * with image buffer
 */
static void set_bpix(uint8_t *buf, int16_t x, int16_t y, int16_t val)
{
    if ((x < 0 || x >= GMEM_WIDTH) ||
        (y < 0 || y >= GMEM_HEIGHT))
        return;
    buf[y * GMEM_WIDTH + x] = val > 0xFF ? 0xFF : val < 0 ? 0 : val;
    return;
}

static int16_t get_bpix(uint8_t *buf, int16_t x, int16_t y)
{
    if ((x < 0 || x >= GMEM_WIDTH) ||
        (y < 0 || y >= GMEM_HEIGHT))
        return 0;
    return buf[y * GMEM_WIDTH + x];
}

void oled_pic_dithering(const uint8_t *im)
{
    /*
     * Store last two lines as dithering
     * propagates error like that:
     *          *    p  7/16
     *        3/16 5/16 1/16
     */
    static uint8_t buffer[GMEM_WIDTH * 2] = {0};
    int16_t i, j;
    uint16_t oldpix, newpix;
    int16_t quant_err;

    /*
     * Init buffer with first two lines
     * of image
     */
    memcpy(buffer, im, GMEM_WIDTH * 2);

    /*
     * Start the Floyd–Steinberg dithering
     * algorithm
     * The array buffer is required to store
     * recomputed values of neighbouring pixels
     * after applying the algorithm
     * It is impossible to store the whole array
     * as we have limited memory
     */
    for (j = 0; j < GMEM_HEIGHT; j++) {
        for (i = 0; i < GMEM_WIDTH; i++) {
            oldpix = get_bpix(buffer, i, j % 2);
            newpix = (oldpix >= 128) ? 255 : 0;
            oled_set_pix(i, j, newpix);
            quant_err = oldpix - newpix;
            set_bpix(buffer, i + 1, j%2,
                     get_bpix(buffer, i + 1, j%2) + quant_err * 7 / 16);
            set_bpix(buffer, i - 1, (j + 1) % 2,
                     get_bpix(buffer, i - 1, (j + 1)%2) + quant_err * 3 / 16);
            set_bpix(buffer, i, (j + 1) % 2,
                     get_bpix(buffer, i, (j + 1)%2) + quant_err * 5 / 16);
            set_bpix(buffer, i + 1, (j + 1) % 2,
                     get_bpix(buffer, i + 1, (j + 1)%2) + quant_err * 1 / 16);
        }
        /*
         * Shift second line to the first one
         * and fetch next line from image array
         */
        if (j >= 1 && j < (GMEM_HEIGHT - 1)) {
            memcpy(buffer, buffer + GMEM_WIDTH, GMEM_WIDTH);
            memcpy(buffer + GMEM_WIDTH, im + GMEM_WIDTH * (j + 1), GMEM_WIDTH);
        }
    }
}

/* 
 * Sprite drawing
 */

void draw_sprite(Sprite sprite, uint32_t costume, int16_t x, int16_t y, uint32_t options_mask)
{
    int32_t i, j;
    uint8_t *black_costume_data, *white_costume_data;
    uint32_t width, height;
    int16_t sprite_pos, gmem_pos;
    uint8_t byte_shift;
    
    /* Check costume */
    if (costume >= sprite.n_costumes) return;
    black_costume_data = sprite.black_costumes_bank[costume];
    white_costume_data = sprite.white_costumes_bank[costume];
    
    /* Rewrite parameters */
    height = sprite.height;
    width = sprite.width;
    
    /* Return if out of frame */
    if (y <= -(int16_t)(height)*8 || y >= GMEM_HEIGHT || x <= -(int16_t)(width)) return;
    
    /* Return if both WHITE_NO and BLACK_NO active */
    if((options_mask & DRAW_BLACK_NO) && (options_mask & DRAW_WHITE_NO)) return;
    
    /* Identify drawing boundaries */
    int32_t i_start = (y < 0)? -(int32_t)(y/8) : 0;
    int32_t i_stop = (y + height*8 - 8 < GMEM_HEIGHT)? (int32_t)height : (int32_t)(height-(y + height*8 - GMEM_HEIGHT)/8);
    int32_t j_start = (x < 0)? -(int32_t)(x) : 0;
    int32_t j_stop = (x + width < GMEM_WIDTH)? (int32_t)width : (int32_t)(-x + GMEM_WIDTH);
    
    /* Change negative y, so y/8 gives -(|y|/8) */
    if (y < 0) {
        byte_shift = 8 + (y % 8);
        byte_shift = (byte_shift == 8)? 0 : byte_shift;
        y -= 7;
    } else {
        byte_shift = y % 8;
    }
    
    for(i = i_start; i < i_stop; i++) {
        sprite_pos = (options_mask & DRAW_FLIP_SPRITE)? (i*width - j_start + width - 1) : (i*width + j_start);
        gmem_pos = ((y/8) + i) * GMEM_WIDTH + x + j_start; 
        if(gmem_pos >= GMEM_SIZE) continue;
        for(j = j_start; j < j_stop; j++) {
            
            /* Draw black byte */            
            if (!(options_mask & DRAW_BLACK_NO)) {
                if(options_mask & DRAW_BLACK_INVERT) {
                    if(gmem_pos >= 0)
                        gmem[gmem_pos] ^= (black_costume_data[sprite_pos] << byte_shift);
                    if (gmem_pos < GMEM_SIZE-GMEM_WIDTH)            
                        gmem[gmem_pos+GMEM_WIDTH] ^= (black_costume_data[sprite_pos] >> (8-byte_shift));
                } else {
                    if(options_mask & DRAW_BLACK_WHITE) {
                        if(gmem_pos >= 0)
                            gmem[gmem_pos] |= (black_costume_data[sprite_pos] << byte_shift);
                        if (gmem_pos < GMEM_SIZE-GMEM_WIDTH)            
                            gmem[gmem_pos+GMEM_WIDTH] |= (black_costume_data[sprite_pos] >> (8-byte_shift));
                    } else {
                        if(gmem_pos >= 0)
                            gmem[gmem_pos] &= ~(black_costume_data[sprite_pos] << byte_shift);
                        if (gmem_pos < GMEM_SIZE-GMEM_WIDTH)            
                            gmem[gmem_pos+GMEM_WIDTH] &= ~(black_costume_data[sprite_pos] >> (8-byte_shift));
                    }
                }
            }
            
            /* Draw white byte */
            if (!(options_mask & DRAW_WHITE_NO)) {
                if(options_mask & DRAW_WHITE_INVERT) {
                    if(gmem_pos >= 0)
                        gmem[gmem_pos] ^= (white_costume_data[sprite_pos] << byte_shift);
                    if (gmem_pos < GMEM_SIZE-GMEM_WIDTH)            
                        gmem[gmem_pos+GMEM_WIDTH] ^= (white_costume_data[sprite_pos] >> (8-byte_shift));
                } else {
                    if(options_mask & DRAW_WHITE_BLACK) {
                        if(gmem_pos >= 0)
                            gmem[gmem_pos] &= ~(white_costume_data[sprite_pos] << byte_shift);
                        if (gmem_pos < GMEM_SIZE-GMEM_WIDTH)            
                            gmem[gmem_pos+GMEM_WIDTH] &= ~(white_costume_data[sprite_pos] >> (8-byte_shift));
                    } else {
                        if(gmem_pos >= 0)
                            gmem[gmem_pos] |= (white_costume_data[sprite_pos] << byte_shift);
                        if (gmem_pos < GMEM_SIZE-GMEM_WIDTH)            
                            gmem[gmem_pos+GMEM_WIDTH] |= (white_costume_data[sprite_pos] >> (8-byte_shift));
                    }
                }
            }
            
            gmem_pos++;
            if(options_mask & DRAW_FLIP_SPRITE) 
                sprite_pos--;
            else 
                sprite_pos++;
                
        } /* j */
    } /* i */
    
}

/* 
 * Checking if sprite collides with
 * pixels already drawn in memory
 */
uint32_t check_sprite_collision(Sprite sprite, uint32_t costume, int16_t x, int16_t y, uint32_t options_mask)
{
    //uint8_t tmp_byte;
    int32_t i, j;
    uint8_t *black_costume_data, *white_costume_data;
    uint32_t width, height;
    int16_t sprite_pos, gmem_pos;
    uint8_t byte_shift;
    
    /* Check costume */
    if (costume >= sprite.n_costumes) return 0;
    black_costume_data = sprite.black_costumes_bank[costume];
    white_costume_data = sprite.white_costumes_bank[costume];
    
    /* Rewrite parameters */
    height = sprite.height;
    width = sprite.width;
    
    /* No collision if out of frame */
    if (y <= -(int16_t)(height)*8 || y >= GMEM_HEIGHT || x <= -(int16_t)(width)) return 0;
    
    /* Return if no active arguments */
    if((options_mask & (CHECK_BLACK_BLACK || CHECK_BLACK_WHITE || CHECK_WHITE_BLACK || CHECK_WHITE_WHITE)) == 0) return 0;
    
    /* Identify checking boundaries */
    int32_t i_start = (y < 0)? -(int32_t)(y/8) : 0;
    int32_t i_stop = (y + height*8 - 8 < GMEM_HEIGHT)? (int32_t)height : (int32_t)(height-(y + height*8 - GMEM_HEIGHT)/8);
    int32_t j_start = (x < 0)? -(int32_t)(x) : 0;
    int32_t j_stop = (x + width < GMEM_WIDTH)? (int32_t)width : (int32_t)(-x + GMEM_WIDTH);
    
    /* Change negative y, so y/8 gives -(|y|/8) */
    if (y < 0) {
        byte_shift = 8 + (y % 8);
        byte_shift = (byte_shift == 8)? 0 : byte_shift;
        y -= 7;
    } else {
        byte_shift = y % 8;
    }
    
    for(i = i_start; i < i_stop; i++) {
        sprite_pos = (options_mask & CHECK_FLIP_SPRITE)? (i*width - j_start + width - 1) : (i*width + j_start);
        gmem_pos = ((y/8) + i) * GMEM_WIDTH + x + j_start; 
        if(gmem_pos >= GMEM_SIZE) continue;
        for(j = j_start; j < j_stop; j++) {
            
            /* Check black byte */            
            if (options_mask & CHECK_BLACK_BLACK) {
                if(gmem_pos >= 0)
                    if (~gmem[gmem_pos] & (black_costume_data[sprite_pos] << byte_shift)) return 1;
                if (gmem_pos < GMEM_SIZE-GMEM_WIDTH)            
                    if (~gmem[gmem_pos+GMEM_WIDTH] & (black_costume_data[sprite_pos] >> (8-byte_shift))) return 1;
            }
            if(options_mask & CHECK_BLACK_WHITE) {
                if(gmem_pos >= 0)
                    if (gmem[gmem_pos] & (black_costume_data[sprite_pos] << byte_shift)) return 1;
                if (gmem_pos < GMEM_SIZE-GMEM_WIDTH)            
                    if (gmem[gmem_pos+GMEM_WIDTH] & (black_costume_data[sprite_pos] >> (8-byte_shift))) return 1;
            }
            
            /* Check white byte */
            if (options_mask & CHECK_WHITE_WHITE) {
                if(gmem_pos >= 0)
                    if (gmem[gmem_pos] & (white_costume_data[sprite_pos] << byte_shift)) return 1;
                if (gmem_pos < GMEM_SIZE-GMEM_WIDTH)            
                    if (gmem[gmem_pos+GMEM_WIDTH] & (white_costume_data[sprite_pos] >> (8-byte_shift))) return 1;
            }
            if(options_mask & CHECK_WHITE_BLACK) {
                if(gmem_pos >= 0)
                    if (~gmem[gmem_pos] & (white_costume_data[sprite_pos] << byte_shift)) return 1;
                if (gmem_pos < GMEM_SIZE-GMEM_WIDTH)            
                    if (~gmem[gmem_pos+GMEM_WIDTH] & (white_costume_data[sprite_pos] >> (8-byte_shift))) return 1;
            }
            
            gmem_pos++;
            if(options_mask & DRAW_FLIP_SPRITE) 
                sprite_pos--;
            else 
                sprite_pos++;
                
        } /* j */
    } /* i */
    return 0;
}
