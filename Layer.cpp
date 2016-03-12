#include "Layer.h"

Layer::~Layer()
{
  free(_weights);
  _weights = 0;
  free(_biases);
  _biases = 0;
  free(output_arr);
  output_arr = 0;
}

float Layer::activation_func(float out)
{
  return out;
}


//return a pointer to the array of activations
//this pointer is only good untill the next time someone tries to activate the network
//so copy the data if needed elsewhere
float * Layer::activate(float * input_arr)
{
  for (int i = 0; i < outputs; i++)
  {
    output_arr[i] = _biases[i];
    int off = outputs*i;
    for (int j = 0; j < inputs; j++)
    {
      output_arr[j] += _weights[off+j]*input_arr[j];
    }

    output_arr[j] = activation_func(output_arr[j]);
  }
  return output_arr;
}

void Layer::Layer(inputs, outputs, float ** weights, float * biases)
{
  output_arr = (float*)malloc(sizeof(float)*outputs);
  _biases = (float*)malloc(sizeof(float)*outputs);
  _weights = (float*)malloc(sizeof(float)*outputs*inputs);
  setParams(weights, biases);

}

void Layer::setParams(float ** weights, float * biases)
{
  memcpy(_biases, biases, outputs*sizeof(float));
  for (int i = 0; i < outputs; i++)
  {
    memcpy(_weights+i*inputs, weights[i], inputs*sizeof(float));
  }
}
