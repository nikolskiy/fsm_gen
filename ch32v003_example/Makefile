all : flash

TARGET:=example
CH32V003FUN:=tools/ch32v003fun

FSM_FILES:=$(wildcard *.fsm)
ADDITIONAL_C_FILES:=$(FSM_FILES:.fsm=.c)
ADDITIONAL_C_FILES+=config.c system_state.c

%.c: %.fsm
	../fsm_gen $^

include tools/ch32v003fun/ch32v003fun.mk


flash : cv_flash
clean : cv_clean

