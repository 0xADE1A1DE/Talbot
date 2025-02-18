all: trigger query

query:
	cd kernel_space/; $(MAKE) clean; $(MAKE)

trigger:
	cd user_space/; $(MAKE) clean; $(MAKE); cp trigger $(PWD)

.PHONY: query trigger clean

clean:
	rm -f trigger;
	cd kernel_space/; $(MAKE) clean;
	cd user_space/; $(MAKE) clean

run:
	sudo ./trigger;
	sudo dmesg | tail

destroy:
	sudo rmmod query 2>/dev/null || true

ready:
	sudo insmod kernel_space/query.ko

test:
	$(MAKE) trigger;
	$(MAKE) query;
	$(MAKE) destroy;
	$(MAKE) ready;
	$(MAKE) run;
