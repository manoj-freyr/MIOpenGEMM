#include "floattostring.hpp"
#include <string>

namespace floattostring{

std::string float_string_type(double x){
  x += 1; /* Keeping -Wunused-parameter warning quiet */ 
  return "double";
}

std::string float_string_type(float x){
  x += 1; /* Keeping -Wunused-parameter warning quiet */ 
  return "float";
}

char float_char_type(double x){
  x += 1; /* Keeping -Wunused-parameter warning quiet */ 
  return 'd';
}

char float_char_type(float x){
  x += 1; /* Keeping -Wunused-parameter warning quiet */ 
  return 'f';
}



std::string get_float_string(char floattype){
  if (floattype == 'f'){
    return "float";
  }
  else{
    return "double";
  }
}

}