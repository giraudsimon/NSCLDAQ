#ifndef GETINFO_H
#define GETINFO_H

void EpicsInit(int argc, char** argv);
void EpicsShutdown();
void GetChannel(char* output, const char* ch);

#endif
