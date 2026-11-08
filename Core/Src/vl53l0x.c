#include "vl53l0x.h"
#include "display.h"

#define ADDR 0x52 // 地址

static uint8_t stop_val;    /* 芯片内部校准值，每次启动测量都要写回 0x91(来自DeepSeek) */

// 写1字节
static void wr(uint8_t reg, uint8_t val)
{
    HAL_I2C_Mem_Write(&hi2c1, ADDR, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 50);
}

// 读1字节
static uint8_t rd(uint8_t reg)
{
    uint8_t val = 0;

    HAL_I2C_Mem_Read(&hi2c1, ADDR, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 50);
    return val;
}

/* -----------------------------------------------------------------------------
 * 官方调参表：{寄存器, 值}，照抄，不要删(来自DeepSeek)
 * -------------------------------------------------------------------------- */
static const uint8_t cfg[][2] = {
    {0xFF,0x01},{0x00,0x00},{0xFF,0x00},{0x09,0x00},
    {0x10,0x00},{0x11,0x00},{0x24,0x01},{0x25,0xFF},
    {0x75,0x00},{0xFF,0x01},{0x4E,0x2C},{0x48,0x00},
    {0x30,0x20},{0xFF,0x00},{0x30,0x09},{0x54,0x00},
    {0x31,0x04},{0x32,0x03},{0x40,0x83},{0x46,0x25},
    {0x60,0x00},{0x27,0x00},{0x50,0x06},{0x51,0x00},
    {0x52,0x96},{0x56,0x08},{0x57,0x30},{0x61,0x00},
    {0x62,0x00},{0x64,0x00},{0x65,0x00},{0x66,0xA0},
    {0xFF,0x01},{0x22,0x32},{0x47,0x14},{0x49,0xFF},
    {0x4A,0x00},{0xFF,0x00},{0x7A,0x0A},{0x7B,0x00},
    {0x78,0x21},{0xFF,0x01},{0x23,0x34},{0x42,0x00},
    {0x44,0xFF},{0x45,0x26},{0x46,0x05},{0x40,0x40},
    {0x0E,0x06},{0x20,0x1A},{0x43,0x40},{0xFF,0x00},
    {0x34,0x03},{0x35,0x44},{0xFF,0x01},{0x31,0x04},
    {0x4B,0x09},{0x4C,0x05},{0x4D,0x04},{0xFF,0x00},
    {0x44,0x00},{0x45,0x20},{0x47,0x08},{0x48,0x28},
    {0x67,0x00},{0x70,0x04},{0x71,0x01},{0x72,0xFE},
    {0x76,0x00},{0x77,0x00},{0xFF,0x01},{0x0D,0x01},
    {0xFF,0x00},{0x80,0x01},{0x01,0xF8},{0xFF,0x01},
    {0x8E,0x01},{0x00,0x01},{0xFF,0x00},{0x80,0x00},
};

/* -----------------------------------------------------------------------------
 * 等一次测量完成：0x13 的低 3 位变成非 0 就表示数据好了(来自DeepSeek)
 * -------------------------------------------------------------------------- */
static uint8_t wait_done(uint16_t ms)
{
    uint32_t t0 = HAL_GetTick();

    while ((rd(0x13) & 0x07) == 0)
    {
        if ((HAL_GetTick() - t0) > ms) return 0;
    }
    return 1;
}

/* 校准（VHV / 相位）：启动 -> 等完成 -> 清中断 -> 停(来自DeepSeek) */
static uint8_t calibrate(uint8_t start_value)
{
    wr(0x00, start_value);
    if (!wait_done(300)) return 0;

    wr(0x0B, 0x01);
    wr(0x00, 0x00);
    return 1;
}

/* -----------------------------------------------------------------------------
 * 初始化(来自DeepSeek)
 * -------------------------------------------------------------------------- */
