#include "command.h"

// 指令的最小长度
#define COMMAND_MIN_LENGTH 4
// 指令的最大长度（防止越界拷贝）
#define COMMAND_MAX_LENGTH 50
// 循环缓冲区大小
#define BUFFER_SIZE 128

// 循环缓冲区
uint8_t buffer[BUFFER_SIZE];
// 循环缓冲区读索引
uint8_t readIndex = 0;
// 循环缓冲区写索引
uint8_t writeIndex = 0;
// 未处理数据长度
uint8_t dataLength = 0;

/**
 * @brief 增加读索引
 * @param length 要增加的长度
 */
void Command_AddReadIndex(uint8_t length)
{
    readIndex = (readIndex + length) % BUFFER_SIZE;
    dataLength -= length;
}

/**
 * @brief 读取第i位数据 超过缓存区长度自动循环
 * @param i 要读取的数据索引
 */
uint8_t Command_Read(uint8_t i)
{
    uint8_t index = i % BUFFER_SIZE;
    return buffer[index];
}

/**
 * @brief 计算未处理的数据长度
 * @return 未处理的数据长度
 */
uint8_t Command_GetLength(void)
{
    return dataLength;
}

/**
 * @brief 计算缓冲区剩余空间
 * @return 剩余空间
 */
uint8_t Command_GetRemain(void)
{
    return BUFFER_SIZE - dataLength;
}

/**
 * @brief 向缓冲区写入数据
 * @param data 要写入的数据指针
 * @param length 要写入的数据长度
 * @return 写入的数据长度
 */
uint8_t Command_Write(uint8_t *data, uint8_t length)
{
    uint8_t i;

    if (length == 0 || Command_GetRemain() < length)
    {
        return 0;
    }

    for (i = 0; i < length; i++)
    {
        buffer[writeIndex] = data[i];
        writeIndex = (writeIndex + 1) % BUFFER_SIZE;
    }
    dataLength += length;

    return length;
}

/**
 * @brief 尝试获取一条指令
 * @param command 指令存放指针
 * @return 获取的指令长度
 * @retval 0 没有获取到指令
 */
uint8_t Command_GetCommand(uint8_t *command)
{
    // 寻找完整指令
    while (1)
    {
        // 如果缓冲区长度小于COMMAND_MIN_LENGTH 则不可能有完整的指令
        if (Command_GetLength() < COMMAND_MIN_LENGTH)
        {
            return 0;
        }

        // 如果不是包头 则跳过 重新开始寻找
        if (Command_Read(readIndex) != 0xAA)
        {
            Command_AddReadIndex(1);
            continue;
        }

        // 读取指令长度，非法长度直接跳过
        uint8_t length = Command_Read(readIndex + 1);
        if (length < COMMAND_MIN_LENGTH || length > COMMAND_MAX_LENGTH)
        {
            Command_AddReadIndex(1);
            continue;
        }

        // 如果缓冲区长度小于指令长度 则不可能有完整的指令
        if (Command_GetLength() < length)
        {
            return 0;
        }

        // 如果校验和不正确 则跳过 重新开始寻找
        uint8_t sum = 0;
        for (uint8_t i = 0; i < length - 1; i++)
        {
            sum += Command_Read(readIndex + i);
        }
        if (sum != Command_Read(readIndex + length - 1))
        {
            Command_AddReadIndex(1);
            continue;
        }

        // 如果找到完整指令 则将指令写入command 返回指令长度
        for (uint8_t i = 0; i < length; i++)
        {
            command[i] = Command_Read(readIndex + i);
        }
        Command_AddReadIndex(length);
        return length;
    }
}
