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



//A few globals needed because of RPC setup

int pcount = 0; //Keeps track of the number of peers.
int ready_check = 0;
int registered = 0; //Indicates that peer has registered its files.

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

/*void un_registry(int peer, vector<string> file_list)
{
  file_entry fe;

  for (int i = 0; i < file_list.size(); i++)
  {
    for (int j = 0; j < file_index.size(); j++)
    {
      if (file_index[j].name == file_list[i] && file_index[j].id == peer)
      {
        file_index.erase(file_index[j])
        j--;
      }
    }
  }

  cout << endl << "***NEW INDEX***" <<endl;
  for(int i = 0; i < file_index.size(); i++)
  {
    cout << file_index[i].name <<endl;
  }
  cout << "**************" <<endl <<endl;
}*/


//Searches for a file and returns the peers with the file
vector<int> search_file(int peer, string file_name)
{
  vector<int> matches = {};

  //Search vector for file
  for(int i = 0; i < file_index.size(); i++)
  {
    if (file_index[i].name == file_name)
      matches.push_back(file_index[i].id);
  }
  //Print matching entry peer ID #.
  if (matches.size() > 0)
  {
    cout <<"These peer ID(s) have " <<file_name <<":";
    for (int i=0; i <matches.size(); i++)
    {
      cout << " " << matches[i] << " ";
    }
    cout << endl << endl;
    return matches;
  }
  else
  {
    cout << "No peers have " << file_name << "!" <<endl <<endl;
  }

  return matches;
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
  return 9000 + search_file(peer, file_name)[0];

}

//Run val searches for testing purposes. (THIS SEARCH DELAYS TO WAIT FOR OTHER PEERS TO PREPARE SEARCHES)
int test_init()
{
  cout << "Tests will begin in 60 seconds:" <<endl;
  ready_check++;
  int timeout = 30;

  //Wait for all the peers to initiate the test search
  while (timeout > 0 && ready_check != pcount)
  {
    cout << timeout-- <<endl;
    sleep(1);
  }


  //cout << "Initiating Search Tests!" <<endl;

  return 0;
}

int test_search(int val, int peer)
{
    cout << "Performing Search Test." <<endl;
    int count = val;
    ifstream file_stream;
    string line_in;
    file_stream.open("1000_words");

    while (getline(file_stream, line_in) && count > 0)
    {
      search_file(peer,line_in);
      count--;
    }
    file_stream.close();

    ready_check--;


  return 0;
}


/*int test_reg(int val, int peer, vector<string> file_list)
{
    cout << "Performing Reg Test." <<endl;
    int count = val;

    while (count > 0)
    {
      registry(peer,file_list);
      count--;
    }
    file_stream.close();

    ready_check--;


  return 0;
}


int test_obtain(int val, int peer)
{
    cout << "Performing Search Test." <<endl;
    int count = val;
    ifstream file_stream;
    string line_in;
    file_stream.open("1000_words");

    while (getline(file_stream, line_in) && count > 0)
    {
      search_file(peer,line_in);
      count--;
    }
    file_stream.close();

    ready_check--;


  return 0;
}*/

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
  else if (input == "test_search" || input == "t")
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
  /*else if (input == "test_reg")
  {
    return 7;
  }
  else if (input == "test_obtain")
  {
    return 8;
  }*/
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

int get_rand(int port_count)
{
  srand(time(NULL));     //Seed the number generator
  //printf("Randomly generating number between 1 and %d \n", port_count);
  int randy = rand() % port_count + 1; //Returns number between 1 and total number of ports
  //printf("And the lucky winner is.... %d \n", randy);
  return randy;
}

