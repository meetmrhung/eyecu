/*
 * \file eyecu_pins.h
 *
 * \brief This file defines the pins and pin addresses
 * used in the project
 *
 * \author Mike Mozingo
 *
 */

#ifndef EYECU_PINS_H_
#define EYECU_PINS_H_

// defines the GPIO addressing
// FIFO connections
// GPIO1_0 --> CAMERA_D0
// GPIO1_1 --> CAMERA_D1
// GPIO1_2 --> CAMERA_D2
// GPIO1_3 --> CAMERA_D3
// GPIO1_4 --> CAMERA_D4
// GPIO1_5 --> CAMERA_D5
// GPIO1_6 --> CAMERA_D6
// GPIO1_7 --> CAMERA_D7

// GPIO_2_22 --> /PAE
// GPIO_2_23 --> /PAF
// GPIO_2_24 --> /FF
// GPIO_2_25 --> /EF

// GPIO1_14 --> /REN2
// GPIO1_15 --> /WEN1
// GPIO0_26 --> WEN2
// GPIO0_27 --> /REN1

// GPIO2_6 --> RCLK

// GPIO1_12 --> /RS



// CAMERA
// GPIO2_10 --> HD
// GPIO2_11 --> VD
// GPIO1_13 --> RST


// GPIO?? TIMER7 --> LED_Blink



// base addressing
#define GPIO_BASE0						(SOC_GPIO_0_REGS)
#define GPIO_BASE1						(SOC_GPIO_1_REGS)
#define GPIO_BASE2						(SOC_GPIO_2_REGS)

// TODO: define all pins + address required

// ?? defines an address + offset for some GPIO pins??? TODO: figure this out

#define CONTROL_CONF_GPMC1_AD(n)  		(0x804 + (n * 4))


#endif /* EYEEY_PINS_H_ */
