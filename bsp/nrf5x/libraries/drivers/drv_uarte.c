/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-04-28     xckhmf       Modify for <nrfx>
 * 2020-10-31     xckhmf       Support for UART1
 *
 */
#include <rtdevice.h>
#include <nrfx_uarte.h>
#include "drv_uart.h"
#include "nrf_libuarte_async.h"
#include "nrfx_clock.h"

#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_queue.h"

#ifdef BSP_USING_UART
#if defined(BSP_USING_UART0) || defined(BSP_USING_UART1)|| defined(BSP_USING_UART2)|| defined(BSP_USING_UART3)
typedef struct
{
    struct rt_serial_device *serial;
    nrfx_uarte_t uarte_instance;
    uint8_t rx_length;
    uint8_t tx_buffer[1];
    uint8_t rx_buffer[1];
    bool isInit;
    uint32_t rx_pin;
    uint32_t tx_pin;
} drv_uart_cb_t;

#ifdef BSP_USING_UART0
static struct rt_serial_device m_serial_0;
drv_uart_cb_t m_uarte0_cb = {
    .uarte_instance = NRFX_UARTE_INSTANCE(1),
    .rx_length = 0,
    .rx_pin = BSP_UART0_RX_PIN,
    .tx_pin = BSP_UART0_TX_PIN,
    .isInit = false
};
#endif  /* BSP_USING_UART0 */

#ifdef BSP_USING_UART1
static struct rt_serial_device m_serial_1;
drv_uart_cb_t m_uarte1_cb = {
    .uarte_instance = NRFX_UARTE_INSTANCE(0),
    .rx_length = 0,
    .rx_pin = BSP_UART1_RX_PIN,
    .tx_pin = BSP_UART1_TX_PIN,
    .isInit = false
};
#endif  /* BSP_USING_UART1 */
#ifdef BSP_USING_UART2
static struct rt_serial_device m_serial_2;
drv_uart_cb_t m_uarte2_cb = {
    .uarte_instance = NRFX_UARTE_INSTANCE(2),
    .rx_length = 0,
    .rx_pin = BSP_UART2_RX_PIN,
    .tx_pin = BSP_UART2_TX_PIN,
    .isInit = false
};
#endif  /* BSP_USING_UART2 */
#ifdef BSP_USING_UART3
static struct rt_serial_device m_serial_3;
drv_uart_cb_t m_uarte3_cb = {
    .uarte_instance = NRFX_UARTE_INSTANCE(3),
    .rx_length = 0,
    .rx_pin = BSP_UART3_RX_PIN,
    .tx_pin = BSP_UART3_TX_PIN,
    .isInit = false
};
#endif  /* BSP_USING_UART3 */


static void uarte_evt_handler(nrfx_uarte_event_t const * p_event,
                              void *                     p_context)
{
    drv_uart_cb_t *p_cb = RT_NULL;
    p_cb = (drv_uart_cb_t*)p_context;
    switch (p_event->type)
    {
        case NRFX_UARTE_EVT_RX_DONE:
            p_cb->rx_length = p_event->data.rxtx.bytes;
            if(p_cb->serial->parent.open_flag&RT_DEVICE_FLAG_INT_RX)
            {
                rt_hw_serial_isr(p_cb->serial, RT_SERIAL_EVENT_RX_IND);
            }
            (void)nrfx_uarte_rx(&(p_cb->uarte_instance), p_cb->rx_buffer, 1);
            break;

        case NRFX_UARTE_EVT_ERROR:
            (void)nrfx_uarte_rx(&(p_cb->uarte_instance), p_cb->rx_buffer, 1);
            break;

        case NRFX_UARTE_EVT_TX_DONE:
            if(p_cb->serial->parent.open_flag&RT_DEVICE_FLAG_INT_TX)
            {
                rt_hw_serial_isr(p_cb->serial, RT_SERIAL_EVENT_TX_DONE);
            }
            break;

        default:
            break;
    }
}

