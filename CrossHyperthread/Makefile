all: trigger trcrt

trcrt:
	cd kernel_space/; $(MAKE) clean; $(MAKE)

trigger:
	cd user_space/; $(MAKE) clean; $(MAKE); cp trigger $(PWD)

.PHONY: trcrt trigger clean

clean:
	rm -f trigger;
	cd kernel_space/; $(MAKE) clean;
	cd user_space/; $(MAKE) clean

run:
	sudo ./trigger

destroy:
	sudo rmmod trcrt 2>/dev/null || true

ready:
	sudo insmod kernel_space/trcrt.ko

test:
	$(MAKE) trigger;
	$(MAKE) trcrt;
	$(MAKE) destroy;
	$(MAKE) ready;
	$(MAKE) run;
