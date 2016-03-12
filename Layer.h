
class Layer
{
  int inputs;
  int outputs;
  float * _biases;
  float * _weights;
  ~Layer();
  //return a pointer to the array of activations
  //this pointer is only good until the next time someone tries to activate the network
  //so copy the data if needed elsewhere
  float * activate(float * input_arr);
  //weights should be inputs rows, outputs cols
  void Layer(inputs, outputs, float ** weights, float * biases);
  void setParams(float ** weights, float * biases);
  float activation_func(float out);

private:
  // this is so that we don't have to malloc space to store the output array each time
  float * output_arr;
};
