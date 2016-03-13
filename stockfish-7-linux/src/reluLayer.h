#ifndef reluLayer_h
#define reluLayer_h
#include "Layer.h"

class reluLayer : public Layer {
public:
  ~reluLayer();
  reluLayer()
  {

  }

  reluLayer(int ninputs, int noutputs, float ** weights, float * biases) : Layer(ninputs, noutputs, weights, biases)
  {

  }

  reluLayer(int ninputs, int noutputs, float * weights, float * biases) : Layer(ninputs, noutputs, weights, biases)
  {

  }

  float activation_func(float out);
};

#endif
