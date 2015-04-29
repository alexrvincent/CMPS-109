// $Id: cix-client.cpp,v 1.7 2014-07-25 12:12:51-07 - - $

#include <iostream>
#include <utility>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cstdlib>
#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

#include "cix_protocol.h"
#include "logstream.h"
#include "signal_action.h"
#include "sockets.h"

logstream log (cout);

//Command map holds and calls cix commands
unordered_map<string,cix_command> command_map 
{
   {"exit", CIX_EXIT},
   {"help", CIX_HELP},
   {"ls"  , CIX_LS  },
   {"get" , CIX_GET },
   {"put" , CIX_PUT }, 
   {"rm"  , CIX_RM  },
};

//Help command prints following text
void cix_help() 
{
   static vector<string> help = 
   {
      "exit         - Exit the program.  Equivalent to EOF.",
      "get filename - Copy remote file to local host.",
      "help         - Print help summary.",
      "ls           - List names of files on remote server.",
      "put filename - Copy local file to remote host.",
      "rm filename  - Remove file from remote server.",
   };
   for (const auto& line: help) cout << line << endl;
}

//LS command prints contents of running server directory
void cix_ls (client_socket& server) 
{
   //Construct and debug print packet
   cix_header header;
   header.cix_command = CIX_LS;
   log << "sending header " << header << endl;
   
   //Send and debug print packets
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   
   //Case: Server returns incorrect command
   if (header.cix_command != CIX_LSOUT) 
   {
      log << "sent CIX_LS, server did not return CIX_LSOUT" << endl;
      log << "server returned " << header << endl;
   }
   //Otherwise load payload into buffer and print it
   else 
   {
      char buff[header.cix_nbytes + 1];
      recv_packet (server, buff, header.cix_nbytes);
      log << "received " << header.cix_nbytes << " bytes" << endl;
      buff[header.cix_nbytes] = '\0';
      cout << buff;
   }
}


//Get command grabs file from server, saves to local client directory
void cix_get (client_socket& server, const string& input) 
{
   //Construct sending packet
   cix_header header;
   header.cix_command = CIX_GET;
   
   //Parse filename into packet then debug log packet
   for(size_t index = 0; index < input.size(); index++)
   {
     header.cix_filename[index] = input[index]; 
   }
   string name_of_file = header.cix_filename;
   
   //Send and Receive, debug print received packet
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   
   //Handle if server errors (sends NAK or no ACK)
   if (header.cix_command != CIX_ACK) 
   {
      log << "sent CIX_GET, server did not return CIX_ACK" << endl;
      log << "server returned " << header << endl;
      cout << "Failed save file from the server" << endl;
   }
   //Otherwise store payload into a null-terminated buffer
   //Then write it to a new file
   else 
   {
      //Open new output stream and a new buffer
      char buff[header.cix_nbytes + 1];
      ofstream output_stream (header.cix_filename); 
      
      //Receive and print debug log
      recv_packet (server, buff, header.cix_nbytes);
      log << "received " << header.cix_nbytes << " bytes" << endl;
      
      //Fill buffer with file contents, end with a null plug
      buff[header.cix_nbytes] = '\0';
      output_stream.write(buff, header.cix_nbytes);
      output_stream.close();
      cout << "Successfully saved file to local directory" << endl;
   }
}

//Put copies file from local client dir to running server dir
void cix_put (client_socket& server, const string& input) 
{
   //Begin constructing packet
   cix_header header;
   header.cix_command = CIX_PUT;
   
   //Parse input into filename on the header
   for(size_t index = 0; index < input.size(); index++)
   {
     header.cix_filename[index] = input[index];  
   }
   
   //Open a new file to write to
   int buff_size = 0;
   ifstream input_stream (header.cix_filename, ifstream::binary); 
   
   //If the file is unable to be opened, print to err
   if (input_stream == nullptr) 
   {
      log << "get: Unable to open file " << strerror (errno) << endl;
      
      //Load error into the packet and send it
      cerr << "Failed to put file onto the server" << endl;
      header.cix_command = CIX_NAK;
      header.cix_nbytes = errno;
      send_packet (server, &header, sizeof header);
   }
   
   //Otherwise load payload into buffer, into a packet and send it
   else
   {   
       //Delimit to the beginning and ending of the stream
       input_stream.seekg(0, input_stream.end);
       buff_size = input_stream.tellg();
       input_stream.seekg (0, input_stream.beg);
       
       //Create the buffer, load it with the payload
       char* buff = new char[buff_size];
       input_stream.read(buff,buff_size);
       header.cix_nbytes = buff_size;
       
       //Send PUT command then follow with payload packet w/ log
       log << "sending header " << header << endl;
       send_packet (server, &header, sizeof header);
       send_packet (server, buff, header.cix_nbytes);
       log << "sent " << buff_size << " bytes" << endl;
       
       //Accept received command then receive payload from server
       recv_packet(server, &header, sizeof header);
       recv_packet(server, buff, header.cix_nbytes);
       delete[] buff;
       input_stream.close();
       cout << "Successfully put file onto server" << endl;       
   }
 }

