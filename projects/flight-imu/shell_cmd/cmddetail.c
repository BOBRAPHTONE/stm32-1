/*! \file cmddetail.c
 *
 */

/*!
 * \defgroup cmddetail Command Utilities
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <time.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "ff.h"
#include "sdcdetail.h"

#include "chrtclib.h"

#include "cmddetail.h"

#include "psas_sdclog.h"
#include "psas_rtc.h"

#define         DEBUG_SHELLCMD

#ifdef DEBUG_SHELLCMD
#include "usbdetail.h"
BaseSequentialStream    *shellcmd   =  (BaseSequentialStream *)&SDU_PSAS;
#define SHELLDBG(format, ...) chprintf(shellcmd, format, ##__VA_ARGS__ )
#else
#define SHELLDBG(...)
#endif


static time_t      unix_time;

static uint8_t     fbuff[1024];

#define MAX_FILLER 11

static char *long_to_string_with_divisor(BaseSequentialStream *chp,
                                         char *p,
                                         unsigned long long num,
                                         unsigned radix,
                                         long divisor) {

    (void)chp;
  unsigned long long i;
  char *q;
  unsigned long long l, ll;
  char tmpbuf[MAX_FILLER + 1];

  tmpbuf[MAX_FILLER] = '\0';
  p = tmpbuf;
  q = tmpbuf;

  l = num;
  if (divisor == 0) {
    ll = num;
  } else {
    ll = divisor;
  }

  q = p + MAX_FILLER;
  do {
    i =  (unsigned long long)(l % radix);
    i += '0';
    if (i > '9')
      i += 'A' - '0' - 10;
    *--q = i;
    l /= radix;
  } while ((ll /= radix) != 0);

  i = (unsigned long long) (p + MAX_FILLER - q);
  do {
    *p++ = *q++;
  } while (--i);
  SHELLDBG("%s: %s\r\n", __func__, tmpbuf);
  return p;
}

void cmd_tree(BaseSequentialStream *chp, int argc, char *argv[]) {
    FRESULT err;
    unsigned long clusters;
    unsigned long long total;
    FATFS *fsp;
    int howbig;
    char* p;
   // char buffern[20];

    (void)argv;
    if (argc > 0) {
            SHELLDBG("Usage: tree\r\n");
            return;
    }
    if (!fs_ready) {
            SHELLDBG("File System not mounted\r\n");
            return;
    }
    err = f_getfree("/", &clusters, &fsp);
    if (err != FR_OK) {
            err = f_getfree("/", &clusters, &fsp);
            if (err != FR_OK) {
                    SHELLDBG("FS: f_getfree() failed. FRESULT: %d\r\n", err);
                    return;
            }
    }
    SHELLDBG("ULONG_MAX: %lu\n", ULONG_MAX);
    total =  1936690ULL * 8ULL * 512ULL;
    //total = clusters * (uint32_t)SDC_FS.csize * (uint32_t)MMCSD_BLOCK_SIZE;
    p = long_to_string_with_divisor(chp, p, total, 10, 0);
    howbig = sizeof(unsigned long);
    SHELLDBG("howbig: %i\r\n", howbig);
    SHELLDBG("FS: %lu free clusters, %lu sectors per cluster, %lu bytes free.\r\n",
            clusters, (uint32_t)SDC_FS.csize,
            total);
    fbuff[0] = 0;
    sdc_scan_files(chp, (char *)fbuff);
}


void cmd_date(BaseSequentialStream *chp, int argc, char *argv[]){
    (void)argv;
    (void)chp;
  struct   tm timp;
  RTCTime   psas_time;

  if (argc == 0) {
    goto ERROR;
  }

  if ((argc == 1) && (strcmp(argv[0], "get") == 0)){
	  psas_rtc_lld_get_time(&RTCD1, &psas_time);
      psas_stm32_rtc_bcd2tm(&timp, &psas_time);

      unix_time = mktime(&timp);

      if (unix_time == -1){
          SHELLDBG("incorrect time in RTC cell\r\n");
      }
      else{
          SHELLDBG("%Ds %Dus %s",unix_time, psas_time.tv_msec, " - unix time\r\n");
          SHELLDBG("%lu\r\n", psas_rtc_s.fc_boot_time_mark );
          rtcGetTimeTm(&RTCD1, &timp);
          SHELLDBG("%s%s",asctime(&timp)," - formatted time string\r\n");
      }
      // }
      return;
  }

  if ((argc == 2) && (strcmp(argv[0], "set") == 0)){
    unix_time = atol(argv[1]);
    if (unix_time > 0){
      rtcSetTimeUnixSec(&RTCD1, unix_time);
      return;
    }
    else{
      goto ERROR;
    }
  }
  else{
    goto ERROR;
  }

ERROR:
  SHELLDBG("Usage: date get\r\n");
  SHELLDBG("       date set N\r\n");
  SHELLDBG("where N is time in seconds sins Unix epoch\r\n");
  SHELLDBG("you can get current N value from unix console by the command\r\n");
  SHELLDBG("%s", "date +\%s\r\n");
  return;
}

/*! \brief Show memory usage
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
	size_t n, size;
    (void)chp;

	(void)argv;
	if (argc > 0) {
		SHELLDBG("Usage: mem\r\n");
		return;
	}
	n = chHeapStatus(NULL, &size);
	SHELLDBG("core free memory : %u bytes\r\n", chCoreStatus());
	SHELLDBG("heap fragments   : %u\r\n", n);
	SHELLDBG("heap free total  : %u bytes\r\n", size);
}


/*! \brief Show running threads
 *
 * @param chp
 * @param argc
 * @param argv
 */
void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
    (void)chp;
	static const char *states[] = {THD_STATE_NAMES};
	Thread *tp;

	(void)argv;
	if (argc > 0) {
		SHELLDBG("Usage: threads\r\n");
		return;
	}
	SHELLDBG("addr\t\tstack\t\tprio\trefs\tstate\t\ttime\tname\r\n");
	tp = chRegFirstThread();
	do {
		SHELLDBG("%.8lx\t%.8lx\t%4lu\t%4lu\t%9s\t%lu\t%s\r\n",
				(uint32_t)tp, (uint32_t)tp->p_ctx.r13,
				(uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
				states[tp->p_state], (uint32_t)tp->p_time, tp->p_name);
		tp = chRegNextThread(tp);
	} while (tp != NULL);
}


//! @}