static rt_err_t _uart_cfg(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    nrfx_uarte_config_t config = NRFX_UARTE_DEFAULT_CONFIG(NRF_UARTE_PSEL_DISCONNECTED,\
                                                            NRF_UARTE_PSEL_DISCONNECTED);

    drv_uart_cb_t *p_cb = RT_NULL;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    if (serial->parent.user_data == RT_NULL)
    {
        return -RT_ERROR;
    }
    p_cb = (drv_uart_cb_t*)serial->parent.user_data;
    if(p_cb->isInit)
    {
        nrfx_uarte_uninit(&(p_cb->uarte_instance));
        p_cb->isInit = false;
    }

    switch (cfg->baud_rate)
    {
    case BAUD_RATE_2400:
        config.baudrate = NRF_UARTE_BAUDRATE_2400;
        break;
    case BAUD_RATE_4800:
        config.baudrate = NRF_UARTE_BAUDRATE_4800;
        break;
    case BAUD_RATE_9600:
        config.baudrate = NRF_UARTE_BAUDRATE_9600;
        break;
    case BAUD_RATE_19200:
        config.baudrate = NRF_UARTE_BAUDRATE_19200;
        break;
    case BAUD_RATE_38400:
        config.baudrate = NRF_UARTE_BAUDRATE_38400;
        break;
    case BAUD_RATE_57600:
        config.baudrate = NRF_UARTE_BAUDRATE_57600;
        break;
    case BAUD_RATE_115200:
        config.baudrate = NRF_UARTE_BAUDRATE_115200;
        break;
    case BAUD_RATE_230400:
        config.baudrate = NRF_UARTE_BAUDRATE_230400;
        break;
    case BAUD_RATE_460800:
        config.baudrate = NRF_UARTE_BAUDRATE_460800;
        break;
    case BAUD_RATE_921600:
        config.baudrate = NRF_UARTE_BAUDRATE_921600;
        break;
	case 1000000:
        config.baudrate = NRF_UARTE_BAUDRATE_1000000;
		break;
    case BAUD_RATE_2000000:
    case BAUD_RATE_3000000:
        return -RT_EINVAL;
    default:
        config.baudrate = NRF_UARTE_BAUDRATE_115200;
        break;
    }
    config.hal_cfg.parity = (cfg->parity == PARITY_NONE)?\
                            NRF_UARTE_PARITY_EXCLUDED:NRF_UARTE_PARITY_INCLUDED;
    config.hal_cfg.hwfc = NRF_UARTE_HWFC_DISABLED;
    config.pselrxd = p_cb->rx_pin;
    config.pseltxd = p_cb->tx_pin;
    config.p_context = (void *)p_cb;

    nrfx_uarte_init(&(p_cb->uarte_instance),(nrfx_uarte_config_t const *)&config,uarte_evt_handler);
    nrfx_uarte_rx(&(p_cb->uarte_instance),p_cb->rx_buffer,1);
    p_cb->isInit = true;
    return RT_EOK;
}

static rt_err_t _uart_ctrl(struct rt_serial_device *serial, int cmd, void *arg)
{
    drv_uart_cb_t *p_cb = RT_NULL;
    RT_ASSERT(serial != RT_NULL);

    if (serial->parent.user_data == RT_NULL)
    {
        return -RT_ERROR;
    }
    p_cb = (drv_uart_cb_t*)serial->parent.user_data;

    switch (cmd)
    {
        /* disable interrupt */
    case RT_DEVICE_CTRL_CLR_INT:
        break;

        /* enable interrupt */
    case RT_DEVICE_CTRL_SET_INT:
        break;

    case RT_DEVICE_CTRL_CUSTOM:
        if ((rt_uint32_t)(arg) == UART_CONFIG_BAUD_RATE_9600)
        {
            p_cb->serial->config.baud_rate = 9600;
        }
        else if ((rt_uint32_t)(arg) == UART_CONFIG_BAUD_RATE_115200)
        {
            p_cb->serial->config.baud_rate = 115200;
        }
        _uart_cfg(serial, &(serial->config));
        break;

    case RT_DEVICE_CTRL_PIN:
        _uart_cfg(serial, &(serial->config));
        break;

    case RT_DEVICE_POWERSAVE:
        if(p_cb->isInit)
        {
            nrfx_uarte_uninit(&(p_cb->uarte_instance));
            p_cb->isInit = false;
        }
        break;

    case RT_DEVICE_WAKEUP:
        _uart_cfg(serial, &(serial->config));
        break;

    default:
        return -RT_ERROR;
    }

    return RT_EOK;
}

