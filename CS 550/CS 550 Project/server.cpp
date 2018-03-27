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
int max_count = 4;
int turn = 0;  //0 = dealer, 1 - 4 = player turn, -1 = game over
int turn_done = 0;
int g_shutdown = 0;
int locker = 0;


vector<int> player_status = {0, 0 ,0 ,0 ,0}; //0 = pending, -1 = lose, 1 = win, 2 = done/waiting.
vector<int> last_pstatus = {0, 0, 0, 0, 0};
vector<string> deck = {"2D", "2H", "2S", "2C", "3D", "3H", "3S", "3C", "4D", "4H", "4S", "4C", "5D",
               "5H", "5S", "5C", "6D", "6H", "6S", "6C", "7D", "7H", "7S", "7C", "8D", "8H", "8S", "8C", "9D", "9H", "9S", "9C",
                "1D", "1H", "1S", "1C", "JD", "JH", "JS", "JC", "QD", "QH", "QS", "QC", "KD", "KH", "KS", "KC", "AD", "AH", "AS", "AC"};   //Note: 1D/H/S/C are the "10" cards
vector<string> new_deck;
vector<string> dealer;
vector<string> player1;
vector<string> player2;
vector<string> player3;
vector<string> player4;
vector<string>text_file;
rpc::client client("127.0.0.1", 8000); //For communication with other server.

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


void begin_game()
{
  new_deck = deck;                   //Prepares a new deck
  //Wait for at least 1 player.
  while(player_count < max_count)                   //Wait for full game.
  {
    //Stall
  }
  random_shuffle(new_deck.begin(), new_deck.end()); //Shuffles the new deck
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
  return last_pstatus[peer+1];
}


int player_join(int peer)
{
  if (player_count < max_count)
  {
    return ++player_count;
  }
  else
  {
    return 0;
  }
}

int player_quit(int peer)
{
  while (locker > 0)
  {
  }

  if (g_shutdown == 1)
  {
    locker += 1;
    //cout << "Player " << peer << " has quit!" << endl;
    --peer_count;
    locker -= 1;
    //--player_count;
    return 1;
  }
//  cout << "Peer " << peer << " DONE NADA!" << endl;

  return 0;
}

void update_file(ofstream &output_file)
{
  vector<string> file_out;
  output_file << "D" << endl;
  file_out.push_back("D");
  output_file << dealer.size() << endl;
  file_out.push_back(to_string(dealer.size()));
  for (int i = 0; i < dealer.size(); i++)
  {
    output_file << dealer[i] << endl;
    file_out.push_back(dealer[i]);
  }

  output_file << "P1" << endl;
  file_out.push_back("P1");
  output_file << player1.size() << endl;
  file_out.push_back(to_string(player1.size()));
  for (int i = 0; i < player1.size(); i++)
  {
    output_file << player1[i] << endl;
    file_out.push_back(player1[i]);
  }

  output_file << "P2" << endl;
  file_out.push_back("P2");
  output_file << player2.size() << endl;
  file_out.push_back(to_string(player2.size()));
  for (int i = 0; i < player2.size(); i++)
  {
    output_file << player2[i] << endl;
    file_out.push_back(player2[i]);
  }

  output_file << "P3" << endl;
  file_out.push_back("P3");
  output_file << player3.size() << endl;
  file_out.push_back(to_string(player3.size()));
  for (int i = 0; i < player3.size(); i++)
  {
    output_file << player3[i] << endl;
    file_out.push_back(player3[i]);
  }

  output_file << "P4" << endl;
  file_out.push_back("P4");
  output_file << player4.size() << endl;
  file_out.push_back(to_string(player4.size()));
  for (int i = 0; i < player4.size(); i++)
  {
    output_file << player4[i] << endl;
    file_out.push_back(player4[i]);
  }

  client.call("copy_file", file_out);

}

