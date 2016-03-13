#include "KatyushaNet.h"


void KatyushaNet::load_layer(string layer_name, Layer* layer, cnpy::npz_t& archive)
{
  string wname = layer_name;
  wname += "_weights";
  cnpy::NpyArray weights = archive[wname];
  string bname = layer_name;
  bname += "_biases";
  cnpy::NpyArray biases = archive[bname];
  assert(weights.shape[1] == biases.shape[0]);
  assert(weights.word_size == sizeof(float));
  assert(biases.word_size == sizeof(float));

  layer->init_unitialized(weights.shape[0], weights.shape[1]);
  layer->setBiases((float*)biases.data);
  layer->weightsFromRotated((float*)weights.data);
  delete[] weights.data;
  delete[] biases.data;
}

void KatyushaNet::load(string archive_name)
{
  cnpy::npz_t weights_npz = cnpy::npz_load(archive_name);
  for (size_t i = 0; i < initial_layers.size(); i++)
  {
    load_layer(initial_layers[i].name, (Layer*)(initial_layers[i].layer), weights_npz);
  }

  load_layer("layer1", (Layer*)(&layer1), weights_npz);
  load_layer("outlayer", (Layer*)(&out), weights_npz);
}


float KatyushaNet::evaluate(int * pos_features)
{
  float fvec[TOTAL_FEATURES];
  for (int i = 0; i < TOTAL_FEATURES; i++)
  {
    fvec[i] = (float)pos_features[i];
  }

  float * f = fvec;

  float first_layer_out[layer1.inputs];

  int out_off = 0;
  int in_off = 0;
  for (size_t i = 0; i < initial_layers.size(); i++)
  {
    memcpy(first_layer_out+out_off, initial_layers[i].layer->activate(fvec+in_off), sizeof(float) * initial_layers[i].layer->outputs);
    out_off += initial_layers[i].layer->outputs;
    in_off += initial_layers[i].layer->inputs;
  }
  assert(in_off == TOTAL_FEATURES);
  assert(out_off == layer1.inputs);

  return out.activate(layer1.activate(first_layer_out))[0];
}

KatyushaNet::~KatyushaNet()
{
  for (size_t i = 0; i < initial_layers.size(); i++)
  {
    delete initial_layers[i].layer;
  }
}
