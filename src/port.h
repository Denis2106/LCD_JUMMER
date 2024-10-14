#ifndef _H_PORT
#define _H_PORT

#include <stdint.h>

class Port {
  public:
    virtual bool is_available();
    virtual uint8_t read();
    virtual void write(uint8_t data);
};

#endif