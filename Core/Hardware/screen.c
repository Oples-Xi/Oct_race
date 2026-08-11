#include "screen.h"
#include "control.h"
#include "tjc_usart_hmi.h"

extern GoodsSlot GoodsTable[MAX_GOODS_TYPE];

/**
 * @brief 查颜色
 *
 * @param color
 * @return const char*
 */
const char *GetColorName(uint8_t color)
{
    switch (color)
    {
        case 0: return "红色";
        case 1: return "橙色";
        case 2: return "黄色";
        case 3: return "蓝色";
        case 4: return "绿色";
        case 5: return "黑色";
        case 6: return "白色";
        case 7: return "紫色";
        default: return "未知颜色";
    }
}

/**
 * @brief 查询形状
 *
 * @param shape
 * @return const char*
 */
const char *GetShapeName(uint8_t shape)
{
    switch (shape)
    {
        case 0: return "正方体";
        case 1: return "五棱柱";
        case 2: return "四棱锥";
        case 3: return "三棱锥";
        case 4: return "圆柱体";
        case 5: return "四棱柱";
        case 6: return "球";
        default: return "未知形状";
    }
}

/**
 * @brief 发送数据给屏幕
 *
 * @param slot
 */
void Display_Goods(uint8_t slot)
{
    char text[32];
    char objname[8];

    if (slot >= MAX_GOODS_TYPE)
    {
        return;
    }

    snprintf(text, sizeof(text), "%s%s", GetColorName(GoodsTable[slot].color), GetShapeName(GoodsTable[slot].shape));

    snprintf(objname, sizeof(objname), "t%u", slot);
    tjc_send_txt(objname, "txt", text);

    snprintf(objname, sizeof(objname), "n%u", slot);
    tjc_send_val(objname, "val", GoodsTable[slot].count - 1);
}
