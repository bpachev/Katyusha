#ifndef tanhLayer_h
#define tanhLayer_h
#include "math.h"
#include "Layer.h"

class tanhLayer : public Layer {
public:
  ~tanhLayer();
  tanhLayer()
  {

  }

  tanhLayer(int ninputs, int noutputs, float ** weights, float * biases) : Layer(ninputs, noutputs, weights, biases)
  {

  }

  tanhLayer(int ninputs, int noutputs, float * weights, float * biases) : Layer(ninputs, noutputs, weights, biases)
  {

  }

  float activation_func(float out);
};


#endif