uint8_t VL53L0X_Init(void)
{
    uint16_t i;

    HAL_Delay(10);
    if (rd(0xC0) != 0xEE) return 0;         /* 芯片 ID 不对，后面别写了 */

    /* --- 1. 基本模式 + 取 stop_val --- */
    wr(0x89, rd(0x89) | 0x01);              /* 2.8V I/O 模式 */
    wr(0x88, 0x00);                         /* I2C 标准模式 */

    wr(0x80, 0x01);                         /* 切私有页 */
    wr(0xFF, 0x01);
    wr(0x00, 0x00);
    stop_val = rd(0x91);                    /* 取出内部校准值 */
    wr(0x00, 0x01);
    wr(0xFF, 0x00);                         /* 切回公共页 */
    wr(0x80, 0x00);

    /* --- 2. 参考 SPAD（用出厂默认值，只补三个参数）--- */
    wr(0xFF, 0x01);
    wr(0x4F, 0x00);
    wr(0x4E, 0x2C);
    wr(0xFF, 0x00);
    wr(0xB6, 0xB4);

    /* --- 3. 官方调参表（80 条，一次写完）--- */
    for (i = 0; i < (sizeof(cfg) / 2); i++)
    {
        wr(cfg[i][0], cfg[i][1]);
    }

    /* --- 4. 中断源：每次测量完成（缺了它 0x13 永远不置位）--- */
    wr(0x0A, 0x04);
    wr(0x0B, 0x01);                         /* 清一次中断标志 */

    wr(0x01, 0xE8);                         /* 只保留必要的测量步骤 */

    /* --- 5. VHV 校准 + 相位校准 --- */
    wr(0x01, 0x01);
    if (!calibrate(0x41)) return 0;         /* 0x41 = 0x01|0x40 */

    wr(0x01, 0x02);
    if (!calibrate(0x01)) return 0;

    wr(0x01, 0xE8);                         /* 恢复正常测量 */
    return 1;
}

/* -----------------------------------------------------------------------------
 * 单次测距(来自DeepSeek)
 * -------------------------------------------------------------------------- */
uint16_t VL53L0X_ReadDistance_mm(void)
{
    uint16_t mm;
    uint8_t  st;

    /* 启动序列：固定动作，一行都不能少 */
    wr(0x80, 0x01);
    wr(0xFF, 0x01);
    wr(0x00, 0x00);
    wr(0x91, stop_val);                     /* 写回校准值 */
    wr(0x00, 0x01);
    wr(0xFF, 0x00);
    wr(0x80, 0x00);
    wr(0x00, 0x01);                         /* 开始测量 */

    if (!wait_done(200)) return 0xFFFF;     /* 约 30~40ms */

    mm = ((uint16_t)rd(0x1E) << 8) | rd(0x1F);  /* 高字节在前 */
    st = rd(0x14);
    wr(0x0B, 0x01);                         /* 清中断 */

    if (((st & 0x78) >> 3) != 11) return 0xFFFF;   /* 状态 11 才有效 */
    if (mm >= 8180) return 0xFFFF;                 /* 8190 = 没打到东西 */

    return mm;
}

// 测三次取中值
uint16_t VL53L0X_MedianDistance(void)
{
    uint16_t d1 = VL53L0X_ReadDistance_mm();
    uint16_t d2 = VL53L0X_ReadDistance_mm();
    uint16_t d3 = VL53L0X_ReadDistance_mm();
    
    d2 = d1>d2?(d2>d3?d2:(d1>d3?d3:d1)):(d1>d3?d1:(d2>d3?d3:d2));

    return d2;
}

void Vl53l0x_Init(void){
    uint8_t ok = 0;

    for (uint8_t t = 0; t < 3; t++){
        if (VL53L0X_Init()){
        ok = 1;
        break;
        }
        HAL_Delay(200);
    }

    if (!ok){
        char err[] = "SENSOR ERROR";
        Display_Error(err);
        while (1){}
    }
}