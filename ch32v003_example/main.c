#include "config.h"
#include <stdio.h>
#include "systick.h"
#include "encoder.h"

#include "tm1637.h"
#include "tm1637_i2c.h"
#include "timer.h"
#include "display.h"
#include "menu_mode.h"
#include "menu_cfg.h"
#include "main_mode.h"
#include "uart.h"
#include "protocol.h"
#include "motor.h"

void uart_send_complete()
{
    motor_uart_send_complete_enqueue();
}

void uart_received_line(const char* line)
{
    uint8_t cmd;
    uint32_t val;
    if (parse_rply(line, &cmd, &val)) {
        motor_rply_received_enqueue(cmd, val);
    } else {
        motor_rply_error_enqueue();
    }
}

static inline void button_press_enqueue()
{
    menu_mode_button_press_enqueue();
    menu_cfg_button_press_enqueue();
    main_mode_button_press_enqueue();
}
static inline void button_longpress_enqueue()
{
    menu_mode_button_longpress_enqueue();
    menu_cfg_button_longpress_enqueue();
    main_mode_button_longpress_enqueue();
}
static inline void button_release_enqueue()
{
    menu_mode_button_release_enqueue();
    menu_cfg_button_release_enqueue();
    main_mode_button_release_enqueue();
}

static inline void encoder_cw()
{
    menu_mode_encoder_cw_enqueue();
    menu_cfg_encoder_cw_enqueue();
    main_mode_encoder_cw_enqueue();
}
static inline void encoder_ccw()
{
    menu_mode_encoder_ccw_enqueue();
    menu_cfg_encoder_ccw_enqueue();
    main_mode_encoder_ccw_enqueue();
}

volatile static uint32_t button_press_syscnt = 0;

void systick_clock_missed()
{
}

void systick_clock()
{
    tm1637_i2c_clock_enqueue();
    timer_clock_enqueue();
    if (button_press_syscnt != 0 && button_press_syscnt != 0xFFFFFFFF) {
        if (SysTick->CNT - button_press_syscnt > 48000000) {
            // long button press
            button_longpress_enqueue();
            button_press_syscnt = 0xFFFFFFFF;
        }
    }
}

void exti_encoder_rotate(int dir, uint32_t _)
{
    if(dir) {
        encoder_cw();
    } else {
        encoder_ccw();
    }
}

void exti_button(int state, uint32_t tick)
{
    if (state) {
        button_press_syscnt = tick;
        button_press_enqueue();
    } else {
        if (button_press_syscnt != 0xFFFFFFFF) {
            // longpress was not activated
            button_release_enqueue();
        }
        button_press_syscnt = 0;
    }
}

int main()
{
    SystemInit();

    timer_init();
    tm1637_i2c_init();
    tm1637_init();
    display_init();
    menu_mode_init();
    menu_cfg_init();
    main_mode_init();
    systick_init();
    exti_init();
    uart_init();
    motor_init();

    timer_set(10000, tm1637_brightness_delayed(0x3));
    while(1) {
        timer_process_queue();
        tm1637_i2c_process_queue();
        tm1637_process_queue();
        main_mode_process_queue();
        menu_mode_process_queue();
        menu_cfg_process_queue();
        motor_process_queue();
    }
}
