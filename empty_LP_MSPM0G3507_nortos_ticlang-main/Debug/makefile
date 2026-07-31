################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS

GEN_OPTS__FLAG := @"./device.opt" 
GEN_CMDS__FLAG := -Wl,-l"./device_linker.cmd" 

ORDERED_OBJS += \
"./empty.o" \
"./ti_msp_dl_config.o" \
"./startup_mspm0g350x_ticlang.o" \
"./app/app_car.o" \
"./app/app_debug.o" \
"./app/app_scheduler.o" \
"./bsp/bsp_board.o" \
"./bsp/bsp_encoder.o" \
"./bsp/bsp_i2c.o" \
"./bsp/bsp_imu_uart.o" \
"./bsp/bsp_line_adc.o" \
"./bsp/bsp_oled_soft_i2c.o" \
"./bsp/bsp_tb6612.o" \
"./bsp/bsp_time.o" \
"./bsp/bsp_uart.o" \
"./control/line_control.o" \
"./control/pid.o" \
"./control/speed_control.o" \
"./control/vehicle_mixer.o" \
"./drivers/encoder.o" \
"./drivers/imu.o" \
"./drivers/line_sensor.o" \
"./drivers/motor.o" \
"./drivers/oled.o" \
"./drivers/oled_data.o" \
"./drivers/tb6612.o" \
"./empty_LP_MSPM0G3507_nortos_ticlang/Debug/ti_msp_dl_config.o" \
$(GEN_CMDS__FLAG) \
-Wl,-ldevice.cmd.genlibs \
-Wl,-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include app/subdir_vars.mk
-include bsp/subdir_vars.mk
-include control/subdir_vars.mk
-include drivers/subdir_vars.mk
-include empty_LP_MSPM0G3507_nortos_ticlang/Debug/subdir_vars.mk
-include empty_LP_MSPM0G3507_nortos_ticlang/subdir_vars.mk
-include subdir_rules.mk
-include app/subdir_rules.mk
-include bsp/subdir_rules.mk
-include control/subdir_rules.mk
-include drivers/subdir_rules.mk
-include empty_LP_MSPM0G3507_nortos_ticlang/Debug/subdir_rules.mk
-include empty_LP_MSPM0G3507_nortos_ticlang/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
empty_LP_MSPM0G3507_nortos_ticlang.out 

EXE_OUTPUTS__QUOTED += \
"empty_LP_MSPM0G3507_nortos_ticlang.out" 

BIN_OUTPUTS += \
empty_LP_MSPM0G3507_nortos_ticlang.hex 

BIN_OUTPUTS__QUOTED += \
"empty_LP_MSPM0G3507_nortos_ticlang.hex" 


# All Target
all: $(OBJS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "empty_LP_MSPM0G3507_nortos_ticlang.out" secondary-outputs

# Tool invocations
empty_LP_MSPM0G3507_nortos_ticlang.out: $(OBJS) $(GEN_CMDS)
	@echo 'Arm Linker - building target: "$@"'
	"C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -g -Wall -Wl,-m"empty_LP_MSPM0G3507_nortos_ticlang.map" -Wl,-i"C:/ti/mspm0_sdk_2_11_00_07/source" -Wl,-i"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang" -Wl,-i"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/Debug/syscfg" -Wl,-i"C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/lib" -Wl,--diag_wrap=off -Wl,--display_error_number -Wl,--warn_sections -Wl,--xml_link_info="empty_LP_MSPM0G3507_nortos_ticlang_linkInfo.xml" -Wl,--rom_model -o "empty_LP_MSPM0G3507_nortos_ticlang.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

empty_LP_MSPM0G3507_nortos_ticlang.hex: $(EXE_OUTPUTS)
	@echo 'Arm Hex Utility - building secondary target: "$@"'
	"C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmhex.exe" --memwidth=8 --romwidth=8 --diag_wrap=off --intel -o "empty_LP_MSPM0G3507_nortos_ticlang.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(GEN_MISC_FILES__QUOTED)$(BIN_OUTPUTS__QUOTED)$(GEN_FILES__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "empty.o" "ti_msp_dl_config.o" "startup_mspm0g350x_ticlang.o" "app\app_car.o" "app\app_debug.o" "app\app_scheduler.o" "bsp\bsp_board.o" "bsp\bsp_encoder.o" "bsp\bsp_i2c.o" "bsp\bsp_imu_uart.o" "bsp\bsp_line_adc.o" "bsp\bsp_oled_soft_i2c.o" "bsp\bsp_tb6612.o" "bsp\bsp_time.o" "bsp\bsp_uart.o" "control\line_control.o" "control\pid.o" "control\speed_control.o" "control\vehicle_mixer.o" "drivers\encoder.o" "drivers\imu.o" "drivers\line_sensor.o" "drivers\motor.o" "drivers\oled.o" "drivers\oled_data.o" "drivers\tb6612.o" "empty_LP_MSPM0G3507_nortos_ticlang\Debug\ti_msp_dl_config.o" 
	-$(RM) "empty.d" "ti_msp_dl_config.d" "startup_mspm0g350x_ticlang.d" "app\app_car.d" "app\app_debug.d" "app\app_scheduler.d" "bsp\bsp_board.d" "bsp\bsp_encoder.d" "bsp\bsp_i2c.d" "bsp\bsp_imu_uart.d" "bsp\bsp_line_adc.d" "bsp\bsp_oled_soft_i2c.d" "bsp\bsp_tb6612.d" "bsp\bsp_time.d" "bsp\bsp_uart.d" "control\line_control.d" "control\pid.d" "control\speed_control.d" "control\vehicle_mixer.d" "drivers\encoder.d" "drivers\imu.d" "drivers\line_sensor.d" "drivers\motor.d" "drivers\oled.d" "drivers\oled_data.d" "drivers\tb6612.d" "empty_LP_MSPM0G3507_nortos_ticlang\Debug\ti_msp_dl_config.d" "ti_msp_dl_config.d" "startup_mspm0g350x_ticlang.d" 
	-@echo ' '

secondary-outputs: $(BIN_OUTPUTS)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

