//lang::CwC

// Some of the code in this file, particularly the select() loops were interpreted from Beej's Guide to Socking Programming

#pragma once

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>

#include "object.h"
#include "helper.h"
#include "string.h"
#include "serial.h"

#define PORT "8080"
// The fixed number of devices that this network supports (1 server, the rest are clients)
#define BACKLOG 6
// The size of the string buffer used to send messages
#define BUF_SIZE 1024

/**
 * Class representing a device in our network layer.
 * Contains functionalities for both the "server" and the "client"
 * The device on node 0 acts as the server, all of the subsequent devices act as clients
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class Device : public Object {
    public:
        char* ip_;
        // This device's socket file descriptor
        int fd_;
        // struct that will be filled with basic info in order to generate
        // other full structs used to build sockets
        struct addrinfo hints_;
        // The file descriptor and address info of another Device connecting to this once
        int their_fd_;
        struct sockaddr_storage their_addr_;
        // A string buffer used to send messages
        char* buffer_;
        // An array of socket file descriptors to the other Devices
        // The array indices are the node indices of each Device
        int* nodes_;
        // master file descriptor list
        fd_set master_;
        // temporary fd list used by select()
        fd_set read_fds_;
        // the maxmimum fd value in the master list
        int fdmax_;
        // Thread that the select() loop is run in
        std::thread* t_;
        // The index of the node running this Device
        size_t idx_;

        // The server's directory containing every client IP
        // Used by the server only
        Directory* directory_;
        
        // The file descriptor for the socket connected to the Server
        // Used by the client only
        int servfd_;

        /**
         * Destructor. Takes care of terminating the thread.
         */
        ~Device() {
            t_->join();
            delete t_;
        }

        /**
         * Startup protocol for the Server.
         * Creates the socket that the clients will connect through, binds the IP and port to it,
         * and starts client registration.
         */
        void startup_server() {
            idx_ = 0;
            buffer_ = new char[BUF_SIZE];
            ip_ = idx_to_ip_(idx_);
            directory_ = new Directory();
            // This is an array that maps the socket fds of each client to their IP address
            nodes_ = new int[BACKLOG];
            for (int i = 0; i < BACKLOG; i++) nodes_[i] = -1;
            // Fill in the addrinfo struct, configuring the server's options, address, and port
            struct addrinfo* info;
            memset(&hints_, 0, sizeof(hints_));
            hints_.ai_family = AF_INET;
            hints_.ai_socktype = SOCK_STREAM;
            exit_if_not(getaddrinfo(ip_, PORT, &hints_, &info) == 0, 
                "Call to getaddrinfo() failed");
            // Create the socket
            exit_if_not((fd_ = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) >= 0,
                "Call to socket() failed");
            // Bind the Server's IP and port to the socket
            exit_if_not(bind(fd_, info->ai_addr, info->ai_addrlen) >= 0, "Call to bind() failed");
            freeaddrinfo(info);
            // Starts client registration
            t_ = new std::thread(&Device::register_clients_, this);
        }

        /**
         * Shutdown protocol for the Server.
         * Closes the Server's socket to the Clients and deletes all fields.
         */
        void shutdown_server() {
            printf("\033[1;3%dmNode %d: Shutting down\033[0m\n", idx_, idx_);
            close(fd_);
            clear_map_();
            delete directory_;
            delete[] buffer_;
            delete[] nodes_;
        }

        /**
         * Startup protocol for a Client.
         * Creates the sockets that the Client will use to connect to the server and other clients
         * and sends the Client's IP to the server.
         * 
         * @param idx The index of the node running this Client.
         */
        void startup_client(size_t idx) {
            idx_ = idx;
            buffer_ = new char[BUF_SIZE];
            ip_ = idx_to_ip_(idx);
            // This is an array that maps the socket fds of each client to their IP address
            nodes_ = new int[BACKLOG];
            for (int i = 0; i < BACKLOG; i++) nodes_[i] = -1;
            struct addrinfo *servinfo, *info;
            // Fill in 2 addrinfo structs, one for this Client and one for the Server
            memset(&hints_, 0, sizeof(hints_));
            hints_.ai_family = AF_INET;
            hints_.ai_socktype = SOCK_STREAM;
            exit_if_not(getaddrinfo(ip_, PORT, &hints_, &info) == 0, 
                "Call to getaddrinfo() failed");
            // Calculate the server's IP using its node index (always 0) and use that to generate the struct
            exit_if_not(getaddrinfo(idx_to_ip_(0), PORT, &hints_, &servinfo) == 0,
                "Call to getaddrinfo() failed");
            // Create the socket to the Server and the socket that other clients will use to connect
            // to this one
            exit_if_not((fd_ = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) >= 0,
                "Call to socket() failed");
            exit_if_not((servfd_ = socket(servinfo->ai_family, servinfo->ai_socktype, 
                servinfo->ai_protocol)) >= 0,
                "Call to socket() failed");
            // Bind this Clients's IP and port to its socket
            exit_if_not(bind(fd_, info->ai_addr, info->ai_addrlen) >= 0, "Call to bind() failed");
            freeaddrinfo(info);
            // Connect to the Server
            exit_if_not(connect(servfd_, servinfo->ai_addr, servinfo->ai_addrlen) >= 0, 
                "Call to connect() failed");
            // Add the server fd to the fd/idx map
            nodes_[0] = servfd_;
            freeaddrinfo(servinfo);
            // Send IP to server
            printf(
                "\033[1;3%dmNode %d: Sending my IP \"%s\" to the server for registration.\033[0m\n",
                idx_, idx_, ip_
            );
            exit_if_not(send(servfd_, ip_, strlen(ip_), 0) > 0, "Sending IP to server failed");
            // Start listening for incoming messages
            t_ = new std::thread(&Device::monitor_sockets_, this);
        }

        /**
         * Shutdown protocol for a Client.
         * Closes the Client's sockets and deletes all fields.
         */
        void shutdown_client() {
            // Sleep for a second to give the server enough time to fully shut down
            // Else, there will be errors in its select() loop
            sleep(1);
            clear_map_();
            close(servfd_);
            close(fd_);
            delete[] buffer_;
            delete[] nodes_;
        }

        /**
         * Send a message to all clients connected to this one.
         * 
         * @param msg The message to be sent
         */
        void send_all(const char* msg) {
            for (int i = 0; i < BACKLOG; i++) {
                int fd = nodes_[i];
                if (fd != -1) {
                    printf("\033[1;3%dmNode %d: Sending msg to node %d\033[0m\n", idx_, idx_, i);
                    exit_if_not(send(fd, msg, strlen(msg) + 1, 0) > 0, 
                        "Sending msg to other client failed");
                }
            }
        }

        /**
         * Send a message to a specific client.
         * 
         * @param msg The message to be sent
         * @param dst The index of the destination node
         */
        void send_to_node(const char* msg, size_t dst) {
            printf("\033[1;3%dmNode %d: Attempting to send msg to node %d\033[0m\n", idx_, idx_, dst);
            int fd = nodes_[dst];
            if (fd == -1) {
                printf("\033[1;3%dmNode %d: Could not find a node with the given index\033[0m\n", idx_, idx_);
            } else {
                exit_if_not(send(fd, msg, strlen(msg) + 1, 0) > 0, "Sending msg to other client failed");
                printf("\033[1;3%dmNode %d: Msg sent to IP successfully\033[0m\n", idx_, idx_);
            }
        }

        /**
         * Server function
         * Listens for incoming connections from new clients.
         * Clients send IPs to server who then forwards them to its other clients.
         */
        void register_clients_() {
            socklen_t addrlen;  // length of client's addrinfo struct
            int nbytes;         // number of bytes read by recv()
            int num_clients = 0;  // The number of clients in the directory
            // Clear the two fd lists
            FD_ZERO(&master_);
            FD_ZERO(&read_fds_);
            // Start listening
            exit_if_not(listen(fd_, BACKLOG) == 0, "Call to listen() failed");
            // Add this Server's fd to the master list and use it to initialize the max fd value
            FD_SET(fd_, &master_);
            fdmax_ = fd_;
            // Main loop
            for (;;) {
                read_fds_ = master_; // Copy the master list
                // Select the existing socket connections and iterate through them
                exit_if_not(select(fdmax_ + 1, &read_fds_, NULL, NULL, NULL) >= 0, 
                    "Call to select() failed");
                for (int i = 0; i <= fdmax_; i++) {
                    if (FD_ISSET(i, &read_fds_)) {
                        // Found a connection
                        if (i == fd_) {
                            // The Server is receiving a connection from a new client
                            if (num_clients + 1 > BACKLOG) {
                                // The network is full, do not accept this connection
                                printf("\033[1;3%dmNode %d: Network is full, cannot accept new connection.\033[0m\n", 
                                    idx_);
                                continue;
                            }
                            num_clients++;
                            addrlen = sizeof(their_addr_);
                            // Accept the connection and add the new socket fd to the master list
                            their_fd_ = accept(fd_, (struct sockaddr*)&their_addr_, &addrlen);
                            if(their_fd_ < 0) {
                                exit_if_not(false, "Call to accept() failed");
                            }
                            FD_SET(their_fd_, &master_);
                            // Update the max fd value
                            if (their_fd_ > fdmax_) {
                                fdmax_ = their_fd_;
                            }
                            printf("\033[1;3%dmNode %d: New connection on socket %d\033[0m\n",
                                idx_, idx_, their_fd_);
                        } else {
                            // The Server is receving a message from a client
                            if ((nbytes = recv(i, buffer_, BUF_SIZE, 0)) <= 0) {
                                // Connection to the client was closed or there was an error,
                                // so shut down
                                shutdown_server();                                
                            } else {
                                Deserializer ds(buffer_);
                                Message* m = dynamic_cast<Message*>(ds.deserialize());
                                if (m->kind() == MsgKind::Register) {
                                    Register* r = dynamic_cast<Register*>(m);
                                    char* new_ip = r->get_ip()->c_str();
                                    size_t new_idx = r->get_sender();
                                    printf("\033[1;3%dmNode %d: Received IP \"%s\" from new client at socket %d\033[0m\n",
                                        idx_, idx_, new_ip, i);
                                    // Add the new IP to the directory
                                    directory_->add_client(new_ip, new_idx);
                                    // Keep track of the new client's node index and socket fd
                                    nodes_[new_idx] = i;
                                    // Send the updated directory to all other clients
                                    send_directory_();
                                }
                            }
                        }
                    }
                }
            }
        }

        /**
         * Server function
         * Send the directory to all clients on the network except the one that the server is
         * actively communicating with.
         */
        void send_directory_() {
            const char* serial_directory = directory_->serialize();
            printf("\033[1;3%dmNode %d: Sending updated directory to existing clients\033[0m\n",
                idx_, idx_);
            for (int j = 0; j <= fdmax_; j++) {
                if (FD_ISSET(j, &master_)) {
                    if (j != fd_) {
                        exit_if_not(send(j, serial_directory, strlen(serial_directory) + 1, 0) > 0,
                            "Call to send() failed");
                    }
                }
            }
            delete serial_directory;
        }

        /**
         * Client function
         * Listens for incoming messages coming from other devices on the network.
         * If the device is the server, it is sending the directory, so process that.
         * If the device is another client, TODO
         */
        void monitor_sockets_() {
            socklen_t addrlen;              // length of other device's addrinfo struct
            int nbytes;                     // number of bytes read by recv()
            bool directory_init = false;    // has this Client received its first directory update?
            // Clear the two fd lists
            FD_ZERO(&master_);
            FD_ZERO(&read_fds_);
            // Start listening
            exit_if_not(listen(fd_, BACKLOG) == 0, "Call to listen() failed");
            // Add this Client's fd and the Server's to the master list and use them to initialize
            // the max fd value
            FD_SET(fd_, &master_);
            FD_SET(servfd_, &master_);
            fdmax_ = fd_ > servfd_ ? fd_ : servfd_;
            // Main loop
            for (;;) {
                read_fds_ = master_; // Copy the master list
                // Select the existing socket connections and iterate through them
                exit_if_not(select(fdmax_ + 1, &read_fds_, NULL, NULL, NULL) > 0, "Call to select() failed");
                for (int i = 0; i <= fdmax_; i++) {
                    if (FD_ISSET(i, &read_fds_)) {
                        // Found a connection
                        if (i == fd_) {
                            // The Client is receiving a new connection from another client
                            addrlen = sizeof(their_addr_);
                            // Accept the connection and add the new socket fd to the master list
                            exit_if_not((their_fd_ = accept(fd_, (struct sockaddr*)&their_addr_, &addrlen)), 
                                "Call to accept() failed");
                            FD_SET(their_fd_, &master_);
                            // Update the max fd value
                            if (their_fd_ > fdmax_) {
                                fdmax_ = their_fd_;
                            }
                            printf("\033[1;3%dmNode %d: New connection on socket %d\033[0m\n", idx_, idx_, their_fd_);
                        } else {
                            // The Client is receving a message
                            if ((nbytes = recv(i, buffer_, BUF_SIZE, 0)) <= 0) {
                                // Connection to the other device was closed or there was an error,
                                // so shut down
                                shutdown_client();
                            } else {
                                // Figure out what kind of message was received
                                Deserializer ds(buffer_);
                                Message* m = dynamic_cast<Message*>(ds.deserialize());
                                if (m->kind() == MsgKind::Directory) {
                                    // The Server sent a directory
                                    if (!directory_init) {
                                        // If this is the first directory message received from the server,
                                        // it is a new Client, so it creates sockets to all other Clients
                                        process_directory_(dynamic_cast<Directory*>(m));
                                        directory_init = true;
                                    }
                                } else if (m->kind() == MsgKind::Register) {
                                    // Another client sent a Register, so keep track of its fd and node index
                                    nodes_[dynamic_cast<Register*>(m)->get_sender()] = i;
                                }
                            }
                        }
                    }
                }
            }
        }

        /**
         * Client function
         * Parse the directory message sent from the server.
         * Open sockets to all other clients in the directory and send them
         * a greeting message.
         * 
         * @param directory The directory
         */
        void process_directory_(Directory* directory) {
            Vector* ips = directory->get_addresses();
            IntVector* indices = directory->get_indices();
            for (int i = 0; i < ips->size(); i++) {
                connect_to_client_(ips->get(i)->c_str(), indices->get(i));
            }
        }

        /**
         * Client function
         * Create a socket to the client at the given IP, connect to it, and send it a greeting message.
         * 
         * @param ip  The IP address of the other client
         * @param idx The node index of the other client
         */
        void connect_to_client_(char* ip, size_t idx) {
            struct addrinfo* client_info;
            int client_fd;
            // Generate an addrinfo struct for the other client
            memset(&hints_, 0, sizeof(hints_));
            hints_.ai_family = AF_INET;
            hints_.ai_socktype = SOCK_STREAM;
            exit_if_not(getaddrinfo(ip, PORT, &hints_, &client_info) == 0, "Call to getaddrinfo() failed");
            // Create a socket to connect to the client
            exit_if_not((client_fd = socket(client_info->ai_family, client_info->ai_socktype, client_info->ai_protocol)) >= 0,
                "Call to socket() failed");
            // Connect to the client
            exit_if_not(connect(client_fd, client_info->ai_addr, client_info->ai_addrlen) >= 0, "Call to connect() failed");
            freeaddrinfo(client_info);
            // Send the client a Register message
            Register reg(new String(ip_), idx_);
            printf("\033[1;3%dmNode %d: Sending Register to other client at %s, socket %d\033[0m\n", idx_, idx_, ip, client_fd);
            exit_if_not(send(client_fd, reg.serialize(), BUF_SIZE, 0) > 0, "Sending greeting to other client failed");
            // Add the fd to the master list
            FD_SET(client_fd, &master_);
            // Update the max fd value
            if (client_fd > fdmax_) {
                fdmax_ = client_fd;
            }
            // Keep track of the client's fd and node index
            nodes_[idx] = client_fd;
            print_map_();
        }

        /**
         * Empties the fd/ip map
         */
        void clear_map_() {
            for (int i = 0; i < BACKLOG; i++) {
                int fd = nodes_[i];
                if (fd != -1) {
                    close(fd);
                    nodes_[i] = -1;
                }
            }
        }

        /**
         * Prints the nodes_ map
         */
        void print_map_() {
            printf("\033[1;3%dmNode %d: Map: \033[0m", idx_, idx_);
            for (int i = 0; i < BACKLOG; i++) {
                int fd = nodes_[i];
                if (fd != -1) {
                    printf("\033[1;3%dm{fd: %d, idx: %d} \033[0m", idx_, fd, i);
                }
            }
            printf("\n");
        }

        /**
         * The IP address of a Device is 127.0.0.x where x is the node index + 1.
         * So given a node index, this function returns the corresponding IP address.
         */
        char* idx_to_ip_(size_t idx) {
            char* ip = new char[INET_ADDRSTRLEN + 1];
            int bytes = sprintf(ip, "127.0.0.%d", idx + 1);
            exit_if_not(bytes > INET_ADDRSTRLEN, "Invalid index");
            return ip;
        }
};