BUILD ?= obj

all: talbot.ko

obj-m += talbot.o
talbot-objs += src/module.o
talbot-objs += src/paging.o
talbot-objs += src/eviction_sets.o
talbot-objs += src/hash_functions.o
talbot-objs += src/experiments.o
talbot-objs += src/kernel_functions.o
talbot-objs += src/pointer_chasing.o
talbot-objs += src/replacement.o
talbot-objs += src/evaluation.o
talbot-objs += src/cpu_instructions.o
talbot-objs += ../shared/src/tlb.o

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
