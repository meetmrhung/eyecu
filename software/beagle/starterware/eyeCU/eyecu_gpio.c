#include "eyecu_gpio.h"

#include "soc_AM335x.h"
#include "beaglebone.h"
#include "hw_types.h"

#include "gpio_v2.h"

#include "hw_control_AM335x.h"
#include "hw_cm_wkup.h"
#include "hw_cm_per.h"

#include "eyecu_pins.h"

/******************************************************************************
 **              INTERNAL FUNCTION PROTOTYPES
 ******************************************************************************/
static void GPIO2ModuleClkConfig(void);
static void gpioPinMux(void);
static void gpioDirModeSet(void);

void gpioInit(void)
{
	// enable clock for gpio 0,1,2
	GPIO0ModuleClkConfig();
	GPIO1ModuleClkConfig();
	GPIO2ModuleClkConfig();


	// select gpio pins for use
	gpioPinMux();

	/* Enabling the GPIO module. */
	GPIOModuleEnable(GPIO_BASE1);
	GPIOModuleEnable(GPIO_BASE0);
	GPIOModuleEnable(GPIO_BASE2);

	/* Resetting the GPIO module. */
	GPIOModuleReset(GPIO_BASE0);
	GPIOModuleReset(GPIO_BASE1);
	GPIOModuleReset(GPIO_BASE2);

	// set directions for the gpio pins in use
	gpioDirModeSet();
}

/******************************************************************************
 **              FUNCTION DEFINITIONS
 ******************************************************************************/
// TODO: need to init more pins here
void gpioPinMux(void)
{
	// gpio1 pin 23
	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_A(7)) = CONTROL_CONF_MUXMODE(7);

	//
	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_AD(2)) = CONTROL_CONF_MUXMODE(7);

	//
	HWREG(SOC_CONTROL_REGS + CONTROL_CONF_GPMC_OEN_REN) = CONTROL_CONF_MUXMODE(7);
}

// TODO:L need to set dir correctly + more pins
void gpioDirModeSet(void)
{
	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(GPIO_BASE1, 23, GPIO_DIR_OUTPUT);
	GPIODirModeSet(GPIO_BASE1, 17, GPIO_DIR_OUTPUT);
    GPIODirModeSet(GPIO_BASE0, 4, GPIO_DIR_OUTPUT);
	GPIODirModeSet(GPIO_BASE2, 3, GPIO_DIR_OUTPUT);
}

void GPIO2ModuleClkConfig(void)
{

    /* Writing to MODULEMODE field of CM_PER_GPIO2_CLKCTRL register. */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) |=
          CM_PER_GPIO2_CLKCTRL_MODULEMODE_ENABLE;

    /* Waiting for MODULEMODE field to reflect the written value. */
    while(CM_PER_GPIO2_CLKCTRL_MODULEMODE_ENABLE !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
           CM_PER_GPIO2_CLKCTRL_MODULEMODE));
    /*
    ** Writing to OPTFCLKEN_GPIO_2_GDBCLK bit in CM_PER_GPIO2_CLKCTRL
    ** register.
    */
    HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) |=
          CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK;

    /*
    ** Waiting for OPTFCLKEN_GPIO_2_GDBCLK bit to reflect the desired
    ** value.
    */
    while(CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
           CM_PER_GPIO2_CLKCTRL_OPTFCLKEN_GPIO_2_GDBCLK));

    /*
    ** Waiting for IDLEST field in CM_PER_GPIO2_CLKCTRL register to attain the
    ** desired value.
    */
    while((CM_PER_GPIO2_CLKCTRL_IDLEST_FUNC <<
           CM_PER_GPIO2_CLKCTRL_IDLEST_SHIFT) !=
           (HWREG(SOC_CM_PER_REGS + CM_PER_GPIO2_CLKCTRL) &
            CM_PER_GPIO2_CLKCTRL_IDLEST));

    /*
    ** Waiting for CLKACTIVITY_GPIO_2_GDBCLK bit in CM_PER_L4LS_CLKSTCTRL
    ** register to attain desired value.
    */
    while(CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_2_GDBCLK !=
          (HWREG(SOC_CM_PER_REGS + CM_PER_L4LS_CLKSTCTRL) &
           CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_GPIO_2_GDBCLK));
}
