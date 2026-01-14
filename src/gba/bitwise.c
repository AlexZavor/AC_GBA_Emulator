#include "gba/bitwise.h"

inline void swp(void* d1, void* d2){
  uint32_t tmp = *(uint32_t*)d1;
  *(uint32_t*)d1 = *(uint32_t*)d2;
  *(uint32_t*)d2 = tmp;
}

// Get/Set bit function
inline void set_bit(uint32_t* data, uint8_t bit, bool set){
  if(set){
    *data |= (((1)<<bit));
  }else{
    *data &= ~(((1)<<bit));
  }
}
inline bool get_bit(uint32_t data, uint8_t bit){
  return (data>>bit)&0x1;
}

int32_t sign_extend(uint32_t data, uint8_t original_bit_width){
  if((data>>(original_bit_width-1))&0x1){
    //fill negative FF
    int32_t data_s = data;
    int x = original_bit_width;
    while(x < 32){
      data_s |= (1<<x);
      x++;
    }
    return data_s;
  }else{
    return data;
  }
}
int64_t sign_extend64(uint64_t data, uint8_t original_bit_width){
  if((data>>(original_bit_width-1))&0x1){
    //fill negative FF
    int64_t data_s = data;
    uint64_t x = original_bit_width;
    while(x < 64){
      data_s |= (((uint64_t)1)<<x);
      x++;
    }
    return data_s;
  }else{
    return data;
  }
}

inline int high_count(uint32_t data){
  int count = 0;
  for(int i=0; i < 32; i++){
    if((data>>i)&0x1){
      count++;
    }
  }
  return count;
}