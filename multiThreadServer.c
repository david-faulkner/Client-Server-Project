/*
 * multiThreadServer.c -- a multithreaded server
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <fstream>

using namespace std;

#define PORT 19875  // port we're listening on
#define MAX_LINE 256
#define MAX_ARRAY 20     //the list of messages will have no more than this many entries and no more than this many users can connect

fd_set master;   // master file descriptor list
int listener;    // listening socket descriptor
int fdmax;

struct userInfo {   //struct that will hold client information
     long thisSocket;
     string ipAddr;
     bool currLog,
          rootLog;
     string userName;

     userInfo() {
          thisSocket = 0;
          ipAddr = "\0";
          currLog = false;
          rootLog = false;
          userName = "\0";
     }
};

struct userInfo userArray[MAX_ARRAY];        //only 40 users will be tracked
int currCount = 0, currItems = 0;
ifstream inFile("input.txt");
string strStore;                    //used to store messages
string msgList[MAX_ARRAY];          //messages will be stored here
ofstream outFile;


/*
pre-condition: takes in the character array which is the input from the client, and child socket
post-condition: if the comparison of the user input is equal to the logout command and the user is already logged in, it will give the "200 OK" message and turn the login booleans false
description: Allows the client to log out through the server
*/
void Logout(char buf[], const long& childSocket) {

     for (int i = 0; i < MAX_ARRAY; ++i) {
          if (userArray[i].thisSocket == childSocket) {
               if (strcmp(buf, "LOGOUT\n") == 0 && userArray[i].currLog) { //allows user to logout when already logged into the system

                    strcpy(buf, "200 OK");//send this message to the client, from the server

                    //reset log variables
                    userArray[i].currLog = false;
                    userArray[i].rootLog = false;
                    userArray[i].userName = "\0";

               }
               else {

                    strcpy(buf, "Not logged in yet");// if the user is not already logged in

               }
               break;
          }
     }

}

/*
 pre-condition: takes in the character array which is the input from the client, and the child socket
 post-condition: if the string received by the server, which is input by the client/user, is the same as any of the predetermined User ID and passwords then the server will respond a "200 OK" message
 description: allows the user to login with the permission from the server
 */
void Login(char buf[], const long& childSocket) {


     if (strcmp(buf, "LOGIN root root01\n") == 0) {//login for the root

          strcpy(buf, "200 OK");//message sent when logged into the client, sent from the server


          for (int i = 0; i < MAX_ARRAY; ++i) {
               if (userArray[i].thisSocket == childSocket) {
                    userArray[i].currLog = true;
                    userArray[i].rootLog = true;
                    userArray[i].userName = "root";
                    break;
               }
          }

     }
     else if (strcmp(buf, "LOGIN john john01\n") == 0) {//login for john

          strcpy(buf, "200 OK");//message sent when logged into the client, sent from the server

          for (int i = 0; i < MAX_ARRAY; ++i) {
               if (userArray[i].thisSocket == childSocket) {
                    userArray[i].currLog = true;
                    userArray[i].userName = "john";
                    break;
               }
          }
     }
     else if (strcmp(buf, "LOGIN david david01\n") == 0) {//login for david

          strcpy(buf, "200 OK");//message sent when logged into the client, sent from the server

          for (int i = 0; i < MAX_ARRAY; ++i) {
               if (userArray[i].thisSocket == childSocket) {
                    userArray[i].currLog = true;
                    userArray[i].userName = "david";
                    break;
               }
          }
     }
     else if (strcmp(buf, "LOGIN mary mary01\n") == 0) {//login for mary

          strcpy(buf, "200 OK");//message sent when logged into the client, sent from the server

          for (int i = 0; i < MAX_ARRAY; ++i) {
               if (userArray[i].thisSocket == childSocket) {
                    userArray[i].currLog = true;
                    userArray[i].userName = "mary";
                    break;
               }
          }
     }
     else {

          strcpy(buf, "410 Wrong UserID or Password");//when an incorrect login command is entered, sent from the server to client

     }


}

