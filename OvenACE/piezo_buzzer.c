/*
 * Piezo Buzzer Handler
 *
 * Can play music on the Piezo Buzzer.
 */   
#include "atmel_asf4.h"
#include "piezo_buzzer.h"

#include "SimplePIO.h"
#include "HWPinAssignments.h"

#define DEFAULT_HZ 440

#define TCC(x) TCC_(x)
#define TCC_(x) TCC ## x

#define BUZZER_APBCMASK(x) BUZZER_APBCMASK_(x)
#define BUZZER_APBCMASK_(x) (PM->APBCMASK.bit.TCC ## x ## _)

/**
 * PlayTone
 *
 * \param[in] freq The raw frequency to put on the buzzer. 
 *                 0 = stop.
 *
 */
void PlayTone(uint16_t freq)
{
    PORT_PINCFG_Type cfg = {0};
    cfg.bit.INEN   = false;
    cfg.bit.PULLEN = false;
    cfg.bit.DRVSTR = true;

    if (freq == 0) {
        // Disable MUX (Silence buzzer)
        cfg.bit.PMUXEN = false;
        PORT_PINCFG(PIEZO_BUZZER) = cfg;
    } else {
        // Change Frequency and Enable MUX (Enable Buzzer)
        TCC(PIEZO_TCC)->PERB.reg = freq;
        cfg.bit.PMUXEN = true;
        PORT_PINCFG(PIEZO_BUZZER) = cfg;
    }
}

void initPiezoBuzzer(void)
{

    // 1. Enable the TCC bus clock (CLK_TCCx_APB).
    BUZZER_APBCMASK(PIEZO_TCC) = true;

    /* Set GCLK_GEN0 as source for GCLK_ID_SERCOMx_CORE */
    GCLK_CLKCTRL_Type clkctrl = {0};
    clkctrl.bit.GEN   = 3; // GP Clock (1.8Mhz)
    clkctrl.bit.CLKEN = true;
    clkctrl.bit.ID    = GCLK_CLKCTRL_ID_TCC2_TC3_Val;
    GCLK->CLKCTRL     = clkctrl;

    // 2. Software Reset the TCC (Which also disables it)
    TCC_CTRLA_Type tccctrl_a = {0};
    tccctrl_a.bit.SWRST = true;
    TCC(PIEZO_TCC)->CTRLA = tccctrl_a;
    while ((TCC(PIEZO_TCC)->CTRLA.bit.SWRST) == 1) {}
    while ((TCC(PIEZO_TCC)->SYNCBUSY.bit.SWRST) == 1) {}

    // 3. Set waveform Generation Mode.
    // Note: Default Waveform mode is to toggle the output on overflow, 
    // which is exactly what we want.  Nothing to do.

    // 4. Set the period (which defines our frequency)
    // We use a divide by 64 on the CPU Clock, which gives a 
    // range in a 16 bit counter to generate audible frequencies.
    // The Output toggles every overflow, so we need to do that
    // twice as often as the desired frequency.
    TCC_PER_Type tccper  = {0};
    tccper.bit.PER       = HZ_TO_FREQ(DEFAULT_HZ);
    TCC(PIEZO_TCC)->PER  = tccper;

    // 5. Set up the IO Port Mux to enable the waveform output.
    PORT_PMUX_Type pmux = {0};
    pmux.reg = PORT_PMUX(PIEZO_BUZZER);
#if ((PIEZO_BUZZER & 1) == 0)
    pmux.bit.PMUXE = PORT_PMUX_PMUXE_E_Val;
#else
    pmux.bit.PMUXO = PORT_PMUX_PMUXE_E_Val;
#endif
    PORT_PMUX(PIEZO_BUZZER) = pmux.reg;

    // 6. Make sure the Buzzer is off, to start.
    PORT_PINCFG_Type cfg = {0};
    cfg.bit.INEN   = false;
    cfg.bit.PULLEN = false;
    cfg.bit.DRVSTR = true;
    cfg.bit.PMUXEN = false;
    PORT_PINCFG(PIEZO_BUZZER) = cfg;

    // 7. Finally, set CTRLA and Enable the Timer.
    tccctrl_a.bit.SWRST      = false;
    tccctrl_a.bit.ENABLE     = true;
    tccctrl_a.bit.RESOLUTION = TCC_CTRLA_RESOLUTION_NONE_Val; 
    tccctrl_a.bit.PRESCALER  = TCC_CTRLA_PRESCALER_DIV64_Val;
    tccctrl_a.bit.RUNSTDBY   = true;
    tccctrl_a.bit.PRESCSYNC  = TCC_CTRLA_PRESCSYNC_RESYNC_Val;
    tccctrl_a.bit.ALOCK      = false;
    tccctrl_a.bit.CPTEN0     = false;
    tccctrl_a.bit.CPTEN1     = false;
    tccctrl_a.bit.CPTEN2     = false;
    tccctrl_a.bit.CPTEN3     = false;
    TCC(PIEZO_TCC)->CTRLA = tccctrl_a;

    // NOW, all we need to do to toggle the waveform on/off is
    // Enable or Disable the pinmux, as required.
    // Change PER to change the frequency.
}