static int _uart_putc(struct rt_serial_device *serial, char c)
{
    drv_uart_cb_t *p_cb = RT_NULL;
    int rtn = -1;
    RT_ASSERT(serial != RT_NULL);

    if (serial->parent.user_data != RT_NULL)
    {
        p_cb = (drv_uart_cb_t*)serial->parent.user_data;
    }
    p_cb->tx_buffer[0] = c;
    nrfx_uarte_tx(&(p_cb->uarte_instance),p_cb->tx_buffer,1);
    if(!(serial->parent.open_flag&RT_DEVICE_FLAG_INT_TX))
    {
        while(nrfx_uarte_tx_in_progress(&(p_cb->uarte_instance)))
        {
        }
    }
    return rtn;
}

static int _uart_getc(struct rt_serial_device *serial)
{
    int ch = -1;
    drv_uart_cb_t *p_cb = RT_NULL;
    RT_ASSERT(serial != RT_NULL);

    if (serial->parent.user_data != RT_NULL)
    {
        p_cb = (drv_uart_cb_t*)serial->parent.user_data;
    }
    if(p_cb->rx_length)
    {
        ch = p_cb->rx_buffer[0];
        p_cb->rx_length--;
    }
    return ch;
}

static struct rt_uart_ops _uart_ops = {
    _uart_cfg,
    _uart_ctrl,
    _uart_putc,
    _uart_getc
};

int rt_hw_uart_init(void)
{
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

#ifdef BSP_USING_UART0
    m_serial_0.config = config;
	m_serial_0.config.baud_rate =  1000000;
    m_serial_0.ops = &_uart_ops;
    m_uarte0_cb.serial = &m_serial_0;
    rt_hw_serial_register(&m_serial_0, "uart0", \
                            RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX ,  &m_uarte0_cb);
#endif  /* BSP_USING_UART0 */

#ifdef BSP_USING_UART1
    m_serial_1.config = config;
    m_serial_1.ops = &_uart_ops;
    m_uarte1_cb.serial = &m_serial_1;
    rt_hw_serial_register(&m_serial_1, "uart1", \
                            RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX |RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX,  &m_uarte1_cb);
#endif  /* BSP_USING_UART1 */

#ifdef BSP_USING_UART2
    m_serial_2.config = config;
    m_serial_2.ops = &_uart_ops;
    m_uarte2_cb.serial = &m_serial_2;
    rt_hw_serial_register(&m_serial_2, "uart2", \
                            RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX |RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX,  &m_uarte2_cb);
#endif  /* BSP_USING_UART2 */

#ifdef BSP_USING_UART3
    m_serial_3.config = config;
    m_serial_3.ops = &_uart_ops;
    m_uarte3_cb.serial = &m_serial_3;
    rt_hw_serial_register(&m_serial_3, "uart3", \
                            RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX |RT_DEVICE_FLAG_INT_TX | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX,  &m_uarte3_cb);
#endif  /* BSP_USING_UART3 */


}


NRF_LIBUARTE_ASYNC_DEFINE(libuarte, 0, 0, 0, NRF_LIBUARTE_PERIPHERAL_NOT_USED, 255, 3);

