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

  tanhLayer(int inputs, int outputs, float ** weights, float * biases) : Layer(inputs, outputs, weights, biases)
  {

  }

  tanhLayer(int inputs, int outputs, float * weights, float * biases) : Layer(inputs, outputs, weights, biases)
  {

  }

  float activation_func(float out);
};


#endif
