BUILD ?= obj

all: talbot.ko

obj-m += talbot.o
talbot-objs += src/module.o
talbot-objs += src/paging.o
talbot-objs += src/experiments.o
talbot-objs += src/kernel_functions.o
talbot-objs += src/evaluation.o
talbot-objs += src/hyperthreaded.o
talbot-objs += src/cpu_instructions.o

ccflags-y += -I$(PWD)/include -I$(PWD)/../shared/include -std=gnu99

deps += $(talbot-objs:.o=.d)

-include $(deps)

talbot.ko:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

$(BUILD)/%.o: %.c
	@echo "CC $<"
	@mkdir -p "$(dir $@)"
	@${CC} -c $< -o $@ ${CFLAGS} -MT $@ -MMD -MP -MF $(@:.o=.d)

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	@rm -rf ${BUILD}
	@rm -f ../shared/src/.tlb.o.cmd
