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

using namespace std;



//A few globals needed because of RPC setup

int pcount = 0; //Keeps track of the number of peers.

struct file_entry  //An entry in the indexing server file index
{
  string name;
  int id;
};

vector<file_entry> file_index;

//Does the registry RPC call
void registry(int peer, vector<string> file_list)
{
  file_entry fe;

  for (int i = 0; i < file_list.size(); i++)
  {
    fe.name = file_list[i];
    fe.id = peer;
    cout << "Updating file index, adding: " << fe.name << endl;
    file_index.push_back(fe);
  }

  cout << endl << "***NEW INDEX***" <<endl;
  for(int i = 0; i < file_index.size(); i++)
  {
    cout << file_index[i].name <<endl;
  }
  cout << "**************" <<endl <<endl;



}


//Searches for a file and returns the peers with the file
int search_file(int peer, string file_name)
{
  vector<int> matches;

  //Search vector for file
  for(int i = 0; i < file_index.size(); i++)
  {
    if (file_index[i].name == file_name)
      matches.push_back(file_index[i].id);
  }

  //Print matching entry peer ID #.
  if (matches.size() > 0)
  {
    cout <<"The peer ID(s) with " <<file_name << ": ";
    for (int i=0; i <matches.size(); i++)
    {
      cout <<"Peer "<< matches[i];
    }
    cout <<endl <<endl;
    return matches[0];
  }
  else
  {
    cout << "No matches found!" <<endl <<endl;
  }

  return 0;
}

//Returns a string vector for file copying
vector<string> copy_file(string input)
{
  vector<string> file_content;
  ifstream file_stream;
  string line_in;
  file_stream.open(input);
  while (getline(file_stream, line_in))
  {
    //cout << line_in <<endl;
    file_content.push_back(line_in);
  }
  file_stream.close();

  return file_content;
}

//Gives a peer the port of another peer
int obtain_peer(int peer, string file_name)
{
  return 9000 + search_file(peer, file_name);

}

//Run val searches for testing purposes.
int test_search(int val)
{
  int count = val;
  ifstream file_stream;
  string line_in;
  file_stream.open("1000_words");
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  while (getline(file_stream, line_in) && count > 0)
  {
    search_file(1,line_in);
    count--;
  }
  file_stream.close();

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "The searches took this much time (in ms) = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() <<std::endl;
  return 0;
}

//Used to select the type of process
int type_select()
{
  int sel;
  cout << "Select operation mode (Server = 1, Peer = 2): ";
  cin >> sel;
  return sel;

}


int convert_input(string input)
{
  if (input == "?")
  {
    return 0;
  }
  else if (input == "r" || input == "registry")
  {
    return 1;
  }
  else if (input == "s" || input == "search")
  {
    return 2;
  }
  else if (input == "o" || input == "obtain")
  {
    return 3;
  }
  else if (input == "t" || input == "test")
  {
    return 4;
  }
  else if (input == "q" || input == "quit")
  {
    return 5;
  }
  else if (input == "c" || input == "close" || input == "close_server")
  {
    return 6;
  }
  else
  {
    return 9;
  }
}

//Gives a peer their ID
int get_id()
{
  cout << "Peer " <<++pcount <<" has connected!" <<endl;
  return pcount;
}

//Gets list of a peer's files
vector<string> get_files()
{
  vector<string> files;
  cout <<"Adding file information to indexing server..." <<endl;

  DIR *fdir = opendir("./");
  dirent *file_direc = readdir(fdir);

  while (file_direc = readdir(fdir))
  {
    if (strlen(file_direc->d_name) > 3) //Removes unnecessary extras
    {
      cout << "Updating file index, adding: " <<file_direc->d_name <<endl;
      files.push_back(file_direc->d_name);
    }

  }

  cout << "Operation complete." <<endl <<endl;
  return files;
}




int main()
{
  string ip;
  int port;
  int select = 0;

  int peer_id = 0;

  select = type_select();

  //Process for server
  if (select == 1)
  {
    cout << "This process will operate as the Server." <<endl <<endl;
    cout << "Select the port that will be used for listening: ";
    cin >> port;
    cout << endl;
    cout << "The server is now active." <<endl;

    //Set up server, bind each of the commands for peer
    rpc::server server(port);
    server.bind("registry", &registry);
    //server.bind("quit", &quit);
    server.bind("search", &search_file);
    server.bind("obtain", &obtain_peer);
    server.bind("test_search", &test_search);
    server.bind("get_id", &get_id);
    server.bind("exit", []() {
    rpc::this_session().post_exit(); // post exit to the queue
    });
    server.bind("stop_server", []() {
        rpc::this_server().stop();
    });

    server.run();
  }

  //Process for the peer
  else if (select == 2)
  {
    cout << "This process will operate as a Peer." <<endl <<endl;

    cout << "Select the IP address to connect to: ";
    cin >> ip;

    cout << "Select the port: ";
    cin >> port;

    rpc::client client(ip, port);

    //get peer id
    peer_id = client.call("get_id").as<int>();

    //Set up peer server
    int peer_port = peer_id + 9000;
    rpc::server pserver("127.0.0.1", peer_port);
    pserver.bind("copy_file", &copy_file);
    pserver.bind("exit", []() {
    rpc::this_session().post_exit(); // post exit to the queue
    });
    //Spawns thread to handle P2P operations and have it listen
    pserver.async_run(1);

    int status = 1;
    string input;
    int int_in;
    cout << "Connection Established!" <<endl <<endl;

    //Determines what call is sent to the server
    while (status == 1)
    {
      cout << "Please enter a command (Use ? for help): ";
      cin >> input;
      int_in = convert_input(input);
      switch(int_in)
      {
        case 0:
          cout << "The following commands can be used: registry, search, obtain, exit, stop_server, test" <<endl;
          break;

        case 1:
          cout <<endl;
          client.call("registry",peer_id, get_files());
          break;

        case 2:
          cout << "Enter the file you are searching for: ";
          cin >>input;
          cout << "Peer ID(s) with file: Peer with ID # " <<client.call("search", peer_id, input).as<int>() <<endl;
          break;

        case 3:
        cout << "Enter the file you are trying to obtain: ";
        cin >>input;
        //Cannot use the rpc::client code here due to the scope of the case statement.
        break; //Do file transfer outside of case statement.

        case 4:
          cout << "How many iterations?: ";
          cin >> int_in;
          client.call("test_search", int_in);
          break;

        case 5:
          client.call("exit", 1);
          break;

        case 6:
          client.call("stop_server", 1);
          break;

        default:
          cout << "Please enter a valid command." <<endl;
      }

      //Copies the file for the peer who requests it
      if (int_in == 3)
      {
        vector<string> file_content;
        ofstream output_file (input);

        rpc::client pclient("127.0.0.1", client.call("obtain", peer_id, input).as<int>());
        file_content = pclient.call("copy_file",input).as<vector<string>>();
        cout <<endl <<endl;
        for (int i=0; i < file_content.size(); i++)
        {
          output_file << file_content[i];

          if (i < 10)
            cout << file_content[i];

          if (i == 11)
            cout << "..........." <<endl <<endl;

        }
        cout << input << " has been copied." <<endl;
        output_file.close();          //close file stream
        pclient.call("exit");         //close session with other peer
      }

    }
  }
  else
  {
    cout << "Please select 1 for Server or 2 for Peer." <<endl;
  }


  cout << "The process has ended." <<endl <<endl;
  return 0;
}
