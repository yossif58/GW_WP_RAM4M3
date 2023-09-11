################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/DLMScrc.c \
../src/Wrapper.c \
../src/crc.c \
../src/flash_hp_ep.c \
../src/gsm.c \
../src/hal_entry.c \
../src/srec2bin.c \
../src/uart_LTE.c \
../src/utils.c 

C_DEPS += \
./src/DLMScrc.d \
./src/Wrapper.d \
./src/crc.d \
./src/flash_hp_ep.d \
./src/gsm.d \
./src/hal_entry.d \
./src/srec2bin.d \
./src/uart_LTE.d \
./src/utils.d 

OBJS += \
./src/DLMScrc.o \
./src/Wrapper.o \
./src/crc.o \
./src/flash_hp_ep.o \
./src/gsm.o \
./src/hal_entry.o \
./src/srec2bin.o \
./src/uart_LTE.o \
./src/utils.o 

SREC += \
gw_ra4m3.srec 

MAP += \
gw_ra4m3.map 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal  -g -D_RENESAS_RA_ -D_RA_CORE=CM33 -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/src" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/inc" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra/fsp/inc" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra/fsp/inc/api" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra/fsp/inc/instances" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra/arm/CMSIS_5/CMSIS/Core/Include" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra_gen" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra_cfg/fsp_cfg/bsp" -I"C:/yosif/wtaer_pulser_RA2E1/water_meter_bulk/gw_ra4m3/ra_cfg/fsp_cfg" -std=c99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

