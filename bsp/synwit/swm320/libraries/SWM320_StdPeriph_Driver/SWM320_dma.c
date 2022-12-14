/******************************************************************************************************************************************
* 文件名称: SWM320_dma.c
* 功能说明: SWM320单片机的DMA功能驱动库
* 技术支持: http://www.synwit.com.cn/e/tool/gbook/?bid=1
* 注意事项:
* 版本日期: V1.1.0      2017年10月25日
* 升级记录:
*
*
*******************************************************************************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH CODING INFORMATION
* REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME. AS A RESULT, SYNWIT SHALL NOT BE HELD LIABLE
* FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
* OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN IN CONN-
* -ECTION WITH THEIR PRODUCTS.
*
* COPYRIGHT 2012 Synwit Technology
*******************************************************************************************************************************************/
#include "SWM320.h"
#include "SWM320_dma.h"

/******************************************************************************************************************************************
* 函数名称: DMA_CHM_Config()
* 功能说明: DMA通道配置，用于存储器间（如Flash和RAM间）搬运数据
* 输    入: uint32_t chn          指定要配置的通道，有效值有DMA_CH0、DMA_CH1、DMA_CH2
*           uint32_t src_addr       源地址，必须字对齐，即地址的最低2位必须是00
*           uint32_t src_addr_incr  0 固定地址    1 地址递增
*           uint32_t dst_addr       目的地址，必须字对齐，即地址的最低2位必须是00
*           uint32_t dst_addr_incr  0 固定地址    1 地址递增
*           uint32_t num_word       要搬运的数据字数，最大1024
*           uint32_t int_en         中断使能，1 数据搬运完成后产生中断    0 数据搬运完成后不产生中断
* 输    出: 无
* 注意事项: 搬运数据量以字为单元，不是字节
******************************************************************************************************************************************/
void DMA_CHM_Config(uint32_t chn, uint32_t src_addr, uint32_t src_addr_incr, uint32_t dst_addr, uint32_t dst_addr_incr, uint32_t num_word, uint32_t int_en)
{
    DMA->EN = 1;            //每个通道都有自己独立的开关控制，所以总开关可以是一直开启的

    DMA_CH_Close(chn);      //配置前先关闭该通道

    DMA->CH[chn].SRC = src_addr;
    DMA->CH[chn].DST = dst_addr;

    DMA->CH[chn].CR = ((num_word*4-1) << DMA_CR_LEN_Pos) |
                      (0 << DMA_CR_AUTORE_Pos);

    DMA->CH[chn].AM = (src_addr_incr << DMA_AM_SRCAM_Pos) |
                      (dst_addr_incr << DMA_AM_DSTAM_Pos) |
                      (0 << DMA_AM_BURST_Pos);

    DMA->IF  = (1 << chn);      //清除中断标志
    DMA->IE |= (1 << chn);
    if(int_en)  DMA->IM &= ~(1 << chn);
    else        DMA->IM |=  (1 << chn);

    if(int_en)
    {
        NVIC_EnableIRQ(DMA_IRQn);
    }
    else
    {
        //不能调用NVIC_DisalbeIRQ(DMA_IRQn)，因为其他通道可能使用DMA中断
    }
}

/******************************************************************************************************************************************
* 函数名称: DMA_CH_Open()
* 功能说明: DMA通道打开
* 输    入: uint32_t chn          指定要配置的通道，有效值有DMA_CH0、DMA_CH1、DMA_CH2
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void DMA_CH_Open(uint32_t chn)
{
    DMA->CH[chn].CR |= (1 << DMA_CR_TXEN_Pos);
}

/******************************************************************************************************************************************
* 函数名称: DMA_CH_Close()
* 功能说明: DMA通道关闭
* 输    入: uint32_t chn          指定要配置的通道，有效值有DMA_CH0、DMA_CH1、DMA_CH2
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void DMA_CH_Close(uint32_t chn)
{
    DMA->CH[chn].CR &= ~(1 << DMA_CR_TXEN_Pos);
}

/******************************************************************************************************************************************
* 函数名称: DMA_CH_INTEn()
* 功能说明: DMA中断使能，数据搬运完成后触发中断
* 输    入: uint32_t chn          指定要配置的通道，有效值有DMA_CH0、DMA_CH1、DMA_CH2
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void DMA_CH_INTEn(uint32_t chn)
{
    DMA->IM &= ~(1 << chn);
}

/******************************************************************************************************************************************
* 函数名称: DMA_CH_INTDis()
* 功能说明: DMA中断禁止，数据搬运完成后不触发中断
* 输    入: uint32_t chn          指定要配置的通道，有效值有DMA_CH0、DMA_CH1、DMA_CH2
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void DMA_CH_INTDis(uint32_t chn)
{
    DMA->IM |= (1 << chn);
}

/******************************************************************************************************************************************
* 函数名称: DMA_CH_INTClr()
* 功能说明: DMA中断标志清除
* 输    入: uint32_t chn          指定要配置的通道，有效值有DMA_CH0、DMA_CH1、DMA_CH2
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
void DMA_CH_INTClr(uint32_t chn)
{
    DMA->IF = (1 << chn);
}

/******************************************************************************************************************************************
* 函数名称: DMA_CH_INTStat()
* 功能说明: DMA中断状态查询
* 输    入: uint32_t chn          指定要配置的通道，有效值有DMA_CH0、DMA_CH1、DMA_CH2
* 输    出: uint32_t              1 数据搬运完成    0 数据搬运未完成
* 注意事项: 无
******************************************************************************************************************************************/
uint32_t DMA_CH_INTStat(uint32_t chn)
{
    return (DMA->IF & (1 << chn)) ? 1 : 0;
}
