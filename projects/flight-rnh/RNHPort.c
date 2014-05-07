#include <stddef.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "utils_general.h"
#include "utils_hal.h"

#include "RNHPort.h"

typedef enum {
    RNH_PORT_STATUS = 0,
    RNH_PORT_FAULT = 1,
    RNH_PORT_ON = 2,
    RNH_PORT_OFF = 3,
    RNH_PORT_CURRENT_FREQ = 4
} RNHAction;

static void cmd_port(struct RCICmdData * rci_data, void * user_data UNUSED);
const struct RCICommand RCI_CMD_PORT = {
    .name = "#PORT",
    .function = cmd_port,
    .user_data = NULL
};

#define NUM_PORT 8
static const uint32_t power[NUM_PORT] = {
    GPIO_E0_NODE1_N_EN,
    GPIO_E1_NODE2_N_EN,
    GPIO_E2_NODE3_N_EN,
    GPIO_E3_NODE4_N_EN,
    GPIO_E4_NC,
    GPIO_E5_NODE6_N_EN,
    GPIO_E6_NODE7_N_EN,
    GPIO_E7_NODE8_N_EN
};
static const uint32_t fault[NUM_PORT] = {
    GPIO_E8_NODE1_N_FLT,
    GPIO_E9_NODE2_N_FLT,
    GPIO_E10_NODE3_N_FLT,
    GPIO_E11_NODE4_N_FLT,
    GPIO_E12_NC,
    GPIO_E13_NODE6_N_FLT,
    GPIO_E14_NODE7_N_FLT,
    GPIO_E15_NODE8_N_FLT
};

#define portGPTfreq 40000

EVENTSOURCE_DECL(rnhPortCurrent);
static struct rnhPortCurrent outBuffer;

static void select_port_imon(int port){
    (port & 1) ? palSetPad(GPIOD, GPIO_D7_IMON_A0) : palClearPad(GPIOD, GPIO_D7_IMON_A0);
    (port & 2) ? palSetPad(GPIOD, GPIO_D8_IMON_A1) : palClearPad(GPIOD, GPIO_D8_IMON_A1);
}

static void ADCCallback(ADCDriver *adcp UNUSED, adcsample_t *buffer, size_t n UNUSED){
    static uint8_t remaining_samples = RNH_PORT_ALL;

    remaining_samples &= ~(1 << (buffer - outBuffer.current));

    if(!remaining_samples){
        remaining_samples = RNH_PORT_ALL;
        chSysLockFromIsr();
        chEvtBroadcastI(&rnhPortCurrent);
        chSysUnlockFromIsr();
    }
}

#define makeBankConversionGroup(channel) \
{ \
    .circular = FALSE, \
    .num_channels = 1, \
    .end_cb = ADCCallback, \
    .error_cb = NULL, \
    .cr1 = ADC_CR1_EOCIE, \
    .cr2 = ADC_CR2_SWSTART, \
    .smpr1 = ADC_SMPR1_SMP_AN ## channel (ADC_SAMPLE_480), \
    .smpr2 = 0, \
    .sqr1 = ADC_SQR1_NUM_CH(1), \
    .sqr2 = 0, \
    .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN ## channel) \
}

static void StartADCSample(GPTDriver *gptp UNUSED){
    static int activeport;

    static ADCConversionGroup bank0 = makeBankConversionGroup(10);
    static ADCConversionGroup bank1 = makeBankConversionGroup(11);

    select_port_imon(activeport);
    chSysLockFromIsr();
    if((1 << activeport) & RNH_PORT_ALL){
        adcStartConversionI(&ADCD1, &bank0, &outBuffer.current[activeport], 1);
    }
    if((1 << (activeport + 4)) & RNH_PORT_ALL){
        adcStartConversionI(&ADCD2, &bank1, &outBuffer.current[activeport + 4], 1);
    }
    chSysUnlockFromIsr();
    activeport = (activeport + 1) % 4;
}


static rnhPortFaultHandler fault_handler = NULL;
static void * fault_handler_data = NULL;

static void portErrorCallback (EXTDriver *extp UNUSED, expchannel_t channel){
    if(fault_handler){
        fault_handler(channel - NUM_PORT, fault_handler_data);
    }
}

void rnhPortStart(void){
    static ADCConfig conf = {0}; //nothing to config on STM32

    adcStart(&ADCD1, &conf);
    adcStart(&ADCD2, &conf);

    static GPTConfig gptcfg = {
            .frequency = portGPTfreq,
            .callback = StartADCSample,
            .dier = 0,
    };
    gptStart(&GPTD2, &gptcfg);
    gptStartContinuous(&GPTD2, 1000);


    for(int i = 0; i < NUM_PORT; ++i){
        extAddCallback( &(struct pin){.port=GPIOE, .pad=fault[i]}
                      , EXT_CH_MODE_RISING_EDGE | EXT_CH_MODE_AUTOSTART
                      , portErrorCallback
                      );
    }
    extUtilsStart();
}

RNHPort rnhPortStatus(void){
    RNHPort return_port = 0;

    for(int i = 0; i < NUM_PORT; ++i){
        return_port |= palReadPad(GPIOE, power[i])<<i;
    }
    return return_port & RNH_PORT_ALL;
};

RNHPort rnhPortFault(void){
    RNHPort return_port = 0;

    for(int i = 0; i < NUM_PORT; ++i){
        return_port |= palReadPad(GPIOE, fault[i])<<i;
    }
    return return_port & RNH_PORT_ALL;
}

void rnhPortOn(RNHPort port){
    port &= RNH_PORT_ALL;

    for(int i = 0; i < NUM_PORT; ++i){
        if(port & 1<<i){
            palClearPad(GPIOE, power[i]);
        }
    }
}

void rnhPortOff(RNHPort port){
    port &= RNH_PORT_ALL;

    for(int i = 0; i < NUM_PORT; ++i){
        if(port & 1<<i){
            palSetPad(GPIOE, power[i]);
        }
    }
}

void rnhPortSetFaultHandler(rnhPortFaultHandler handler, void * data){
    chSysLock();
    fault_handler = handler;
    fault_handler_data = data;
    chSysUnlock();
}

void rnhPortGetCurrentData(struct rnhPortCurrent * measurement){
     chSysLock();
     *measurement = outBuffer;
     chSysUnlock();
 }

void rnhPortSetCurrentDataRate(unsigned freq){
    gptStopTimer(&GPTD2);
    gptStartContinuous(&GPTD2, portGPTfreq / 4 / freq);
}

static void cmd_port(struct RCICmdData * rci_data, void * user_data UNUSED){
    if(rci_data->cmd_len < 1){
        return;
    }

    RNHAction action = rci_data->cmd_data[0];
    int data = 0;
    for(int i = 1; i < rci_data->cmd_len; ++i){
        data <<= 8;
        data |= rci_data->cmd_data[i];
    }
    RNHPort ret = 0;

    switch(action){
    case RNH_PORT_STATUS:
        ret = rnhPortStatus();
        break;
    case RNH_PORT_FAULT:
        ret = rnhPortFault();
        break;
    case RNH_PORT_ON:
        rnhPortOn(data);
        ret = rnhPortStatus();
        break;
    case RNH_PORT_OFF:
        rnhPortOff(data);
        ret = rnhPortStatus();
        break;
    case RNH_PORT_CURRENT_FREQ:
        rnhPortSetCurrentDataRate(data);
        return;
    default:
        return;
    }

    rci_data->return_data[0] = ret;
    rci_data->return_len = 1;
}
