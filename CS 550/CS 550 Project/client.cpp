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


int decision(vector<string> cards, int risk)
{
  int sum = 0;
  for (int i =0; i < cards.size(); i++)
  {
    if (cards[i][0] == 'J' || cards[i][0] == 'Q' || cards[i][0] == 'K' || cards[i][0] == '1')
    {
      sum += 10;
    }
    else if (cards[i][0] == 'A')
    {
      if (sum + 11 > 21)
        sum += 1;
      else
        sum += 11;
    }
    else
    {
      sum += (cards[i][0] - 48);  //Converts char to int;
    }
  }

  if (sum < 17)
  {
    return 1; // Hit
  }
  else if (sum > 17)
  {
    //risk factor
  }

  return 0;
}

int main()
{
  string ip;
  int port;
  int peer_id = 0;
  int player_id = 0;
  int risk = rand() % 10 + 1;      //Gives a risk factor between 1 - 10.
  int patience = rand() % 100 + 1;  //Gives a patience factor between 1 - 100.
  int has_last_result = -1;
  int active = 1;
  //cout << "Select the IP address to connect to: ";
  //cin >> ip;
  //cout <<endl;

  //cout << "Select the port: ";
  //cin >> port;
  //cout <<endl;
  rpc::client client("127.0.0.1", 9000);
  peer_id = client.call("get_id").as<int>();
  //cout << peer_id << endl;
  player_id = client.call("player_join", peer_id).as<int>();
  //cout << player_id << endl;
  /*if (player_id == 0)
    client.call("player_quit", player_id);*/

  while(client.call("player_quit", player_id).as<int>() == 0)               //Peer Loop
  {
    sleep(2);
    //cout << client.call("check_game_status").as<int>() << has_last_result << " " << active <<endl;
    if (client.call("check_game_status").as<int>() == 0 && has_last_result == 0 && active == 0)
    {
      if (client.call("check_results", peer_id).as<int>() == -1)
        patience -= 5;


      has_last_result = 1;
      active = 1;
    }
    else if (client.call("check_game_status").as<int>() == 1 && active == 1)
    {
      while(client.call("check_game_status").as<int>() == 1 && active == 1)
      {
        has_last_result = 0;
        if (client.call("check_turn").as<int>() == player_id)
        {
          vector<string> cards = client.call("check", player_id).as<vector<string>>(); //Return the cards the client holds

          while (decision(cards, risk))                  //Ask client for decision
          {
            cout << "You hit ... ";
            cards = client.call("hit", player_id).as<vector<string>>();
            cout << "Current Hand: ";
            for (int i = 0; i < cards.size(); i++)
            {
              cout << cards[i] << " ";
            }
            cout << endl <<endl;

          }

          cout << "You stay ... " <<endl;
          active = 0;
          client.call("stay", player_id);
          /*cout << "Current Hand: ";
          for (int i = 0; i < cards.size(); i++)
          {
            cout << cards[i] << " ";
          }
          cout << endl <<endl;*/
        }
      }
    }
  }

  return 0;
}
