#ifndef Layer_h
#define Layer_h
#include "string.h"
#include "stdlib.h"

class Layer
{
public:
  int inputs;
  int outputs;
  float * _biases;
  float * _weights;
  ~Layer();

  Layer()
  {
    output_arr = 0;
    _biases = 0;
    _weights  = 0;
  }

  //return a pointer to the array of activations
  //this pointer is only good until the next time someone tries to activate the network
  //so copy the data if needed elsewhere
  float * activate(float * input_arr);
  //weights should be inputs cols, outputs rows
  Layer(int layer_inputs, int layer_outputs, float ** weights, float * biases);
  //This contructor assumes weights is stored contingously in row-major order
  Layer(int layer_inputs, int layer_outputs, float * weights, float * biases);
  void init_unitialized(int layer_inputs, int layer_outputs);

  void setBiases(float * biases);
  void setParams(float ** weights, float * biases);
  void weightsFromRotated(float * rotWeights);
  virtual float activation_func(float out);

private:
  // this is so that we don't have to malloc space to store the output array each time
  float * output_arr;
};

#endif
