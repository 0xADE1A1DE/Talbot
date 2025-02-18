all: trigger talbot

talbot:
	cd kernel_space/; $(MAKE) clean; $(MAKE)

trigger:
	cd user_space/; $(MAKE) clean; $(MAKE); cp trigger $(PWD)

.PHONY: talbot trigger clean

clean:
	rm -f trigger;
	cd kernel_space/; $(MAKE) clean;
	cd user_space/; $(MAKE) clean

run:
	sudo ./trigger

destroy:
	sudo rmmod talbot 2>/dev/null || true

ready:
	sudo insmod kernel_space/talbot.ko

test:
	$(MAKE) trigger;
	$(MAKE) talbot;
	$(MAKE) destroy;
	$(MAKE) ready;
	$(MAKE) run;