void stop_server()
{
  cout << "This server has closed!" <<endl;
  exit(0);

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
    server.bind("test_init", &test_init);
    server.bind("test_search", &test_search);
    //server.bind("test_reg", &test_reg);
    //server.bind("test_obtain", &test_obtain);
    server.bind("get_id", &get_id);
    /*server.bind("exit", []() {
    rpc::this_session().post_exit(); // post exit to the queue
  });*/
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
    //cin >> ip;
    cout <<endl;
    ip = "127.0.0.1";
    //Distributed Method of binding client to server(s)
    int port_count = 0;
    string line;
    vector<int> serv_status = {0,0,0,0,0,0,0,0};

    //Reads config file for the amount of available servers
    ifstream input_file("config");
    while(getline(input_file, line))
    {
      if (line[0] != '/') //These would be commented lines
      {
        serv_status[port_count] = 1;  //Mark that a server is present
        port_count++;
        //cout << port_count << endl;
      }
    }


    //Binds to each of the available servers (Servers must be listening!)

    //Read lines from files to get the port values.
    rpc::client client(ip, 2000);
    rpc::client client2(ip, 2001);
    rpc::client client3(ip, 2002);
    rpc::client client4(ip, 2003);
    rpc::client client5(ip, 2004);
    rpc::client client6(ip, 2005);
    rpc::client client7(ip, 2006);
    rpc::client client8(ip, 2007);
    input_file.close();




    //get peer id (for simplicity, only the first server issues the ID)

    //Include if statements to check if the servers are even there!!

    peer_id = client.call("get_id").as<int>();
    if(serv_status[1] == 1)
    {
      client2.call("get_id");
    }

    if(serv_status[2] == 1)
    {
      client3.call("get_id");
    }

    if(serv_status[3] == 1)
    {
      client4.call("get_id");
    }

    if(serv_status[4] == 1)
    {
      client5.call("get_id");
    }

    if(serv_status[5] == 1)
    {
      client6.call("get_id");
    }

    if(serv_status[6] == 1)
    {
      client7.call("get_id");
    }

    if(serv_status[7] == 1)
    {
      client8.call("get_id");
    }
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
    int it;
    //Used to keep track of search result peers
    vector<int> search_list;

    //Used for timing the test durations
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    cout << "Connection Established!" <<endl <<endl;

    //Determines what call is sent to the server
    while (status == 1)
    {
      int rand_num = get_rand(port_count);
      cout << "Please enter a command (Use ? for help): ";
      cin >> input;
      int_in = convert_input(input);
      switch(int_in)
      {
        case 0:   //HELP CASE
          cout << "The following commands can be used: registry, search, obtain, exit, stop_server, test" <<endl;
          break;

        case 1:  //REGISTRY CASE
          cout <<endl;
          cout << rand_num <<endl;
          if(rand_num == 1)
          {
            client.call("registry",peer_id, get_files());
          }
          else if(rand_num == 2)
          {
            client2.call("registry",peer_id, get_files());
          }
          else if(rand_num == 3)
          {
            client3.call("registry",peer_id, get_files());
          }
          else if(rand_num == 4)
          {
            client4.call("registry",peer_id, get_files());
          }
          else if(rand_num == 5)
          {
            client5.call("registry",peer_id, get_files());
          }
          else if(rand_num == 6)
          {
            client6.call("registry",peer_id, get_files());
          }
          else if(rand_num == 7)
          {
            client7.call("registry",peer_id, get_files());
          }
          else if(rand_num == 8)
          {
            client8.call("registry",peer_id, get_files());
          }
          //Register a peer's files to a random server

          break;

        case 2: //SEARCH CASE
          cout << "Enter the file you are searching for: ";
          cin >>input;

          cout << endl << "These are the search results for " << input << ":" <<endl <<endl;
          if(serv_status[0] == 1)
          {

            cout << "(On Server 1): Peers = " ;

            search_list = client.call("search", peer_id, input).as<vector<int>>();
            if (search_list.size() > 0) //Take the list of search results and lists them
            {
              for (it = 0; it < search_list.size(); it++)
              {
                  cout << search_list[it] << " ";
              }
              cout <<endl;
            }

            else
              cout << "None." <<endl;
          }
          if(serv_status[1] == 1)
          {

            cout << "(On Server 2): Peers = " ;

            search_list = client2.call("search", peer_id, input).as<vector<int>>();
            if (search_list.size() > 0) //Take the list of search results and lists them
            {
              for (it = 0; it < search_list.size(); it++)
              {
                  cout << search_list[it] << " ";
              }
              cout <<endl;
            }

            else
              cout << "None." <<endl;
          }
          if(serv_status[2] == 1)
          {

            cout << "(On Server 3): Peers = " ;

            search_list = client3.call("search", peer_id, input).as<vector<int>>();
            if (search_list.size() > 0) //Take the list of search results and lists them
            {
              for (it = 0; it < search_list.size(); it++)
              {
                  cout << search_list[it] << " ";
              }
              cout <<endl;
            }

            else
              cout << "None." <<endl;
          }
          if(serv_status[3] == 1)
          {

            cout << "(On Server 4): Peers = " ;

            search_list = client4.call("search", peer_id, input).as<vector<int>>();
            if (search_list.size() > 0) //Take the list of search results and lists them
            {
              for (it = 0; it < search_list.size(); it++)
              {
                  cout << search_list[it] << " ";
              }
              cout <<endl;
            }

            else
              cout << "None." <<endl;
          }
          if(serv_status[4] == 1)
          {

            cout << "(On Server 5): Peers = " ;

            search_list = client5.call("search", peer_id, input).as<vector<int>>();
            if (search_list.size() > 0) //Take the list of search results and lists them
            {
              for (it = 0; it < search_list.size(); it++)
              {
                  cout << search_list[it] << " ";
              }
              cout <<endl;
            }

            else
              cout << "None." <<endl;
          }
          if(serv_status[5] == 1)
          {

            cout << "(On Server 6): Peers = " ;

            search_list = client6.call("search", peer_id, input).as<vector<int>>();
            if (search_list.size() > 0) //Take the list of search results and lists them
            {
              for (it = 0; it < search_list.size(); it++)
              {
                  cout << search_list[it] << " ";
              }
              cout <<endl;
            }

            else
              cout << "None." <<endl;
          }
          if(serv_status[6] == 1)
          {

            cout << "(On Server 7): Peers = " ;

            search_list = client7.call("search", peer_id, input).as<vector<int>>();
            if (search_list.size() > 0) //Take the list of search results and lists them
            {
              for (it = 0; it < search_list.size(); it++)
              {
                  cout << search_list[it] << " ";
              }
              cout <<endl;
            }

            else
              cout << "None." <<endl;
          }
          if(serv_status[7] == 1)
          {

            cout << "(On Server 8): Peers = " ;

            search_list = client8.call("search", peer_id, input).as<vector<int>>();
            if (search_list.size() > 0) //Take the list of search results and lists them
            {
              for (it = 0; it < search_list.size(); it++)
              {
                  cout << search_list[it] << " ";
              }
              cout <<endl;
            }

            else
              cout << "None." <<endl;
          }
          cout <<endl;
          break;

        case 3:  //FILE TRANSFER CASE
          cout << "Enter the file you are trying to obtain: ";
          cin >>input;
          //Cannot use the rpc::client code here due to the scope of the case statement.
          break; //Do file transfer outside of case statement.

        case 4: //TESTING CASE
          cout << "How many iterations?: ";
          cin >> int_in;
          //START TIME
          client.call("test_init");
          begin = std::chrono::steady_clock::now();
          client.call("test_search", int_in, peer_id);

          if (serv_status[1] == 1)
            client2.call("test_search", int_in, peer_id);

          if (serv_status[2] == 1)
            client3.call("test_search", int_in, peer_id);

          if (serv_status[3] == 1)
            client4.call("test_search", int_in, peer_id);

          if (serv_status[4] == 1)
            client5.call("test_search", int_in, peer_id);

          if (serv_status[5] == 1)
            client6.call("test_search", int_in, peer_id);

          if (serv_status[6] == 1)
            client7.call("test_search", int_in, peer_id);

          if (serv_status[7] == 1)
            client8.call("test_search", int_in, peer_id);

          end = std::chrono::steady_clock::now();
          std::cout << "The searches took this much time (in ms) = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() <<std::endl;
          break;

        case 5:  //QUIT CASE (FOR CLIENT)
          cout << "The client has disconnected." <<endl;
          return 0;
          break;

        case 6:    //CLOSE THE SERVER CASE
          client.call("stop_server");

          if (serv_status[1] == 1)
            client2.async_call("stop_server");

          if (serv_status[2] == 1)
            client3.async_call("stop_server");

          if (serv_status[3] == 1)
            client4.async_call("stop_server");

          if (serv_status[4] == 1)
            client5.async_call("stop_server");

          if (serv_status[5] == 1)
            client6.async_call("stop_server");

          if (serv_status[6] == 1)
            client7.async_call("stop_server");

          if (serv_status[7] == 1)
            client8.async_call("stop_server");
          break;

        /*case 7: //TESTING CASE (REG)
          cout << "How many iterations?: ";
          cin >> int_in;
          //START TIME
          client.call("test_init");
          begin = std::chrono::steady_clock::now();

          if(rand_num == 1)
          {
            client.call("test_reg", int_in, peer_id, get_files());
          }
          else if(rand_num == 2)
          {
            client2.call("test_reg", int_in, peer_id, get_files());
          }
          else if(rand_num == 3)
          {
            client3.call("test_reg", int_in, peer_id, get_files());
          }
          else if(rand_num == 4)
          {
              client4.call("test_reg", int_in, peer_id, get_files());
            }
          else if(rand_num == 5)
          {
              client5.call("test_reg", int_in, peer_id, get_files());
          }
          else if(rand_num == 6)
          {
              client6.call("test_reg", int_in, peer_id, get_files());
          }
          else if(rand_num == 7)
          {
              client7.call("test_reg", int_in, peer_id, get_files());
          }
          else if(rand_num == 8)
          {
              client8.call("test_reg", int_in, peer_id, get_files());
          }

          end = std::chrono::steady_clock::now();
          std::cout << "The registrations took this much time (in ms) = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() <<std::endl;
          break;

        case 8: //TESTING CASE (OBTAIN)
          cout << "How many iterations?: ";
          cin >> int_in;
          //START TIME
          client.call("test_init");
          begin = std::chrono::steady_clock::now();
          client.call("test_search", int_in, peer_id);

          if (serv_status[1] == 1)
            client2.call("test_search", int_in, peer_id);

          if (serv_status[2] == 1)
            client3.call("test_search", int_in, peer_id);

          if (serv_status[3] == 1)
            client4.call("test_search", int_in, peer_id);

          if (serv_status[4] == 1)
            client5.call("test_search", int_in, peer_id);

          if (serv_status[5] == 1)
            client6.call("test_search", int_in, peer_id);

          if (serv_status[6] == 1)
            client7.call("test_search", int_in, peer_id);

          if (serv_status[7] == 1)
            client8.call("test_search", int_in, peer_id);

          end = std::chrono::steady_clock::now();
          std::cout << "The searches took this much time (in ms) = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() <<std::endl;
          break;*/

        default:
          cout << "Please enter a valid command." <<endl;
      }

      //Copies the file for the peer who requests it
      if (int_in == 3)
      {
        //INSERT FOR LOOP!
        vector<string> file_content;
        ofstream output_file (input);
        int continue_search = 0;
        int target_ID = 0;

        //Obtain file from first server with matching file)
        if (serv_status[0] == 1)
        {
          target_ID = client.call("obtain", peer_id, input).as<int>();
          if (target_ID == 0)
            continue_search = 1;
          else
            continue_search = 0;
        }

        if (serv_status[1] == 1 && continue_search == 1)
        {
          target_ID = client2.call("obtain", peer_id, input).as<int>();
          if (target_ID == 0)
            continue_search = 1;
          else
            continue_search = 0;
        }

        if (serv_status[2] == 1 && continue_search == 1)
        {
          target_ID = client3.call("obtain", peer_id, input).as<int>();
          if (target_ID == 0)
            continue_search = 1;
          else
            continue_search = 0;
        }

        if (serv_status[3] == 1 && continue_search == 1)
        {
          target_ID = client4.call("obtain", peer_id, input).as<int>();
          if (target_ID == 0)
            continue_search = 1;
          else
            continue_search = 0;
        }
        if (serv_status[4] == 1 && continue_search == 1)
        {
          target_ID = client5.call("obtain", peer_id, input).as<int>();
          if (target_ID == 0)
            continue_search = 1;
          else
            continue_search = 0;
        }

        if (serv_status[5] == 1 && continue_search == 1)
        {
          target_ID = client6.call("obtain", peer_id, input).as<int>();
          if (target_ID == 0)
            continue_search = 1;
          else
            continue_search = 0;
        }

        if (serv_status[6] == 1 && continue_search == 1)
        {
          target_ID = client7.call("obtain", peer_id, input).as<int>();
          if (target_ID == 0)
            continue_search = 1;
          else
            continue_search = 0;
        }

        if (serv_status[7] == 1 && continue_search == 1)
        {
          target_ID = client8.call("obtain", peer_id, input).as<int>();
          if (target_ID == 0)
            continue_search = 1;
          else
            continue_search = 0;
        }

        rpc::client pclient("127.0.0.1", target_ID);
        file_content = pclient.call("copy_file",input).as<vector<string>>();
        cout <<endl <<endl;
        for (int i=0; i < file_content.size(); i++)
        {
          output_file << file_content[i];

          if (i < 10)
            cout << file_content[i] <<endl;

          if (i == 11)
            cout << "..........." <<endl
          <<endl;

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
