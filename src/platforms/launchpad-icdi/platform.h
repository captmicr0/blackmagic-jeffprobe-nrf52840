#ifndef __PLATFORM_H
#define __PLATFORM_H

#include <stdint.h>

#include <setjmp.h>
#include <alloca.h>

#include <libopencm3/lm4f/gpio.h>
#include <libopencm3/usb/usbd.h>

#include "gdb_packet.h"

#define CDCACM_PACKET_SIZE 	64
#define BOARD_IDENT             "Black Magic Probe (Launchpad ICDI), (Firmware 1.5" VERSION_SUFFIX ", build " BUILDDATE ")"
#define BOARD_IDENT_DFU		"Black Magic (Upgrade) for Launchpad, (Firmware 1.5" VERSION_SUFFIX ", build " BUILDDATE ")"
#define DFU_IDENT               "Black Magic Firmware Upgrade (Launchpad)"
#define DFU_IFACE_STRING	"lolwut"

extern usbd_device *usbdev;
#define CDCACM_GDB_ENDPOINT	1
#define CDCACM_UART_ENDPOINT	3

extern jmp_buf fatal_error_jmpbuf;
extern uint8_t running_status;
extern volatile uint32_t timeout_counter;

#define TMS_PORT	GPIOA_BASE
#define TMS_PIN		GPIO3

#define TCK_PORT	GPIOA_BASE
#define TCK_PIN		GPIO2

#define TDI_PORT	GPIOA_BASE
#define TDI_PIN		GPIO5

#define TDO_PORT	GPIOA_BASE
#define TDO_PIN		GPIO4

#define SWO_PORT	GPIOD_BASE
#define SWO_PIN		GPIO6

#define SWDIO_PORT	TMS_PORT
#define SWDIO_PIN	TMS_PIN

#define SWCLK_PORT	TCK_PORT
#define SWCLK_PIN	TCK_PIN

#define SRST_PORT	GPIOA_BASE
#define SRST_PIN	GPIO6

#define TMS_SET_MODE()	{								\
	gpio_mode_setup(TMS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, TMS_PIN);		\
	gpio_set_output_config(TMS_PORT, GPIO_OTYPE_PP, GPIO_DRIVE_2MA, TMS_PIN);	\
}

#define SWDIO_MODE_FLOAT() {								\
	gpio_mode_setup(SWDIO_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, SWDIO_PIN);	\
}

#define SWDIO_MODE_DRIVE() {									\
	gpio_mode_setup(SWDIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, SWDIO_PIN);		\
	gpio_set_output_config(SWDIO_PORT, GPIO_OTYPE_PP, GPIO_DRIVE_2MA, SWDIO_PIN);		\
}

extern usbd_driver lm4f_usb_driver;
#define USB_DRIVER	lm4f_usb_driver
#define USB_IRQ		NVIC_USB0_IRQ
#define USB_ISR		usb0_isr

#define IRQ_PRI_USB	(2 << 4)

#define USBUART		UART0
#define USBUART_CLK	RCC_UART0
#define USBUART_IRQ	NVIC_UART0_IRQ
#define USBUART_ISR	uart0_isr
#define UART_PIN_SETUP() do {								\
	periph_clock_enable(RCC_GPIOA);							\
	__asm__("nop"); __asm__("nop"); __asm__("nop");					\
	gpio_set_af(GPIOA_BASE, 0x1, GPIO0 | GPIO1);				\
	gpio_mode_setup(GPIOA_BASE, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0);	\
	gpio_mode_setup(GPIOA_BASE, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO1);	\
	} while (0)

#define TRACEUART	UART2
#define TRACEUART_CLK	RCC_UART2
#define TRACEUART_IRQ	NVIC_UART2_IRQ
#define TRACEUART_ISR	uart2_isr

/* Use newlib provided integer only stdio functions */
#define sscanf siscanf
#define sprintf siprintf
#define vasprintf vasiprintf

#define DEBUG(...)

#define SET_RUN_STATE(state)	{running_status = (state);}
#define SET_IDLE_STATE(state)	{}
#define SET_ERROR_STATE(state)	SET_IDLE_STATE(state)

#define PLATFORM_SET_FATAL_ERROR_RECOVERY()	{setjmp(fatal_error_jmpbuf);}
#define PLATFORM_FATAL_ERROR(error) {			\
	if( running_status ) gdb_putpacketz("X1D");	\
		else gdb_putpacketz("EFF");		\
	running_status = 0;				\
	target_list_free();				\
	morse("TARGET LOST.", 1);			\
	longjmp(fatal_error_jmpbuf, (error));		\
}

#define PLATFORM_HAS_TRACESWO

int platform_init(void);

inline static void gpio_set_val(uint32_t port, uint8_t pin, uint8_t val) {
	gpio_write(port, pin, val == 0 ? 0 : 0xff);
}

inline static uint8_t gpio_get(uint32_t port, uint8_t pin) {
	return !(gpio_read(port, pin) == 0);
}

void platform_delay(uint32_t delay);
const char *platform_target_voltage(void);

/* <cdcacm.c> */
void cdcacm_init(void);
/* Returns current usb configuration, or 0 if not configured. */
int cdcacm_get_config(void);
int cdcacm_get_dtr(void);

#define disconnect_usb() do { usbd_disconnect(usbdev,1); nvic_disable_irq(USB_IRQ);} while(0)
#define setup_vbus_irq()

#endif
