BUILD ?= obj

all: query.ko

obj-m += query.o
query-objs += src/module.o
query-objs += src/paging.o
query-objs += src/hash_functions.o
query-objs += src/kernel_functions.o
query-objs += src/cpu_instructions.o
query-objs += src/query.o

ccflags-y += -I$(PWD)/include -I$(PWD)/../shared/include -std=gnu99

deps += $(query-objs:.o=.d)

-include $(deps)

query.ko:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

$(BUILD)/%.o: %.c
	@echo "CC $<"
	@mkdir -p "$(dir $@)"
	@${CC} -c $< -o $@ ${CFLAGS} -MT $@ -MMD -MP -MF $(@:.o=.d)

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	@rm -rf ${BUILD}
	@rm -f ../shared/src/.tlb.o.cmd
