#include "reluLayer.h"
#include "string.h"
#include "stdlib.h"
#include <iostream>
using namespace std;

float reluLayer::activation_func(float out)
{
//  cout << "calling child activation" << endl;
  return (out > 0) ? out : 0;
}

reluLayer::~reluLayer()
{

}
