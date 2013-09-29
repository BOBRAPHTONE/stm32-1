#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "usbdetail.h"

#include "eventlogger.h"

/*
 * Globals
 */




int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Initializes serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU_PSAS);
  sduStart(&SDU_PSAS, &serusbcfg);

  /*!
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1000);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  /*!
   * Activates the serial driver 6 and SDC driver 1 using default
   * configuration.
   */
  sdStart(&SD6, NULL);

  chThdSleepMilliseconds(1300);

  eventlogger_init();

  /*
   * Normal main() thread activity,
   */
  while (1) {
    chThdSleep(MS2ST(1000));
  }
  exit(0);
}