/*
Description:    Will store an argument string in the list of messages. If the there is already MAX_ARRAY messages in the list
                then the function will return false

Pre-condition:  The storage array is global and will not be passed to the function.

Post-condition: Will store string in the list and return true. If the list is full, will return false.
*/
bool msgstore() {
     if (currItems + 1 > MAX_ARRAY) {
          return false;
     }

     msgList[currItems++] = strStore;
     return true;
}

/*
Description:    Will retrieve a message from the stored array of messages and return that message. The next message
                to be retrived will depend upon an iterating count to determine the index of the stored message.

Pre-condition:  The list of messages, stored in an array, has at least one entry in it.

Post-condition: Will return a string containing the message, increments the current count.
*/
string msgget() {

     if (currCount > currItems - 1) {       //increment the count, if it's out of array range then reset to beginning
          currCount = 0;
     }

     return msgList[currCount++];
}

/*
Description: A function that will store the messages in the input file for future use
Pre-condition: Requires the initial list to have at least one entry and the length of the array to
               be up to date.
Post-condition: The messages of the day will be stored in the file.
*/
void storeInFile() {

     //store the first message without adding a first newline
     outFile << msgList[0];

     //add the rest of the messages without adding a newline to the end of file
     for (int i = 1; i < currItems; ++i) {
          outFile << endl << msgList[i];
     }
}

/*
Description: This function will return a list of names of all users currently logged in.
Pre-conditions: The function takes a character array to store the message that will be sent back to the client as an argument.
Post-conditions: The list of logged in users is stored in the char array.
*/
void ListUsers(char buf[]) {

     string tempString = "\0";

     for (int i = 0; i < MAX_ARRAY; ++i) {   //search through to find logged in users
          if (userArray[i].currLog) {

               if (tempString == "\0") {     //first user logged in
                    tempString = "The list of the active users:\n" + userArray[i].userName + "\t" + userArray[i].ipAddr;
               }
               else {         //any additional user logged in
                    tempString = tempString + "\n" + userArray[i].userName + "\t" + userArray[i].ipAddr;
               }
          }
     }

     if (tempString == "\0") {     //handle no users logged in
          tempString = "No users currently logged in";
     }

     strcpy(buf, tempString.c_str());
}