vector<string> check(int peer)
{
    vector<string> hand;
    string line_in;
    int card_count;
    ifstream file_stream;
    file_stream.open("game_log");

    while (getline(file_stream, line_in))
    {
      if (line_in == "P" + to_string(peer))
      {
        getline(file_stream, line_in);
        card_count = stoi(line_in);
        for (int i=0; i < card_count; i++)
        {
          if (getline(file_stream, line_in));
            hand.push_back(line_in);
        }
      }

    }
    file_stream.close();
    return hand;
}


vector<string> hit(int peer)
{
  //Give the peer the next card on the stack.
  ofstream output_file("game_log");
  switch(peer)
  {
    case 1:
      player1.push_back(new_deck[0]);
      new_deck.erase(new_deck.begin());
      update_file(output_file);
      output_file.close();
      return player1;
      break;
    case 2:
      player2.push_back(new_deck[0]);
      new_deck.erase(new_deck.begin());
      update_file(output_file);
      output_file.close();
      return player2;
      break;

    case 3:
      player3.push_back(new_deck[0]);
      new_deck.erase(new_deck.begin());
      update_file(output_file);
      output_file.close();
      return player3;
      break;

    case 4:
      player4.push_back(new_deck[0]);
      new_deck.erase(new_deck.begin());
      update_file(output_file);
      output_file.close();
      return player4;
      break;

      default:
      dealer.push_back(new_deck[0]);
      new_deck.erase(new_deck.begin());
      update_file(output_file);
      output_file.close();
  }
  return dealer;
}


void stay(int peer)
{
  //End the peer's play
  turn_done = 1;
  player_status[peer+1] = 2;
  //cout << "You stay" <<endl;;
}


int decision()
{
  int sum = 0;
  for (int i =0; i < dealer.size(); i++)
  {
    if (dealer[i][0] == 'J' || dealer[i][0] == 'Q' || dealer[i][0] == 'K' || dealer[i][0] == '1')
    {
      sum += 10;
    }
    else if (dealer[i][0] == 'A')
    {
      if (sum + 11 > 21)
        sum += 1;
      else
        sum += 11;
    }
    else
    {
      sum += (dealer[i][0] - 48);  //Converts char to int;
    }
  }

  if (sum < 17)
  {
    return 1; // Hit
  }

  return 0;
}


void resolve_round()
{
  //Dealer gets his cards and compares them with the players, updates player_status
    while(decision())
    {
      cout << "Dealer hits ... ";
      vector<string> dealer = hit(0);
      cout << "Current Hand: ";
      for (int i = 0; i < dealer.size(); i++)
      {
        cout << dealer[i] << " ";
      }
      cout << endl <<endl;


    }
    stay(0);
    player_status[0] = 2;


    vector<int> sum = {0, 0, 0, 0, 0};
    vector<vector<string>> hands;
    hands.push_back(dealer);
    hands.push_back(player1);
    hands.push_back(player2);
    hands.push_back(player3);
    hands.push_back(player4);

    //Calculate score for the hands of each player (and dealer)
    for (int k=0; k < hands.size(); k++)
    {


      for (int i =0; i < hands[k].size(); i++)
      {
        if (hands[k][i][0] == 'J' || hands[k][i][0] == 'Q' || hands[k][i][0] == 'K' || hands[k][i][0] == '1')
        {
          sum[k] += 10;
        }
        else if (hands[k][i][0] == 'A')
        {
          if (sum[k] + 11 > 21)
            sum[k] += 1;
          else
            sum[k] += 11;
        }
        else
        {
          sum[k] += (hands[k][i][0] - 48);  //Converts char to int;
        }
      }

    }

    for (int j = 1; j < sum.size(); j++)
    {
      if (sum[0] > sum[j])
        player_status[j] = -1;
      else
        player_status[j] = 1;
    }



  game_status = 0;
  last_pstatus = player_status;


}


