#ifndef reluLayer_h
#define reluLayer_h
#include "Layer.h"

class reluLayer : public Layer {
public:
  ~reluLayer();
  reluLayer()
  {
    
  }

  reluLayer(int inputs, int outputs, float ** weights, float * biases) : Layer(inputs, outputs, weights, biases)
  {

  }

  reluLayer(int inputs, int outputs, float * weights, float * biases) : Layer(inputs, outputs, weights, biases)
  {

  }

  float activation_func(float out);
};

#endif
