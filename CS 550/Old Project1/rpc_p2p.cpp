
#include <stdio>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "rpc/client.h"

using namespace std;

int type_select()
{
  int sel;
  cout << "Select operation mode (Server = 1, Peer = 2): ";
  cin >> sel;
  return sel;

}