void game_flow()
{
  if (new_deck.size() > 15)
  {
    int continue_check = 0;
    player_status = {0, 0, 0, 0, 0};
    game_status = 1;
    //The player and dealer hands
    dealer.clear();
    player1.clear();
    player2.clear();
    player3.clear();
    player4.clear();
    ofstream output_file ("game_log");

    //connect to backup server


    //Initial card dealing
    for (int i=0; i < 2; i++)
    {
      player1.push_back(new_deck[0]);
      new_deck.erase(new_deck.begin());
      player2.push_back(new_deck[0]);
      new_deck.erase(new_deck.begin());
      player3.push_back(new_deck[0]);
      new_deck.erase(new_deck.begin());
      player4.push_back(new_deck[0]);
      new_deck.erase(new_deck.begin());
      dealer.push_back(new_deck[0]);
      new_deck.erase(new_deck.begin());
    }
    cout << "Starting Hands" <<endl;
    cout << "======================" <<endl;
    cout << "(Dealer): ";
    for (int i = 0; i < dealer.size(); i++)
    {
      cout << dealer[i] << " ";
    }
    cout << endl <<endl;


    cout << "(Player 1): ";
    for (int i = 0; i < player1.size(); i++)
    {
      cout << player1[i] << " ";
    }
    cout << endl <<endl;


    cout << "(Player 2): ";
    for (int i = 0; i < player2.size(); i++)
    {
      cout << player2[i] << " ";
    }
    cout << endl <<endl;


    cout << "(Player 3): ";
    for (int i = 0; i < player3.size(); i++)
    {
      cout << player3[i] << " ";
    }
    cout << endl <<endl;

    cout << "(Player 4): ";
    for (int i = 0; i < player4.size(); i++)
    {
      cout << player4[i] << " ";
    }
    cout << endl <<endl;

    update_file(output_file);

    output_file.close();
    turn = 0;
    while(game_status)
    {
      turn += 1;
      turn_done = 0;
      sleep(1);
      if(player_status[turn] != 0)
      {
        //Skip
        turn_done = 1;
        turn += 1;
        sleep(1);
      }
      while(!turn_done) //Something to block client?
      {
        //Wait for client to complete turn
      }
      //cout << turn <<endl;
      if (turn >=  player_count)
      {
        for (int j = 0; j < player_status.size(); j++)
        {
          continue_check = 0;
          if (player_status[j] == 0)
          {
            continue_check++;
            turn = 0;
          }
        }

        if (continue_check == 0)
        {
          resolve_round();
        }

      }
    }
    cout << "Final Hands" <<endl;
    cout << "======================" <<endl;
    cout << "(Dealer): ";
    for (int i = 0; i < dealer.size(); i++)
    {
      cout << dealer[i] << " ";
    }
    cout << endl <<endl;


    cout << "(Player 1): ";
    for (int i = 0; i < player1.size(); i++)
    {
      cout << player1[i] << " ";
    }
    cout << endl <<endl;


    cout << "(Player 2): ";
    for (int i = 0; i < player2.size(); i++)
    {
      cout << player2[i] << " ";
    }
    cout << endl <<endl;


    cout << "(Player 3): ";
    for (int i = 0; i < player3.size(); i++)
    {
      cout << player3[i] << " ";
    }
    cout << endl <<endl;

    cout << "(Player 4): ";
    for (int i = 0; i < player4.size(); i++)
    {
      cout << player4[i] << " ";
    }
    cout << endl <<endl;
  }
}


int main()
{
  int port;
  int select = 0;
  cout << "This process will operate as the Server." <<endl <<endl;
  //cout << "Select the port that will be used for listening: ";
  //cin >> port;
  //cout << endl;
  cout << "The server is now active. Waiting for players..." <<endl;
  //initialize_file();
  //Set up server, bind each of the commands for peer
  rpc::server server(9000);
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


  begin_game();
  while(new_deck.size() > 15)
  {
    cout << "A new game will begin in 5 seconds." <<endl <<endl <<endl;
    sleep(5);
    game_flow();

  }
  cout << "Not enough cards to continue!" <<endl;
  g_shutdown = 1;
  while(peer_count > 0)
  {
  }
  server.close_sessions();


  return 0;
}
