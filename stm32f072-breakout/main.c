#include "ch.h"
#include "hal.h"
#include "usbcfg.h"
#include "test.h"

#include <chprintf.h>
#include <shell.h>

#include <stm32f072xb.h>

static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg)
{
    (void)arg;

    chRegSetThreadName("blink");

    palSetPadMode(GPIOA, 10, PAL_MODE_OUTPUT_PUSHPULL);
    while (1)
    {
	palClearPad(GPIOA, 10);
	chThdSleepMilliseconds(250);
	palSetPad(GPIOA, 10);
	chThdSleepMilliseconds(250);
    }
}

static void cmd_reset(BaseSequentialStream *chp, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    chprintf(chp, "Performing reset\r\n");

    rccEnableWWDG(FALSE);

    WWDG->CFR = 0x60;
    WWDG->CR = 0xff;

    while (1)
	;
}

/* Virtual serial port (CDC ACM) */
SerialUSBDriver SDU1;

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(2048)

static const ShellCommand commands[] =
{
    { "reset", cmd_reset },
    { NULL, NULL }
};

static const ShellConfig shell_cfg1 =
{
    (BaseSequentialStream *)&SDU1,
    commands
};

int main(void)
{
    thread_t *shell = NULL;

    halInit();
    chSysInit();

    shellInit();

    usbDisconnectBus(serusbcfg.usbp);

    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    chThdSleepMilliseconds(100);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

    while (1)
    {
	if (!shell)
	{
	    if (SDU1.config->usbp->state == USB_ACTIVE)
		shell = shellCreate(&shell_cfg1, SHELL_WA_SIZE, NORMALPRIO);
	}
	else
	{
	    if (chThdTerminatedX(shell))
	    {
		chThdRelease(shell);
		shell = NULL;
	    }
	}
	chThdSleepMilliseconds(100);
    }
}
