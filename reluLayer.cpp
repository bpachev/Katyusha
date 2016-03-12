#include "reluLayer.h"
#include "string.h"
#include "stdlib.h"

reluLayer::~reluLayer()
{
  free(_weights);
  _weights = 0;
  free(_biases);
  _biases = 0;
  free(output_arr);
  output_arr = 0;
}

//return a pointer to the array of activations
//this pointer is only good untill the next time someone tries to activate the network
//so copy the data if needed elsewhere 
float * reluLayer::activate(float * input_arr)
{
  for (int i = 0; i < outputs; i++)
  {
    output_arr[i] = _biases[i];
    int off = outputs*i;
    for (int j = 0; j < inputs; j++)
    {
      output_arr[j] += _weights[off+j]*input_arr[j];
    }
    //this is the activation part that probably should be abstracted
    // whoo-hoo RELU
    if (output_arr[j] < 0) output_arr[j] = 0;
  }
  return output_arr;   
}

void reluLayer::reluLayer(inputs, outputs, float ** weights, float * biases)
{
  output_arr = (float*)malloc(sizeof(float)*outputs);
  _biases = (float*)malloc(sizeof(float)*outputs);
  _weights = (float*)malloc(sizeof(float)*outputs*inputs);
  setParams(weights, biases);

}

void reluLayer::setParams(float ** weights, float * biases)
{
  memcpy(_biases, biases, outputs*sizeof(float));
  for (int i = 0; i < outputs; i++)
  {
    memcpy(_weights+i*inputs, weights[i], inputs*sizeof(float));
  }  
}

