#include "Layer.h"
#include <iostream>
using namespace std;

Layer::~Layer()
{
  if (_weights) free(_weights);
  _weights = 0;
  if (_biases) free(_biases);
  _biases = 0;
  if (output_arr) free(output_arr);
  output_arr = 0;
}

float Layer::activation_func(float out)
{
//  cout << "calling parent activation " << endl;
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
    int off = inputs*i;
    for (int j = 0; j < inputs; j++)
    {
      output_arr[i] += _weights[off+j]*input_arr[j];
    }

    output_arr[i] = activation_func(output_arr[i]);
  }
  return output_arr;
}

void Layer::init_unitialized(int layer_inputs, int layer_outputs)
{
  outputs = layer_outputs;
  inputs = layer_inputs;
  output_arr = (float*)malloc(sizeof(float)*layer_outputs);
  _biases = (float*)malloc(sizeof(float)*layer_outputs);
  _weights = (float*)malloc(sizeof(float)*layer_outputs*layer_inputs);
}

Layer::Layer(int layer_inputs, int layer_outputs, float ** weights, float * biases)
{
  init_unitialized(layer_inputs, layer_outputs);
  setParams(weights, biases);
}

Layer::Layer(int layer_inputs, int layer_outputs, float * weights, float * biases)
{
  init_unitialized(layer_inputs, layer_outputs);
  setBiases(biases);
  memcpy(_weights, weights, layer_inputs*layer_outputs*sizeof(float));
}

void Layer::setBiases(float * biases)
{
  memcpy(_biases, biases, outputs*sizeof(float));
}

//convert a weights matrix of the form where w_ij is the connection between the i-th input and the j-th ouput
void Layer::weightsFromRotated(float * rotWeights)
{
  for (int i = 0; i < outputs; i++)
  {
    for (int j = 0; j < inputs; j++)
    {
      _weights[inputs*i + j] = rotWeights[outputs*j + i];
    }
  }
}

void Layer::setParams(float ** weights, float * biases)
{
  setBiases(biases);
  for (int i = 0; i < outputs; i++)
  {
    memcpy(_weights+i*inputs, weights[i], inputs*sizeof(float));
  }
}
