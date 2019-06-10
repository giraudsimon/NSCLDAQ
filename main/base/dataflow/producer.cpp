#include <string>
#include <CRingBuffer.h>

using namespace std;

int main(int argc, char** argv)
{
  string ringname("timing");

  CRingBuffer ring(ringname, CRingBuffer::producer);
  ring.setPollInterval(1);

  char buffer[1024*1024];
  for (int i =0; i < sizeof(buffer); i++) {
    buffer[i] = i;
  }

  for (int i =0; i < 10000; i++) {
    ring.put(buffer, sizeof(buffer));
  }
}
void* gpTCLApplication(0);