#include <cores.h>

#include <definitions.h>

#include <stdio.h>
#include <unistd.h>

#define BUF_LENGTH (100)

// THE FOLLOWING FUNCTIONS ARE COPIED FROM TLB;dr

int number_of_cores = 0;

/*
	Disables a given logical core.
*/
void disable_core(unsigned int core){
	FILE *fp;
	char buf[BUF_LENGTH];

	snprintf(buf, BUF_LENGTH, "/sys/devices/system/cpu/cpu%d/online", core);
	fp = fopen(buf, "w");
	fprintf(fp, "0");
	fclose(fp);
}

/*
	Enables a given logical core.
*/
void enable_core(unsigned int core){
	FILE *fp;
	char buf[BUF_LENGTH];

	snprintf(buf, BUF_LENGTH, "/sys/devices/system/cpu/cpu%d/online", core);
	fp = fopen(buf, "w");
	fprintf(fp, "1");
	fclose(fp);
}

/*
	Returns the physical core of the given logical core.
*/
int get_phys_core(unsigned int core){
	FILE *fp;
	unsigned int phys_core;
	char buf[BUF_LENGTH];

	snprintf(buf, BUF_LENGTH, "/sys/devices/system/cpu/cpu%d/topology/core_id", core);
	fp = fopen(buf, "r");
	fscanf(fp, "%d,", &phys_core);
	fclose(fp);

	return phys_core;
}

/*
	Enables all cores.
*/
void enable_all_cores(){ 
	int core;
	
	for(core = 1; core < number_of_cores; core++){
		enable_core(core);
	}
}

/*
	Finds the numbre of cores and stores this in 
	the variable 'number_of_cores.
*/
void set_number_of_cores(){
	FILE *fp;
	char buf[BUF_LENGTH];
	int core = 1;

	while(1){ 
		snprintf(buf, BUF_LENGTH, "/sys/devices/system/cpu/cpu%d/online", core);
		if(access(buf, F_OK ) == -1) {
			number_of_cores = core;
			break;
		}

		core++;
	}
}

/*
	Returns the co-resident logical core of the given logical core.
*/
int get_co_resident(unsigned long co_core){
	FILE *fp;
	char buf[BUF_LENGTH];
	int needed_phys_core;
	int core;

	snprintf(buf, BUF_LENGTH, "/sys/devices/system/cpu/cpu%d/topology/core_id", co_core);
	fp = fopen(buf, "r");
	fscanf(fp, "%d,", &needed_phys_core);
	fclose(fp);

	for(core = 0; core < number_of_cores; core++){
		if(core == PINNED_CORE){
			continue;
		}

		int phys_core = get_phys_core(core);

		if(phys_core == needed_phys_core){
			return core;
		}
	}


	return -1;
}