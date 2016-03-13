#include "reluLayer.h"
#include "string.h"
#include "stdlib.h"

float reluLayer::activation_func(float out)
{
  return (out > 0) ? out : 0;
}

reluLayer::~reluLayer()
{
  
}
