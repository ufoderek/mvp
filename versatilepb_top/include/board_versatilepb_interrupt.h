#ifndef _BOARD_VERSATILEPB_INTERRUPT_H_
#define _BOARD_VERSATILEPB_INTERRUPT_H_

/* for VIC_0 (irq0x00 ~ irq0x1f) */
#define INT_WATCHDOG    0x00
#define INT_SWI_0       0x01
#define INT_CORX        0x02
#define INT_COTX        0x03
#define INT_TIMER_0     0x04
#define INT_TIMER_1     0x05
#define INT_GPIO_0      0x06
#define INT_GPIO_1      0x07
#define INT_GPIO_2      0x08
#define INT_GPIO_3      0x09
#define INT_RTC         0x0a
#define INT_SSP         0x0b
#define INT_UART_0      0x0c
#define INT_UART_1      0x0d
#define INT_UART_2      0x0e
#define INT_SCI         0x0f
#define INT_CLCD        0x10
#define INT_DMA         0x11
#define INT_PWRFAIL     0x12
#define INT_MBX         0x13
#define INT_CALCULATOR  0x1e
#define INT_VIC_0       0x1f

/* for VIC_1 (irq0x20 ~ irq0x3f) */
#define INT_SWI_1       0x20
#define INT_MCI_B       0x21
#define INT_KEYBOARD    0x23
#define INT_MOUSE       0x24
#define INT_UART_3      0x26
#define INT_TOUCH       0x28
#define INT_KEYPAD      0x29
#define INT_DISKONCHIP  0x35
#define INT_MCI_A       0x36
#define INT_AACI        0x38
#define INT_ETHERNET    0x39
#define INT_USB         0x3a
#endif
