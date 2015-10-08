#ifndef _BOARD_REALVIEWEB_MAP_H_
#define _BOARD_REALVIEWEB_MAP_H_
#define BASE_RAM        0x00000000
#define END_RAM         (BASE_RAM + (ram_size << 20) - 1)
#define BASE_SYS_CTRL_0 0x10000000
#define END_SYS_CTRL_0  (BASE_SYS_CTRL_0 + 0x00000fff)
#define BASE_SYS_CTRL_1 0x10001000
#define END_SYS_CTRL_1  (BASE_SYS_CTRL_1 + 0x00000fff)
#define BASE_AACI       0x10004000
#define END_AACI        (BASE_AACI + 0x00000fff)
#define BASE_MCI        0x10005000
#define END_MCI     (BASE_MCI + 0x00000fff)
#define BASE_KEYBOARD   0x10006000
#define END_KEYBOARD    (BASE_KEYBOARD + 0x00000fff)
#define BASE_MOUSE      0x10007000
#define END_MOUSE       (BASE_MOUSE + 0x00000fff)
#define BASE_UART_0     0x10009000
#define END_UART_0      (BASE_UART_0 + 0x00000fff)
#define BASE_UART_1     0x1000a000
#define END_UART_1      (BASE_UART_1 + 0x00000fff)
#define BASE_UART_2     0x1000b000
#define END_UART_2      (BASE_UART_2 + 0x00000fff)
#define BASE_UART_3     0x1000c000
#define END_UART_3      (BASE_UART_3 + 0x00000fff)
#define BASE_SSP        0x1000d000
#define END_SSP         (BASE_SSP + 0x00000fff)
#define BASE_SCI        0x1000e000
#define END_SCI         (BASE_SCI + 0x00000fff)
#define BASE_WATCHDOG   0x10010000
#define END_WATCHDOG    (BASE_WATCHDOG + 0x00000fff)
#define BASE_TIMER_0    0x10011000
#define END_TIMER_0     (BASE_TIMER_0 + 0x00000fff)
#define BASE_TIMER_1    0x10012000
#define END_TIMER_1     (BASE_TIMER_1 + 0x00000fff)
#define BASE_GPIO_0     0x10013000
#define END_GPIO_0      (BASE_GPIO_0 + 0x00000fff)
#define BASE_GPIO_1     0x10014000
#define END_GPIO_1      (BASE_GPIO_1 + 0x00000fff)
#define BASE_GPIO_2     0x10015000
#define END_GPIO_2      (BASE_GPIO_2 + 0x00000fff)
#define BASE_RTC        0x10017000
#define END_RTC         (BASE_RTC + 0x00000fff)
#define BASE_CLCD       0x10020000
#define END_CLCD        (BASE_CLCD + 0x0000ffff)
#define BASE_DMA        0x10030000
#define END_DMA         (BASE_DMA + 0x0000ffff)

#define BASE_GIC_0      0x10040000

#define BASE_SSMC       0x10080000
#define BASE_SMP_BOOT_ROM 0xE0000000
#endif
