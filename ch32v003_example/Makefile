all : flash

TARGET:=main
CH32V003FUN:=tools/ch32v003fun

FSM_FILES:=$(wildcard *.fsm)
FSM_C_FILES:=$(FSM_FILES:.fsm=.c)
FSM_H_FILES:=$(FSM_FILES:.fsm=.h)
ADDITIONAL_C_FILES+=config.c system_state.c uart.c systick.c encoder.c protocol.c  $(FSM_C_FILES)

%.c: %.fsm
	fsm_gen $^

include tools/ch32v003fun/ch32v003fun.mk

fsm_clean :
	rm $(FSM_C_FILES) $(FSM_H_FILES)

flash : cv_flash
clean : cv_clean fsm_clean

