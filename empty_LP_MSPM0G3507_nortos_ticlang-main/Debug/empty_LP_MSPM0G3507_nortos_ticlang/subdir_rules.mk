################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-1186143241: ../empty_LP_MSPM0G3507_nortos_ticlang/empty.syscfg
	@echo 'SysConfig - building file: "$<"'
	"C:/ti/sysconfig_1.26.2/sysconfig_cli.bat" -s "C:/ti/mspm0_sdk_2_11_00_07/.metadata/product.json" --script "C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/empty_LP_MSPM0G3507_nortos_ticlang/empty.syscfg" -o "." --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-1186143241 ../empty_LP_MSPM0G3507_nortos_ticlang/empty.syscfg
device.opt: build-1186143241
device.cmd.genlibs: build-1186143241
ti_msp_dl_config.c: build-1186143241
ti_msp_dl_config.h: build-1186143241
Event.dot: build-1186143241


