#ifndef CORES_H
#define CORES_H

extern int number_of_cores;

void disable_core(unsigned int core);
void enable_core(unsigned int core);
int get_phys_core(unsigned int core);
void enable_all_cores(void);
void set_number_of_cores(void);
int get_co_resident(unsigned long co_core);

#endif