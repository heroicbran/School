#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <dirent.h>
#include <sstream>
using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])       //For server: Argv has: [1] as the port
				       //For Client: Argv has: [1] as host, [2] as port
{
     int sockfd, sockfd_peerserver, sockfd_peerclient, newsockfd[4], clients[4], portno, n[4], sel, m;
     string host;
     socklen_t clilen[4], clilen_single;
     char buffer[4][256];
     char cbuffer[256];
     struct sockaddr_in serv_addr, cli_addr[4], cli_addr_single;
     struct hostent *server;
     struct dirent **namelist;

     //STRUCT with file name and Position in list.

     //SERVER VARS
     int server_status = 0; //0 = not active, 1 = active
     vector<string> name_index;
     vector<int> client_index;
     vector<int>host_index;


     //CLIENT VARS
     int client_count = 0;
     int client_id = 0;
     int client_status = 0; //0 = not active, 1 = active
     int registered = 0;



     //Command queue

     cout << "Select Operation Mode (Server = 1, Client = 2): ";
     cin >> sel;
	   cout << endl << endl;

	 //Operate as Server
     if (sel == 1)
    {
     cout << "Enter the port number to be used: ";
     cin >> portno;
      if (portno < 0) {
          fprintf(stderr,"ERROR, invalid port provided\n");
          exit(1);
      }
      sockfd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
      if (sockfd < 0)
         error("ERROR opening socket");
      bzero((char *) &serv_addr, sizeof(serv_addr));
      //Server setup and listening
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = INADDR_ANY;
      serv_addr.sin_port = htons(portno);
      if (bind(sockfd, (struct sockaddr *) &serv_addr,
               sizeof(serv_addr)) < 0)
               error("ERROR on binding");
      listen(sockfd,4);                                //This 5 will have to change to match max clients
      //Maybe add a timeout
      //Add support for more clients?

      //bzero(c_buffer,256);
      server_status = 1;



      cout << "The server is active." <<endl;

      while(server_status == 1)
        //Listen for new person w/ accept?      accept4 with no blocking and assign to array
      {

        clilen[client_count] = sizeof(cli_addr[client_count]);
        if (newsockfd[client_count] = accept4(sockfd,                      //Will become array of length 5 for multiples.
                    (struct sockaddr *) &cli_addr[client_count],
                    &clilen[client_count], SOCK_NONBLOCK))
        {
          if (newsockfd[client_count] < 0)
          {
            client_count = client_count;
          }
          else
          {
              char cc = '1' + client_count;
              char *eof;
              eof = &cc + 1;
              *eof = '\0';                //USED to stop reading after end
              string welcome = "You are peer #";
              char *wel = &welcome[0];
              write(newsockfd[client_count], wel, 256);
              write(newsockfd[client_count], &cc, 256);
              client_count += 1;
              host_index.push_back(client_count);
              cout << "Client " << client_count << " has joined." <<endl;

          }

        }

//FOR LOOP FOR BUFFERS

        for (int j =0; j < client_count; j++)
        {
          bzero(buffer[j],256);
          //write(newsockfd[j],"ready",256);
          n[j] = read(newsockfd[j],buffer[j],256);


          if (n[j] > 0)
          {
            cout << "Client #" << " sent command: " << buffer[j] << "." << endl;  //FIX LATER
            string server_buffer = buffer[j];
            if (server_buffer == "registry")
            {
              n[j] = write(newsockfd[j], "ACK",256);

              /*if (n[j] < 0)
              {
                //BETTER HANDLE ON REMOVING CLIENTS
                error("ERROR writing to socket");
                close(newsockfd[j]);
                close(sockfd);
              }*/
              //while reading
              while(n[j] = read(newsockfd[j],buffer[j],256))
              {
                if (n[j] <= 0)
                {
                  //error("ERROR writing to socket");
                  //client_status = 0;

                }
                else
                {
                     server_buffer = buffer[j];

                    if (server_buffer == "reg_end")
                    {
                      bzero(buffer[j],256);
                      break;
                    }
                    else
                    {
                      name_index.push_back(server_buffer);
                      client_index.push_back(j+1);
                      cout << server_buffer <<endl;
                    }
                }



              }

              cout <<endl <<endl << "*** NEW INDEX ****" <<endl;
              for (int k=0; k < name_index.size(); k++)
              {
                  cout << name_index[k] <<endl;

              }
            }


            else if (server_buffer == "quit")
            {
              //Capture that client's number
              //Create 2 iterators. Move them together. If the client list finds the removed client,
              // remove that index from both vectors.
              client_count -= 1;

            }
            else if (server_buffer == "search")
            {
              bzero(buffer,256);
              if (read(newsockfd[j],buffer[j],256) > 0)
              {
                 cout << "Searching for file: " << buffer[j] << "..." << endl;
                 if (name_index.size() == 0)
                 {
                   cout << "No files are registered!" <<endl;
                 }
                 else
                 {
                   //Show the matching peers who have the files
                   int match_count = 0;
                   for (int i=0; i < name_index.size(); i++)
                   {

                     if (name_index[i] == buffer[j])
                     {
                        char match = '0' + client_index[i];
                        char *eof;
                        eof = &match + 1;
                        *eof = '\0';                //USED to stop reading after end
                        cout << match <<endl;

                        write(newsockfd[j], &match ,256);
                        match_count++;

                     }

                   }

                   write(newsockfd[j], "read_end", 256);
                   if (match_count == 0)
                   {
                     cout << "File not found!" <<endl;

                   }

                 }

              }

            }
            else if (server_buffer == "obtain")
            {
                //Read file name
                //If someone has it, do below. If not, error
              bzero(buffer,256);
              if (read(newsockfd[j],buffer[j],256) > 0)
              {
                 cout << "Searching for file: " << buffer[j] << "..." << endl;
                 if (name_index.size() == 0)
                 {
                   cout << "No files are registered!" <<endl;
                 }
                 else
                 {
                   //Show the matching peers who have the files
                   int match_count = 0;
                   for (int i=0; i < name_index.size(); i++)
                   {

                     if (name_index[i] == buffer[j])
                     {
                        char match = '0' + client_index[i];
                        char *eof;
                        eof = &match + 1;
                        *eof = '\0';                //USED to stop reading after end
                        cout << match <<endl;

                        write(newsockfd[j], &match ,256);
                        match_count++;

                     }

                   }

                   write(newsockfd[j], "read_end", 256);
                   if (match_count == 0)
                   {
                     cout << "File not found!" <<endl;

                   }

                 }

              }
            }
          }



      //n = read(newsockfd,c_buffer,255);
      //if (n < 0) error("ERROR reading from socket");


      //printf("Here is the message: %s\n",c_buffer);

      //n = write(newsockfd,"I got your message",18);
      //if (n < 0) error("ERROR writing to socket");

        }
      }
      for (int j = 0; j < client_count; j++)
      {
          close(newsockfd[j]);

      }
      close(sockfd);
    }

	//Operate as Client
    else if (sel == 2)
    {
     cout << "Enter the host to be used: ";
     //cin >> host;
     host = "127.0.0.1";
     cout << host << endl;
     cout << "Enter the port number to be used: ";
     cin >> portno;

     //Client Initialization
      if (portno <= 0 || host.length() < 1) {
         fprintf(stderr,"usage %s hostname port\n", argv[0]);
         exit(0);
      }

      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd < 0)
          error("ERROR opening socket");
      server = gethostbyname(host.c_str());
      if (server == NULL) {
          fprintf(stderr,"ERROR, no such host\n");
          exit(0);
      }
      bzero((char *) &serv_addr, sizeof(serv_addr));
      serv_addr.sin_family = AF_INET;
      bcopy((char *)server->h_addr,
           (char *)&serv_addr.sin_addr.s_addr,
           server->h_length);
      serv_addr.sin_port = htons(portno);
      if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
          error("ERROR connecting");


      client_status = 1;
      cout << "You have been connected to the server." <<endl;

      read(sockfd, cbuffer, 256);
      cout << cbuffer;
      bzero(cbuffer, 256);
      read(sockfd, cbuffer, 256);
      cout << cbuffer <<endl;
      client_id = cbuffer[0] - 48;
      cout <<client_id <<endl;

//PEER-SERVER INITIALIZATION:

      sockfd_peerserver = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
      if (sockfd_peerserver < 0)
         error("ERROR opening socket");
      bzero((char *) &serv_addr, sizeof(serv_addr));
      //Server setup and listening
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = INADDR_ANY;
      serv_addr.sin_port = htons(10000 + client_id);
      if (bind(sockfd_peerserver, (struct sockaddr *) &serv_addr,
               sizeof(serv_addr)) < 0)
               error("ERROR on binding");
      listen(sockfd_peerserver,4);

      //==========================================





      clilen_single = sizeof(cli_addr_single);

      //Then do a REPL with the client in charge of what to do.
      while(client_status == 1)
      {

      //Server makes peer ready
        if (sockfd_peerclient = accept4(sockfd_peerserver,                      //Will become array of length 5 for multiples.
                        (struct sockaddr *) &cli_addr_single,
                        &clilen_single, SOCK_NONBLOCK))
        {
              if (sockfd_peerclient < 0)
              {

              }
              else
              {
                  cout << "A peer is downloading some file " <<endl;
                  read(sockfd_peerclient, cbuffer, 256);
                  cout << cbuffer <<endl;
                  close(sockfd_peerclient);
                  //Open file
                  //For until end file, loop write file.
                  //Close file
                  //Close connection
              }

        }



        cout << "Please enter a command (Type 'help' for a list of commands): ";
        //bzero(buffer,256);
        string buf;
        cin >> buf;
        //c_buffer = &buf[0];

        char *client_buffer = &buf[0];
       //c_buffer = buffer;
        //fgets(buffer,255,stdin);

        if (buf == "registry")
        {

            m = write(sockfd, client_buffer,256);   //Send message to server
            if (m < 0)
            {
              error("ERROR writing to socket");
              client_status = 0;
              break;
            }

            if (registered == 0)
            {
              int check = 0;
              bzero(buffer,256);
              if (read(sockfd, cbuffer,256)) //Read server ACK (Doesn't actually work)
              {
                  check = 1;
                  cout <<"Adding files information to indexing server..." <<endl <<endl;

                  //Open Current Directory
                  DIR *testdir = opendir("./");
                  //Overhead to get to correct file names
                  dirent *testd = readdir(testdir);
                  //Count # of files to add?
                  while (testd = readdir(testdir))
                  {
                    if (strlen(testd->d_name) > 3 && check == 1) //Removes unnecessary extras
                    {
                      cout << "Updating file index, adding: " << testd->d_name << endl;


                      //Send file with "FILENAME[client #] to server"
                      //When peer actually needs to download, use attached # to get right peer.

                      if (m = write(sockfd, testd->d_name,256) < 0)
                      {
                        error("ERROR writing to socket");
                        client_status = 0;
                        break;
                      }

                      //Send file pointer
                      //Send client ID
                    }


                  }

                  cout << "Your file(s) have been registered to the indexing server." <<endl <<endl;
                  write(sockfd, "reg_end",256);
                  registered = 1;
                  closedir(testdir);
               }
            }
            else
            {
              cout << "Your files are already registered!" << endl;

            }

          //Send this pointer to the server list and add name to name list.

        }

        else if (buf == "quit")
        {
          m = write(sockfd, client_buffer,256);   //Send message to server
          if (m < 0)
          {
            error("ERROR writing to socket");
            client_status = 0;
            break;
          }

          client_status = 0;
          cout << "You have been disconnected!" <<endl;
        }
        else if (buf == "search")
        {
          cout << "Enter the name of the file you want to search: ";
          string searcher;
          cin >> searcher;
          char *client_buffer2 = &searcher[0];

          m = write(sockfd, client_buffer,256);   //Send search command to server
          if (m < 0)
          {
            error("ERROR writing to socket");
            client_status = 0;
            break;
          }


          m = write(sockfd, client_buffer2,256);   //Send message to server
          if (m < 0)
          {
            error("ERROR writing to socket");
            client_status = 0;
            break;
          }

          while(m = read(sockfd,cbuffer,256))
          {
            if (m < 0)
            {
              error("ERROR writing to socket");
              client_status = 0;

            }
            string sbuffer = cbuffer;
            if (sbuffer == "read_end")
            {
              bzero(buffer,256);
              break;
            }
            else
            {
            string sclient_buffer = cbuffer;
            cout << sclient_buffer <<endl;
            }

            bzero(buffer,256);
          }


        }
        else if (buf == "obtain")
        {
              cout << "Enter the name of the file you want to download: ";
              string dl;
              cin >> dl;
              char *client_buffer2 = &dl[0];

              m = write(sockfd, client_buffer,256);   //Send obtain command to server
              if (m < 0)
              {
                error("ERROR writing to socket");
                client_status = 0;
                break;
              }


              m = write(sockfd, client_buffer2,256);   //Send message to server
              if (m < 0)
              {
                error("ERROR writing to socket");
                client_status = 0;
                break;
              }

              //GET HOST NAME FROM SERVER
              bzero(cbuffer, 256);
              read(sockfd,cbuffer,256);
              //CONNECT WITH PEER_CLIENT
              sockfd_peerclient = socket(AF_INET, SOCK_STREAM, 0);
              if (sockfd_peerclient < 0)
                  error("ERROR opening socket");
              server = gethostbyname(host.c_str());
              if (server == NULL) {
                  fprintf(stderr,"ERROR, no such host\n");
                  exit(0);
              }
              bzero((char *) &serv_addr, sizeof(serv_addr));
              serv_addr.sin_family = AF_INET;
              bcopy((char *)server->h_addr,
                   (char *)&serv_addr.sin_addr.s_addr,
                   server->h_length);
              serv_addr.sin_port = htons(10000 + (cbuffer[0] - 48));
              if (connect(sockfd_peerclient,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
                  error("ERROR connecting");

              write(sockfd,"D",256);
              //OPEN FILE


              /*while(m = read(sockfd,cbuffer,256))
              {
                if (m < 0)
                {
                  error("ERROR writing to socket");
                  client_status = 0;

                }
                string sbuffer = cbuffer;
                if (sbuffer == "read_end")
                {
                  bzero(buffer,256);
                  break;
                }
                else
                {
                string sclient_buffer = cbuffer;
                cout << sclient_buffer <<endl;
                }

                bzero(buffer,256);


              }*/
              close(sockfd_peerclient);

          }

            close(sockfd);
        }
    }
    else
    {
      error("ERROR selecting Operation Mode. Please enter 1 for 'Server' or 2 for 'Client'.");
    }


     return 0;
}