static uint8_t text[] = "UART example started.\r\n Loopback:\r\n";
static uint8_t text_size = sizeof(text);
static volatile bool m_loopback_phase;
static uint8_t test=0;
static volatile bool flag=true;
static volatile uint8_t *pd;
typedef struct {
    uint8_t * p_data;
    uint32_t length;
} buffer_t;

NRF_QUEUE_DEF(buffer_t, m_buf_queue, 10, NRF_QUEUE_MODE_NO_OVERFLOW);

void uart_event_handler(void * context, nrf_libuarte_async_evt_t * p_evt)
{   uint32_t i;
    
    nrf_libuarte_async_t * p_libuarte = (nrf_libuarte_async_t *)context;
    ret_code_t ret;

    switch (p_evt->type)
    {
        case NRF_LIBUARTE_ASYNC_EVT_ERROR:

            break;
        case NRF_LIBUARTE_ASYNC_EVT_RX_DATA:
        //pd=p_evt->data.rxtx.p_data;
        //  for(i=0;i<p_evt->data.rxtx.length;i++)
        //  {
        //    if(*(pd) != test)
        //    {
        //        flag=false;
        //        break;
        //    }
        //     ++test;
        //     ++pd;
          
        //  }
            ret = nrf_libuarte_async_tx(p_libuarte,p_evt->data.rxtx.p_data, p_evt->data.rxtx.length);
            if (ret == NRF_ERROR_BUSY)
            {
                buffer_t buf = {
                    .p_data = p_evt->data.rxtx.p_data,
                    .length = p_evt->data.rxtx.length,
                };

                ret = nrf_queue_push(&m_buf_queue, &buf);
                APP_ERROR_CHECK(ret);
            }
            else
            {
                APP_ERROR_CHECK(ret);
            }

            m_loopback_phase = true;
            break;
        case NRF_LIBUARTE_ASYNC_EVT_TX_DONE:
            if (m_loopback_phase)
            {
                nrf_libuarte_async_rx_free(p_libuarte, p_evt->data.rxtx.p_data, p_evt->data.rxtx.length);
                if (!nrf_queue_is_empty(&m_buf_queue))
                {
                    buffer_t buf;
                    ret = nrf_queue_pop(&m_buf_queue, &buf);
                    APP_ERROR_CHECK(ret);
                    UNUSED_RETURN_VALUE(nrf_libuarte_async_tx(p_libuarte, buf.p_data, buf.length));
                }
            }
           
            break;
        default:
            break;
    }
}
static void clk_event_handler(nrfx_clock_evt_type_t event){}


static int uart_dma_sample(int argc, char *argv[])
	
{


    nrfx_clock_init(clk_event_handler);
    nrfx_clock_enable();
    nrfx_clock_lfclk_start();


    nrf_libuarte_async_config_t nrf_libuarte_async_config = {
            .tx_pin     = BSP_UART1_TX_PIN,
            .rx_pin     = BSP_UART1_RX_PIN,
            .baudrate   = NRF_UARTE_BAUDRATE_1000000,
            .parity     = NRF_UARTE_PARITY_EXCLUDED,
            .hwfc       = NRF_UARTE_HWFC_DISABLED,
            .timeout_us = 100,
            .int_prio   = APP_IRQ_PRIORITY_LOW
    };

     nrf_libuarte_async_init(&libuarte, &nrf_libuarte_async_config, uart_event_handler, (void *)&libuarte);

   

    nrf_libuarte_async_enable(&libuarte);

  nrf_libuarte_async_tx(&libuarte, text, text_size);
  

    while (true)
    {
       
    }

return RT_EOK;

}
MSH_CMD_EXPORT(uart_dma_sample, uartdma sample);

#endif /* defined(BSP_USING_UART0) || defined(BSP_USING_UART1) */
#endif /* BSP_USING_UART */
