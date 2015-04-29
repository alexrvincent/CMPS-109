// $Id: cix-server.cpp,v 1.7 2014-07-25 12:12:51-07 - - $

#include <fstream> 
#include <iostream>
using namespace std;

#include <libgen.h>

#include "cix_protocol.h"
#include "logstream.h"
#include "signal_action.h"
#include "sockets.h"

logstream log (cout);

//LS sends buffer of names of server's contents
void reply_ls (accepted_socket& client_sock, cix_header& header) 
{
   FILE* ls_pipe = popen ("ls -l", "r");
   
   //If ls failed, put failure in package
   if (ls_pipe == NULL) 
   {
      log << "ls -l: popen failed: " << strerror (errno) << endl;
      header.cix_command = CIX_NAK;
      header.cix_nbytes = errno;
      send_packet (client_sock, &header, sizeof header);
   }
   
   //Construct output and buffers
   string ls_output;
   char buff[0x1000];
   
   //Infinite loop gets to buffer until null plug
   for (;;) 
   {
      char* return_code = fgets (buff, sizeof buff, ls_pipe);
      if (return_code == nullptr) break;
      ls_output.append (buff);
   }
   
   header.cix_command = CIX_LSOUT;
   header.cix_nbytes = ls_output.size();
   memset (header.cix_filename, 0, CIX_FILENAME_SIZE);
   
   //Send and receive packets, log debugs
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock, ls_output.c_str(), ls_output.size());
   log << "sent " << ls_output.size() << " bytes" << endl;
}

void reply_get (accepted_socket& client_sock, cix_header& header) 
{
   //Create string buffer and input stream
   int buff_size = 0; 
   ifstream input_stream (header.cix_filename, ifstream::binary);
   
   //Case: If the stream had an error for some reason
   if (input_stream == nullptr) 
   {
      //Alert the user
      log << "get : Failed to open file " << strerror (errno) << endl;
      
      //Send back a NAK
      header.cix_nbytes = errno;
      header.cix_command = CIX_NAK;
      
      send_packet (client_sock, &header, sizeof header);
   }
   //Case: Stream was successful
   else
   {
      //Delimit to the beginning and ending of the stream
      //Then get the buffer size via a tellg call
      input_stream.seekg(0, input_stream.end);
      buff_size = input_stream.tellg();
      input_stream.seekg (0, input_stream.beg);  
      
      //Create the buffer, load it with the payload
      char* buff = new char[buff_size]; 
      input_stream.read(buff,buff_size);
      input_stream.close();
  
      //Send back an ACK
      header.cix_command = CIX_ACK;
      header.cix_nbytes = buff_size;
      
      //Log and send the ACK packet with the buffer
      log << "sending header " << header << endl;
      send_packet (client_sock, &header, sizeof header);
      send_packet (client_sock, buff, header.cix_nbytes);
      log << "sent " << buff_size << " bytes" << endl;
      
      delete[] buff;
   }
}

void reply_put(accepted_socket& client_sock, cix_header& header)
{
   //Create buffer and receive packet
   char buff[header.cix_nbytes +1];
   recv_packet (client_sock, buff, header.cix_nbytes);
   log << "received header " << header << endl;
   
   //Create output stream and file name from received header
   ofstream output_stream (header.cix_filename);
   string name_of_file = header.cix_filename;
   
   //Case: Output stream is successfully received and created
   if(output_stream)
   {
      //Fill the file with the buffer's contents
      buff[header.cix_nbytes] = '\0';
      output_stream.write(buff, header.cix_nbytes);
      output_stream.close();
      
      //Send back an ACK and the buffer
      header.cix_command = CIX_ACK;
      send_packet(client_sock, &header, sizeof header);
      send_packet (client_sock, buff, header.cix_nbytes);
   }
   //Case: Output stream was unsuccessful
   else
   {
      //Send back a NAK
      header.cix_command = CIX_NAK;
      send_packet(client_sock, &header, sizeof header);
   }
 
}

void reply_rm (accepted_socket& client_sock, cix_header& header) 
{
   //Begin to unlink the files
   int return_code = unlink(header.cix_filename);
   
   //Case: There was an error in the unlinking process
   string remove_status = "";
   if (return_code < 0)
   {
      log << "rm : Failed to unlink" << strerror (return_code)<< endl;
      
      //Send back a NAK failure
      header.cix_command = CIX_NAK;
      header.cix_nbytes = return_code;
      send_packet (client_sock, &header, sizeof header);
   }
   //Otherwise success and send an ACK
   //Construct the packet
   header.cix_command = CIX_ACK;
   header.cix_nbytes = remove_status.size();
   memset (header.cix_filename, 0, CIX_FILENAME_SIZE);
   
   //Send the ACK and the the success status
   log << "sending header " << header << endl;
   send_packet (client_sock, &header, sizeof header);
   send_packet (client_sock,
         remove_status.c_str(), remove_status.size());
   log << "sent " << remove_status.size() << " bytes" << endl;
}

bool SIGINT_throw_cix_exit = false;

//Signal handler will alert user of bad input
void signal_handler (int signal) {
   log << "signal_handler: caught " << strsignal (signal) << endl;
   switch (signal) {
      case SIGINT: case SIGTERM: SIGINT_throw_cix_exit = true; break;
      default: break;
   }
}

//Starter Code main function for server
int main (int argc, char** argv) {
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   int client_fd = args.size() == 0 ? -1 : stoi (args[0]);
   log << "starting client_fd " << client_fd << endl;
   try {
      accepted_socket client_sock (client_fd);
      log << "connected to " << to_string (client_sock) << endl;
      
      //Infinite loop: Continously receive packets from the client
      for (;;) {
         if (SIGINT_throw_cix_exit) throw cix_exit();
         cix_header header;
         recv_packet (client_sock, &header, sizeof header);
         log << "received header " << header << endl;
         //Break on the command recieved
         switch (header.cix_command) {
            case CIX_LS:
               reply_ls (client_sock, header);
               break;
            case CIX_GET:
               reply_get(client_sock, header);
               break;
            case CIX_PUT:
               reply_put(client_sock, header);
               break;
            case CIX_RM:
               reply_rm(client_sock, header);
               break;
            default:
               log << "invalid header from client" << endl;
               log << "cix_nbytes = " << header.cix_nbytes << endl;
               log << "cix_command = " << header.cix_command << endl;
               log << "cix_filename = " << header.cix_filename << endl;
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

