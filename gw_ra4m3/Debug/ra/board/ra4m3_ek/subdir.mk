################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra/board/ra4m3_ek/board_init.c \
../ra/board/ra4m3_ek/board_leds.c 

C_DEPS += \
./ra/board/ra4m3_ek/board_init.d \
./ra/board/ra4m3_ek/board_leds.d 

OBJS += \
./ra/board/ra4m3_ek/board_init.o \
./ra/board/ra4m3_ek/board_leds.o 

SREC += \
gw_ra4m3.srec 

MAP += \
gw_ra4m3.map 


# Each subdirectory must supply rules for building sources it contributes
ra/board/ra4m3_ek/%.o: ../ra/board/ra4m3_ek/%.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RENESAS_RA_ -D_RA_CORE=CM33 -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/src" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/inc" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra/fsp/inc" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra/fsp/inc/api" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra/fsp/inc/instances" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra/arm/CMSIS_5/CMSIS/Core/Include" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra_gen" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra_cfg/fsp_cfg/bsp" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra_cfg/fsp_cfg" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

