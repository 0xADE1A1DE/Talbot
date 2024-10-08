BUILD ?= obj

all: trcrt.ko

obj-m += trcrt.o
trcrt-objs += src/module.o
trcrt-objs += src/paging.o
trcrt-objs += src/hash_functions.o
trcrt-objs += src/kernel_functions.o
trcrt-objs += src/cpu_instructions.o
trcrt-objs += src/query.o

ccflags-y += -I$(PWD)/include -I$(PWD)/../shared/include -std=gnu99

deps += $(trcrt-objs:.o=.d)

-include $(deps)

trcrt.ko:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

$(BUILD)/%.o: %.c
	@echo "CC $<"
	@mkdir -p "$(dir $@)"
	@${CC} -c $< -o $@ ${CFLAGS} -MT $@ -MMD -MP -MF $(@:.o=.d)

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	@rm -rf ${BUILD}
	@rm -f ../shared/src/.tlb.o.cmd
