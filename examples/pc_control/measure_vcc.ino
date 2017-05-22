#include <Vcc.h>

Vcc vcc(1.0);

int read_vcc() {
  return vcc.Read_Volts()*1000;
}

