#ifndef __OLED_FONT_H
#define __OLED_FONT_H

#include <stdint.h>

/*中文字符字节宽度*/
#define OLED_CHN_CHAR_WIDTH (2) // UTF-8编码格式给3，GB2312编码格式给2

/*字模基本单元*/
typedef struct
{
    char Index[OLED_CHN_CHAR_WIDTH + 1]; // 汉字索引
    uint8_t Data[60];                    // 字模数据
} ChineseCell20x20_t;
typedef struct
{
    char Index[OLED_CHN_CHAR_WIDTH + 1]; // 汉字索引
    uint8_t Data[32];                    // 字模数据
} ChineseCell16x16_t;
typedef struct
{
    char Index[OLED_CHN_CHAR_WIDTH + 1]; // 汉字索引
    uint8_t Data[24];                    // 字模数据，12*12的汉字需要24字节
} ChineseCell12x12_t;

typedef struct
{
    char Index[OLED_CHN_CHAR_WIDTH + 1]; // 汉字索引
    uint8_t Data[8];                     // 字模数据，8*8的汉字需要24字节
} ChineseCell8x8_t;

/*ASCII字模数据声明*/
extern const uint8_t OLED_F10x20[][30];
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F7x12[][14];
extern const uint8_t OLED_F6x8[][6];

/*汉字字模数据声明*/
extern const ChineseCell20x20_t OLED_CF20x20[];
extern const ChineseCell16x16_t OLED_CF16x16[];
extern const ChineseCell12x12_t OLED_CF12x12[]; // 声明12x12字模数组
extern const ChineseCell8x8_t OLED_CF8x8[];     // 声明12x12字模数组
/*图像数据声明*/
extern const uint8_t Arrow[];
extern const uint8_t UnKnown[];

extern const uint8_t Image_setings[];
extern const uint8_t Image_QRcode[];
extern const uint8_t Image_calc[];
extern const uint8_t Image_night[];
extern const uint8_t Image_ADDV[];
extern const uint8_t Image_ADDV2[];
extern const uint8_t Image_BIPAN[];
extern const uint8_t Image_GAME[];
extern const uint8_t Image_MUSIC[];
extern const uint8_t Image_VIDEO[];
extern const uint8_t Image_humiture[];
extern const uint8_t Image_flashlight[];
extern const uint8_t Image_second[];

extern const uint8_t OLED_UI_LOGO[];
extern const uint8_t OLED_UI_LOGOTEXT[];
extern const uint8_t OLED_UI_LOGOTEXT64[];
extern const uint8_t OLED_UI_LOGOGithub[];
extern const uint8_t OLED_UI_SettingsLogo[];

extern const uint8_t t1[];
extern const uint8_t t2[];
extern const uint8_t t3[];
extern const uint8_t t4[];
extern const uint8_t t5[];
extern const uint8_t t6[];
extern const uint8_t t7[];
extern const uint8_t t8[];
extern const uint8_t t9[];
extern const uint8_t t0[];

extern const uint8_t i1[];
extern const uint8_t i2[];
extern const uint8_t i3[];
extern const uint8_t i4[];
extern const uint8_t i5[];
extern const uint8_t i6[];
extern const uint8_t i7[];
extern const uint8_t i8[];
extern const uint8_t i9[];
extern const uint8_t i0[];
/*按照上面的格式，在这个位置加入新的图像数据声明*/
//...

#endif
