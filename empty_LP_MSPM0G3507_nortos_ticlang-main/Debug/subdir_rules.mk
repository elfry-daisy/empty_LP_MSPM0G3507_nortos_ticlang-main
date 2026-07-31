################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/drivers" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/control" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/bsp" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/app" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/Debug" -I"C:/ti/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_11_00_07/source" -g -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1513806572: ../empty.syscfg
	@echo 'SysConfig - building file: "$<"'
	"C:/ti/sysconfig_1.26.2/sysconfig_cli.bat" -s "C:/ti/mspm0_sdk_2_11_00_07/.metadata/product.json" --script "C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/empty.syscfg" -o "." --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-1513806572 ../empty.syscfg
device.opt: build-1513806572
device.cmd.genlibs: build-1513806572
ti_msp_dl_config.c: build-1513806572
ti_msp_dl_config.h: build-1513806572
Event.dot: build-1513806572

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/drivers" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/control" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/bsp" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/app" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/Debug" -I"C:/ti/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_11_00_07/source" -g -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: C:/ti/mspm0_sdk_2_11_00_07/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/drivers" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/control" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/bsp" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/app" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/Debug" -I"C:/ti/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_11_00_07/source" -g -Wall -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


