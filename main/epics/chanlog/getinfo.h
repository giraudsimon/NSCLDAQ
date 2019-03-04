#ifndef __GETINFO_H
#define __GETINFO_H

void EpicsInit(int argc, char** argv);
void EpicsShutdown();
void GetChannel(char* output, const char* ch);

#endif