/*
Description: The primary function that will handle communications to and from a specific client.
Pre-condition: Requires a new thread to be created for the client and the socket number to be passed.
Post-condition: The function will handle messages received from the client, perform 
                associated actions, and close the socket when finished.
*/
void* ChildThread(void* newfd) {
     char buf[MAX_LINE];
     int nbytes;
     int i, j;
     long childSocket = (long)newfd;

     while (1) {
          // handle data from a client
          if ((nbytes = recv(childSocket, buf, sizeof(buf), 0)) <= 0) {
               // got error or connection closed by client
               if (nbytes == 0) {
                    // connection closed
                    cout << "multiThreadServer: socket " << childSocket << " hung up" << endl;
               }
               else {
                    perror("recv");
               }
               close(childSocket); // bye!
               FD_CLR(childSocket, &master); // remove from master set
               pthread_exit(0);
          }
          else {
               // we got some data from a client
               cout << buf;
               for (j = 0; j <= fdmax; j++) {

                    if (buf[0] == 'L' && buf[1] == 'O' && buf[2] == 'G' && buf[3] == 'I' && buf[4] == 'N') {   //login feature

                         Login(buf, childSocket);

                    }    //end login
                    else if (buf[0] == 'L' && buf[1] == 'O' && buf[2] == 'G' && buf[3] == 'O' && buf[4] == 'U' && buf[5] == 'T') {  //logout

                         Logout(buf, childSocket);

                    }    //end logout
                    else if (strcmp(buf, "MSGGET\n") == 0) {  //MSGGET
                         strStore = msgget();
                         strStore = "200 OK\n" + strStore;
                         strcpy(buf, strStore.c_str());
                    }    //end msgget
                    else if (strcmp(buf, "MSGSTORE\n") == 0) {  //MSGSTORE

                         for (int y = 0; y < MAX_ARRAY; ++y) {
                              if (userArray[y].thisSocket == childSocket) {          //curr user
                                   if (userArray[y].currLog) {    //response if logged in
                                        strcpy(buf, "200 OK");
                                        send(childSocket, buf, sizeof(buf), 0);

                                        //receive string to be stored
                                        recv(childSocket, buf, sizeof(buf), 0);
                                        strStore = buf;
                                        strStore.at(strStore.length() - 1) = '\0';
                                        if (msgstore()) {
                                             strcpy(buf, "200 OK");
                                        }
                                        break;
                                   }
                                   else {    //user not logged in
                                        strcpy(buf, "401 You are not currently logged in, login first.");
                                   }
                              }
                         }
                    }    //end msgstore
                    else if (strcmp(buf, "QUIT\n") == 0) {  //quit, resetting the struct to default

                         strcpy(buf, "200 OK");
                         send(childSocket, buf, strlen(buf) + 1, 0);
                         for (int k = 0; k < MAX_ARRAY; ++k) {
                              if (userArray[k].thisSocket == childSocket) {
                                   userArray[k].thisSocket = 0;
                                   userArray[k].currLog = false;
                                   userArray[k].rootLog = false;
                                   userArray[k].userName = "\0";
                                   userArray[k].ipAddr = "\0";
                              }
                         }
                         close(childSocket);
                         break;
                    }    //end quit
                    else if (strcmp(buf, "SHUTDOWN\n") == 0) {   //shutdown

                         for (int i = 0; i < MAX_ARRAY; ++i) {
                              if (userArray[i].thisSocket == childSocket) {
                                   if (!userArray[i].rootLog) {  //user isn't root
                                        strcpy(buf, "403 User not allowed to execute this command");

                                   }
                                   else { //user is root, shut down
                                        strcpy(buf, "200 OK\n210 the server is about to shutdown....");
                                        send(childSocket, buf, sizeof(buf), 0);
                                        outFile.open("input.txt");
                                        storeInFile();
                                        outFile.close();
                                        strcpy(buf, "210 the server is about to shutdown....");
                                        for (int z = 0; z < MAX_ARRAY; ++z) {   //close all other connections
                                             if (userArray[z].thisSocket && userArray[z].thisSocket != childSocket) {
                                                  send(userArray[z].thisSocket, buf, sizeof(buf), 0);
                                                  close(userArray[z].thisSocket);
                                             }
                                        }
                                        exit(1);
                                   }
                              }
                         }
                    }    //end shutdown
                    else if (strcmp(buf, "WHO\n") == 0) {   //list everyone signed in

                         ListUsers(buf);

                    }    //end who
                    else if (buf[0] == 'S' && buf[1] == 'E' && buf[2] == 'N' && buf[3] == 'D') {
                         
                         bool loggedIn = false;
                         string myName;      //name of the person sending the message
                         for (int i = 0; i < MAX_ARRAY; ++i) {   //find name and verify logged in
                              if (userArray[i].thisSocket == childSocket && userArray[i].currLog) {
                                   myName = userArray[i].userName;
                                   loggedIn = true;
                                   break;
                              }
                         }
                         
                         if (loggedIn) {     //client is logged in and can send message
                              string tempMsg = buf;
                              string tempName = tempMsg.substr(5, tempMsg.size() - 6);    //extract name of recipient from the message

                              int tempCount = 0;  //used to determine if the recipient is signed in
                              for (int i = 0; i < MAX_ARRAY; ++i) {   //if user is signed in, 
                                   if (userArray[i].userName != tempName) {
                                        tempCount++;   //tempCount will == MAX_ARRAY if the recipient is not signed in, otherwise will equal MAX_ARRAY - 1
                                   }
                              }

                              if (tempCount != MAX_ARRAY) { //if recipient of message is logged in
                                   strcpy(buf, "200 OK");
                                   send(childSocket, buf, sizeof(buf), 0);
                                   recv(childSocket, buf, sizeof(buf), 0);

                                   tempMsg = "200 OK you have a new message from " + myName + "\n" + myName + ": " + buf;
                                   strcpy(buf, tempMsg.c_str());

                                   for (int i = 0; i < MAX_ARRAY; ++i) {
                                        if (userArray[i].userName == tempName) {

                                             send(userArray[i].thisSocket, buf, sizeof(buf), 0);
                                             strcpy(buf, "200 OK");
                                             break;
                                        }

                                   }
                              }
                              else {
                                   strcpy(buf, "420 either the user does not exist or is not logged in");
                              }
                         }
                         else {    //handle client not being logged in
                              strcpy(buf, "You need to be logged in to send a message");
                         }
                         
                    }    //end send

                    if (FD_ISSET(j, &master)) {
                         // send to child socket
                         if (j == childSocket) {
                              if (send(j, buf, sizeof(buf), 0) == -1) {
                                   perror("send");
                              }
                         }
                    }
               }
          }
     }
}





