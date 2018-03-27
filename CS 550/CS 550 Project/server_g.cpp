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

int game_status = 0;  //0 = waiting to start, 1 = ongoing
int peer_count = 0;
int player_count = 0;
int turn = 0;  //0 = dealer, 1 - 4 = player turn, -1 = game over
vector<int> player_status = {0, 0 ,0 ,0 ,0}; //0 = pending, -1 = lose, 1 = win.
vector<int> last_pstatus = {0, 0, 0, 0, 0};

vector<string> deck = {"2D", "2H", "2S", "2C", "3D", "3H", "3S", "3C", "4D", "4H", "4S", "4C", "5D",
               "5H", "5S", "5C", "6D", "6H", "6S", "6C", "7D", "7H", "7S", "7C", "8D", "8H", "8S", "8C", "9D", "9H", "9S", "9C",
                "10D", "10H", "10S", "10C", "JD", "JH", "JS", "JC", "QD", "QH", "QS", "QC", "KD", "KH", "KS", "KC", "AD" "AH" "AS" "AC"};


void initialize_graphics()
{
  //Intialize graphical stuff from SDL
}


void initialize_file()
{

}


int get_id()
{
  cout << "Peer " <<++peer_count <<" has connected!" <<endl;
  return peer_count;
}


vector<string> begin_game()
{
  vector<string> new_deck = deck;                   //Prepares a new deck
  random_shuffle(new_deck.begin(), new_deck.end()); //Shuffles the new deck.
  //Wait for at least 1 player.
  //Wait for more?
  return new_deck;
}

void game_flow()
{

}


int check_game_status()
{
  return game_status;
}

int check_turn()
{
  return turn;
}


int check_results(int peer)
{
  return turn;
}


int player_join(int peer)
{
  if (player_count < 4)
  {
    return ++player_count;
  }
  else
  {
    return 0;
  }
}

void player_quit(int peer)
{
  cout << "Peer " << peer << " has quit!" << endl;
  --player_count;
}


void check(int peer)
{
  //Give the peer the file so that they may check it.

  //Include Blackjack case
}


string hit(int peer)
{
  //Give the peer the next card on the stack.
  return "You hit and get a ___"
}


string stay(int peer)
{
  //End the peer's play
  return "You stay";
}

void resolve_round()
{
  //Dealer gets his cards and compares them with the players
}


int main()
{
  string ip;
  int port;
  int select = 0;
  cout << "This process will operate as the Server." <<endl <<endl;
  cout << "Select the port that will be used for listening: ";
  cin >> port;
  cout << endl;
  cout << "The server is now active. Waiting for players..." <<endl;
  //initialize_file();
  //Set up server, bind each of the commands for peer
  rpc::server server(port);
  server.bind("get_id", &get_id);
  server.bind("check", &check);
  server.bind("hit", &hit);
  server.bind("stay", &stay);
  server.bind("check_game_status", &check_game_status);
    server.bind("check_results", &check_results);
  server.bind("check_turn", &check_turn);
  server.bind("player_join", &player_join);
  server.bind("player_quit", &player_quit);
  server.async_run(4);

  return 0;
}
