#include "tanhLayer.h"

float tanhLayer::activation_func(float out)
{
  return tanh(out);
}

tanhLayer::~tanhLayer()
{

}