int main(void)
{
     struct sockaddr_in myaddr;     // server address
     struct sockaddr_in remoteaddr; // client address
     int newfd;        // newly accept()ed socket descriptor
     int yes = 1;        // for setsockopt() SO_REUSEADDR, below
     socklen_t addrlen;

     pthread_t cThread;

     FD_ZERO(&master);    // clear the master and temp sets


     //reads messages from the file and stores in an array
     while (!inFile.eof()) {
          getline(inFile, strStore);
          msgList[currItems++] = strStore;
     }
     inFile.close();


     // get the listener
     if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
          perror("socket");
          exit(1);
     }

     // lose the pesky "address already in use" error message
     if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
          perror("setsockopt");
          exit(1);
     }

     // bind
     myaddr.sin_family = AF_INET;
     myaddr.sin_addr.s_addr = INADDR_ANY;
     myaddr.sin_port = htons(PORT);
     memset(&(myaddr.sin_zero), '\0', 8);
     if (bind(listener, (struct sockaddr*) & myaddr, sizeof(myaddr)) == -1) {
          perror("bind");
          exit(1);
     }

     // listen
     if (listen(listener, 10) == -1) {
          perror("listen");
          exit(1);
     }

     // add the listener to the master set
     FD_SET(listener, &master);

     // keep track of the biggest file descriptor
     fdmax = listener; // so far, it's this one

     addrlen = sizeof(remoteaddr);

     // main loop
     for (;;) {
          // handle new connections
          if ((newfd = accept(listener, (struct sockaddr*) & remoteaddr, &addrlen)) == -1) {
               perror("accept");
               exit(1);
          }
          else {
               FD_SET(newfd, &master); // add to master set
               cout << "multiThreadServer: new connection from "
                    << inet_ntoa(remoteaddr.sin_addr)
                    << " socket " << newfd << endl;

               //set the new connection to a struct in the array
               for (int i = 0; i < MAX_ARRAY; ++i) {
                    if (!userArray[i].thisSocket) {     //current spot in array is not taken
                         userArray[i].thisSocket = (long)newfd;
                         userArray[i].ipAddr = inet_ntoa(remoteaddr.sin_addr);
                         break;
                    }
               }


               if (newfd > fdmax) {    // keep track of the maximum
                    fdmax = newfd;
               }

               if (pthread_create(&cThread, NULL, ChildThread, (void*)newfd) < 0) {
                    perror("pthread_create");
                    exit(1);
               }
          }
     }
     return 0;
}

