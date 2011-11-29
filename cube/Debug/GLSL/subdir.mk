################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
VERT_SRCS += \
../GLSL/toon.vert 

GEOM_SRCS += \
../GLSL/toon.geom 

FRAG_SRCS += \
../GLSL/toon.frag 

OBJS += \
./GLSL/toon.o 

GEOM_DEPS += \
./GLSL/toon.d 

VERT_DEPS += \
./GLSL/toon.d 

FRAG_DEPS += \
./GLSL/toon.d 


# Each subdirectory must supply rules for building sources it contributes
GLSL/%.o: ../GLSL/%.frag
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

GLSL/%.o: ../GLSL/%.geom
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

GLSL/%.o: ../GLSL/%.vert
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