//RM unlinks file in the running server directory
void cix_rm (client_socket& server, const string& input) 
{
   //Begin constructing packet
   cix_header header;
   header.cix_command = CIX_RM;
   
   //Filename is just the rest of the input, copy it to the packet
   string name_of_file = input; 
   strcpy(header.cix_filename, input.c_str());
   
   //Send packet and receive response, then print debug received log
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   
   //Handle case of NAK or non-ACK response
   if (header.cix_command != CIX_ACK) 
   {
      log << "sent CIX_RM, server did not return CIX_ACK" << endl;
      log << "server returned " << header << endl;
      cerr << name_of_file <<" : failed to removed file" << endl;
   }
   else
   {
    cout << "Successfully unlinked file from the server" << endl;
   }
}
bool SIGINT_throw_cix_exit {false};

//Useage error for bad commands
void usage() 
{
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

//Signal handler for interrupt signals on client
void signal_handler (int signal) 
{
   log << "signal_handler: caught " << strsignal (signal) << endl;
   switch (signal) 
   {
      case SIGINT: case SIGTERM: SIGINT_throw_cix_exit = true; break;
      default: break;
   }
}

int main (int argc, char** argv) 
{
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   
   //Print a usage error if the there are too many input commands
   if (args.size() > 2) usage();
   
   //Otherwise get the host and port from daemon
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   
   //Begin to parse the input lines
   if (args.size() == 1) 
   {////
      string prog_name = args[0];
      size_t rc_find = prog_name.find(".");
      
      //Case unable to find server name
      if(rc_find == string::npos) 
      {
         host = get_cix_server_host (args,1);
         port = get_cix_server_port (args,0); 
      }
      //Case: found server name
      else 
      {
         host = get_cix_server_host (args,0);
         port = get_cix_server_port (args,1);
      } 
   }
   //Case: Default, run with args 0, args 1  
   else 
   {
      host = get_cix_server_host (args, 0);
      port = get_cix_server_port (args, 1);
   }
   log << to_string (hostinfo()) << endl;
   
   //Starter code: try to connect
   try 
   {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      log << "connected to " << to_string (server) << endl;
      
      //Infinite loop, prog_name input from cin and parse it
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         if (SIGINT_throw_cix_exit) throw cix_exit();
         //Begin to make
         
         
         //Minor loop, cleans input from cin
         size_t end = 0;
         vector<string> cleaned_line;
         for (;;) 
         {
            size_t start = line.find_first_not_of (" \t", end);
            if (start == string::npos) break;
            end = line.find_first_of (" \t", start);
            cleaned_line.push_back (line.substr (start, end - start));
         }
         
         //If there is nothing there, then continue through
         if(cleaned_line.size() == 0) 
         {
           continue;
         }
         
         //Log the command and find it in the map
         log << "command " << cleaned_line[0] << endl;
         const auto& itor = command_map.find (cleaned_line[0]);
         cix_command cmd = itor == command_map.end()
                         ? CIX_ERROR : itor->second;
                         
         //Switch depending on the command type
         switch (cmd) {
            case CIX_EXIT:
               throw cix_exit();
               break;
            case CIX_HELP:
               cix_help();
               break;
            case CIX_LS:
               cix_ls (server);
               break;
            case CIX_GET:
               //Check for invalid get command
               if(cleaned_line.size() < 2) 
               {
                  cout << "Error: Invalid arguments to: get" << endl;
                  break;
               }
               cix_get (server, cleaned_line[1]);
               break; 
            case CIX_PUT:
               //Check for invalid put command
               if(cleaned_line.size() < 2) 
               {
                  cout << "Error: Invalid arguments to: put" << endl;
                  break;
               }
               cix_put (server, cleaned_line[1]);
               break;
            case CIX_RM:
               //Check for invalid rm command
               if(cleaned_line.size() < 2) 
               {
                  cout << "Error: Invalid arguments to: rm" << endl;
                  break;
               }
               cix_rm (server, cleaned_line[1]);
               break;
            default:
               log << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) 
   {
      log << error.what() << endl;
   }catch (cix_exit& error) 
   {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}

