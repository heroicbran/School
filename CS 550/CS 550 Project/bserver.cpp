#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <list>
#include "rpc/client.h"
#include "rpc/server.h"
#include "rpc/this_session.h"
#include "rpc/this_server.h"
#include <dirent.h>
#include <chrono>
#include <unistd.h>
using namespace std;

void copy_file(vector<string> text)
{
  ofstream output("game_log_backup");
  for (int i=0; i < text.size(); i++)
  {
    output << text[i] <<endl;
  }
  output.close();

}


int main()
{
  rpc::server server(8000);
  server.bind("copy_file", &copy_file);
  server.async_run(1);

  while(1)
  {

  }
  return 0;
}
