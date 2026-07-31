################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
bsp/%.o: ../bsp/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Arm Compiler - building file: "$<"'
	"C:/ti/ccs2100/ccs/tools/compiler/ti-cgt-armllvm_5.1.1.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/drivers" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/control" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/bsp" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/app" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang" -I"C:/Users/yang/Desktop/empty_LP_MSPM0G3507_nortos_ticlang/Debug" -I"C:/ti/mspm0_sdk_2_11_00_07/source/third_party/CMSIS/Core/Include" -I"C:/ti/mspm0_sdk_2_11_00_07/source" -g -Wall -MMD -MP -MF"bsp/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


