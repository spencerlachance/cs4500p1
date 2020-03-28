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
#include <unistd.h>

#include "map.h"
// #include "dataframe.h"
#include "serial.h"

#define PORT "8080"
// The fixed number of nodes that this network supports (1 server, the rest are clients)
#define BACKLOG 6
// The size of the string buffer used to send messages
#define BUF_SIZE 1024

/**
 * A class describing an implementation of a key/value store.
 * This class represents a store kept on one node from a larger distributed system.
 * It also holds all of the functionality needed to exchange data with the other nodes.
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
class KVStore : public Object {
public:
    // The index of this node
    size_t idx_;
    // The map from string keys to deserialized data blobs
    Map* map_;
    // Have we received an Ack?
    bool ack_recvd_;
    // Data returned in a Reply message
    DataFrame* reply_data_;
    // Thread that the select() loop is run in
    std::thread* t_;

    /**
     * Constructor that initializes an empty KVStore.
     */
    KVStore(size_t idx) : idx_(idx), ack_recvd_(false), reply_data_(nullptr) {
        map_ = new Map();
        startup();
    }

    /**
     * Destructor
     */
    ~KVStore() {
        t_->join();
        delete t_;
        delete map_;
    }

    /**
     * Serializes the given data and puts it into the map at the given key.
     * 
     * @param k The key at which the data will be stored
     * @param v The data that will be stored in the k/v store
     */
    void put(Key& k, DataFrame& v) {
        size_t dst_node = k.get_home_node();
        // Check if this key corresponds to this node
        if (dst_node == idx_) {
            // If so, put the data in this KVStore's map
            const char* serial_df = v.serialize();
            map_->put(k.get_keystring(), new String(serial_df));
            delete[] serial_df;
        } else {
            // If not, send a Put message to the correct node
            Put p(&k, &v);
            send_to_node(p.serialize(), dst_node);
            // Wait for an Ack confirming that the data was stored successfully
            while (!ack_recvd_) sleep(1);
            ack_recvd_ = false;
        }
    }

    /**
     * Gets the data stored at the given key, deserializes it, and returns it.
     * 
     * @param k The key at which the reqested data is stored
     * 
     * @return The deserialized data
     */
    DataFrame* get(Key& k) {
        size_t dst_node = k.get_home_node();
        // Check if this key corresponds to this node
        if (dst_node == idx_) {
            // If so, get the data from this KVStore's map
            String* serialized_df = dynamic_cast<String*>(map_->get(k.get_keystring()));
            assert(serialized_df != nullptr);
            Deserializer ds(serialized_df->c_str());
            DataFrame* deserialized_df = dynamic_cast<DataFrame*>(ds.deserialize());
            assert(deserialized_df != nullptr);
            return deserialized_df;
        } else {
            // If not, send a Get message to the correct node
            Get g(&k);
            send_to_node(g.serialize(), dst_node);
            // Wait for a reply with the desired data
            while (reply_data_ == nullptr) sleep(1);
            DataFrame* ret = reply_data_;
            reply_data_ = nullptr;
            return ret;
        }
    }

    /**
     * Waits until there is data in the store at the given key, and then gets it, deserializes it,
     *  and returns it.
     * 
     * @param k The key at which the reqested data is stored
     * 
     * @return The deserialized data
     */
    DataFrame* wait_and_get(Key& k) {
        size_t dst_node = k.get_home_node();
        // Check if this key corresponds to this node
        if (dst_node == idx_) {
            // If so, wait until the data is put into this node's map
            bool contains_key = map_->containsKey(k.get_keystring());
            while (!contains_key) {
                sleep(1);
                contains_key = map_->containsKey(k.get_keystring());
            }
            return get(k);
        } else {
            // If not, send a WaitAndGet message to the correct node
            WaitAndGet wag(&k);
            send_to_node(wag.serialize(), dst_node);
            // Wait for a reply with the desired data
            while (reply_data_ == nullptr) sleep(1);
            DataFrame* ret = reply_data_;
            reply_data_ = nullptr;
            return ret;
        }
    }

    // ############################# NETWORK-SPECIFIC FIELDS AND METHODS ###########################

    char* ip_;
    // This node's socket file descriptor
    int fd_;
    // struct that will be filled with basic info in order to generate
    // other full structs used to build sockets
    struct addrinfo hints_;
    // The file descriptor and address info of another node connecting to this once
    int their_fd_;
    struct sockaddr_storage their_addr_;
    // A string buffer used to send messages
    char* buffer_;
    // An array of socket file descriptors to the other nodes
    // The array indices are the node indices of each node
    int* nodes_;
    // master file descriptor list
    fd_set master_;
    // temporary fd list used by select()
    fd_set read_fds_;
    // the maxmimum fd value in the master list
    int fdmax_;

    // The server's directory containing every client IP
    // Used by the server only
    Directory* directory_;
    
    // The file descriptor for the socket connected to the Server
    // Used by the client only
    int servfd_;

    /**
     * Is this node running the role of the server?
     */
    bool is_server() {
        return idx_ == 0;
    }

    /**
     * Startup protocol for a Client.
     * Creates the sockets that the Client will use to connect to the server and other clients
     * and sends the Client's IP to the server.
     * 
     * @param idx The index of the node running this Client.
     */
    void startup() {
        buffer_ = new char[BUF_SIZE];
        ip_ = idx_to_ip_(idx_);
        // This is an array that maps the indices of each node to their socket fds
        nodes_ = new int[BACKLOG];
        for (int i = 0; i < BACKLOG; i++) nodes_[i] = -1;

        // Fill an addrinfo struct for this node, configuring its options, address, and port
        struct addrinfo *info;
        memset(&hints_, 0, sizeof(hints_));
        hints_.ai_family = AF_INET;
        hints_.ai_socktype = SOCK_STREAM;
        exit_if_not(getaddrinfo(ip_, PORT, &hints_, &info) == 0, "Call to getaddrinfo() failed");
        // Use the struct to create a socket
        exit_if_not((fd_ = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) >= 0,
            "Call to socket() failed");
        // Bind the IP and port to the socket
        exit_if_not(bind(fd_, info->ai_addr, info->ai_addrlen) >= 0, "Call to bind() failed");
        freeaddrinfo(info);
        if (is_server()) {
            directory_ = new Directory();
        } else {
            // Calculate the server's IP using its node index (always 0) and use that to generate 
            // another struct
            struct addrinfo *servinfo;
            exit_if_not(getaddrinfo(idx_to_ip_(0), PORT, &hints_, &servinfo) == 0,
                "Call to getaddrinfo() failed");
            // Create the socket to the Server
            exit_if_not((servfd_ = socket(servinfo->ai_family, servinfo->ai_socktype, 
                servinfo->ai_protocol)) >= 0, "Call to socket() failed");
            freeaddrinfo(info);
            // Connect to the Server
            exit_if_not(connect(servfd_, servinfo->ai_addr, servinfo->ai_addrlen) >= 0, 
                "Call to connect() failed");
            // Add the server fd to the fd/idx map
            nodes_[0] = servfd_;
            freeaddrinfo(servinfo);
            // Send IP to server
            printf("\033[1;3%dmNode %d: Sending my IP \"%s\" to the server for registration.\033[0m\n",
                idx_, idx_, ip_);
            exit_if_not(send(servfd_, ip_, strlen(ip_), 0) > 0, "Sending IP to server failed");
        }
        // Start listening for incoming messages
        t_ = new std::thread(&KVStore::monitor_sockets_, this);
    }

    /**
     * Shutdown protocol for a Client.
     * Closes the Client's sockets and deletes all fields.
     */
    void shutdown() {
        printf("\033[1;3%dmNode %d: Shutting down\033[0m\n", idx_, idx_);
        if (is_server()) {
            // Sleep for a second to give the server enough time to fully shut down
            // Else, there will be errors in its select() loop
            sleep(1);
            close(servfd_);
        } else {
            delete directory_;
        }
        clear_map_();
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
     * Client function
     * Listens for incoming messages coming from other nodes on the network and then processes them
     * accordingly.
     */
    void monitor_sockets_() {
        // length of other node's addrinfo struct
        socklen_t addrlen;
        // number of bytes read by recv()
        int nbytes;
        // The number of nodes on the network
        int num_nodes = 1;
        // Clear the two fd lists
        FD_ZERO(&master_);
        FD_ZERO(&read_fds_);
        // Start listening
        exit_if_not(listen(fd_, BACKLOG) == 0, "Call to listen() failed");
        // Add this Client's fd and the Server's to the master list and use them to initialize
        // the max fd value
        FD_SET(fd_, &master_);
        if (is_server()) {
            fdmax_ = fd_;
        } else {
            FD_SET(servfd_, &master_);
            fdmax_ = fd_ > servfd_ ? fd_ : servfd_;
        }
        // Main loop
        for (;;) {
            read_fds_ = master_; // Copy the master list
            // Select the existing socket connections and iterate through them
            exit_if_not(select(fdmax_ + 1, &read_fds_, NULL, NULL, NULL) > 0,
                "Call to select() failed");
            for (int i = 0; i <= fdmax_; i++) {
                if (FD_ISSET(i, &read_fds_)) {
                    // Found a connection
                    if (i == fd_) {
                        if (num_nodes + 1 > BACKLOG) {
                            // The network is full, do not accept this connection
                            printf("\033[1;3%dmNode %d: Network is full, cannot accept new connection.\033[0m\n", 
                                idx_);
                            continue;
                        }
                        num_nodes++;
                        // The node is receiving a new connection from another one
                        addrlen = sizeof(their_addr_);
                        // Accept the connection and add the new socket fd to the master list
                        exit_if_not((their_fd_ = accept(fd_, (struct sockaddr*)&their_addr_, 
                            &addrlen)), "Call to accept() failed");
                        FD_SET(their_fd_, &master_);
                        // Update the max fd value
                        if (their_fd_ > fdmax_) {
                            fdmax_ = their_fd_;
                        }
                        printf("\033[1;3%dmNode %d: New connection on socket %d\033[0m\n", idx_, 
                            idx_, their_fd_);
                    } else {
                        // The Client is receving a message
                        if ((nbytes = recv(i, buffer_, BUF_SIZE, 0)) <= 0) {
                            // Connection to the other node was closed or there was an error,
                            // so shut down
                            shutdown();
                        } else {
                            // Figure out what kind of message was received
                            Deserializer ds(buffer_);
                            Message* m = dynamic_cast<Message*>(ds.deserialize());
                            process_message_(m, i);
                        }
                    }
                }
            }
        }
    }

    /**
     * Figure out what kind of message was sent and process it accordingly
     * 
     * @param m  The message
     * @param fd The file descriptor of the socket over which the message was sent
     */
    void process_message_(Message* m, int fd) {
        switch (m->kind()) {
            case MsgKind::Directory: {
                // Client received directory update from server
                process_directory_(m->as_directory());
            }
            case MsgKind::Register: {
                Register* r = m->as_register();
                char* new_ip = r->get_ip()->c_str();
                size_t new_idx = r->get_sender();
                if (is_server()) {
                    // A client is registering with the server
                    printf("\033[1;3%dmNode %d: Received IP \"%s\" from new client at socket %d\033[0m\n",
                        idx_, idx_, new_ip, fd);
                    // Add the new IP to the directory
                    directory_->add_client(new_ip, new_idx);
                    // Keep track of the new client's node index and socket fd
                    nodes_[new_idx] = fd;
                    // Send the updated directory back to the client
                    const char* serial_directory = directory_->serialize();
                    printf("\033[1;3%dmNode %d: Sending updated directory back to client\033[0m\n",
                        idx_, idx_);
                    exit_if_not(send(fd, serial_directory, strlen(serial_directory) + 1, 0) > 0,
                        "Call to send() failed");
                    delete serial_directory;
                } else {
                    // Another client sent a Register, so keep track of its fd 
                    // and node index
                    nodes_[new_idx] = fd;
                }
            }
            case MsgKind::Ack: {
                // Set the value that put() is waiting for above
                ack_recvd_ = true;
            }
            case MsgKind::Reply: {
                Reply* rep = m->as_reply();
                // Set the value that get() and wait_and_get() wait for above
                reply_data_ = rep->get_value();
            }
            case MsgKind::Put: {
                Put* p = m->as_put();
                // Ensure that this message was sent to the right node
                exit_if_not(p->get_key()->get_home_node() == idx_,
                    "Put was sent to incorrect node");
                put(*(p->get_key()), *(p->get_value()));
                // Reply with an Ack confirming that the put operation was
                // successful
                Ack* a = new Ack();
                const char* msg = a->serialize();
                exit_if_not(
                    send(fd, msg, strlen(msg) + 1, 0) > 0, 
                    "Call to send() failed"
                );
                delete a; delete msg;
            }
            case MsgKind::Get: {
                Get* g = m->as_get();
                // Ensure that this message was sent to the right node
                exit_if_not(g->get_key()->get_home_node() == idx_,
                    "Put was sent to incorrect node");
                DataFrame* res = get(*(g->get_key()));
                // Send back a Reply with the data
                Reply r(res);
                const char* msg = r.serialize();
                exit_if_not(
                    send(fd, msg, strlen(msg) + 1, 0) > 0, 
                    "Call to send() failed"
                );
                delete msg;
            }
            case MsgKind::WaitAndGet: {
                WaitAndGet* wag = m->as_wait_and_get();
                // Ensure that this message was sent to the right node
                exit_if_not(wag->get_key()->get_home_node() == idx_,
                    "Put was sent to incorrect node");
                DataFrame* res = wait_and_get(*(wag->get_key()));
                // Send back a Reply with the data
                Reply r(res);
                const char* msg = r.serialize();
                exit_if_not(
                    send(fd, msg, strlen(msg) + 1, 0) > 0, 
                    "Call to send() failed"
                );
                delete msg;
            }
        }
    }

    /**
     * Client function
     * Parse the directory message sent from the server.
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
     * Create a socket to the client at the given IP, connect to it, and send it a Register message.
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
     * Empties the fd/ip map and closes every fd
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
     * The IP address of a node is 127.0.0.x where x is the node index + 1.
     * So given a node index, this function returns the corresponding IP address.
     */
    char* idx_to_ip_(size_t idx) {
        char* ip = new char[INET_ADDRSTRLEN + 1];
        int bytes = sprintf(ip, "127.0.0.%d", idx + 1);
        exit_if_not(bytes > INET_ADDRSTRLEN, "Invalid index");
        return ip;
    }
};

/**
 * Builds a DataFrame with one column containing the data in the given int array, adds the
 * DataFrame to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromIntArray(Key* k, KVStore* kv, size_t size, int* vals) {
    Column* col = new IntColumn();
    for (int i = 0; i < size; i++) {
        col->push_back(vals[i]);
    }
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the data in the given bool array, adds the
 * DataFrame to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromBoolArray(Key* k, KVStore* kv, size_t size, bool* vals) {
    Column* col = new BoolColumn();
    for (int i = 0; i < size; i++) {
        col->push_back(vals[i]);
    }
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the data in the given float array, adds the
 * DataFrame to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromFloatArray(Key* k, KVStore* kv, size_t size, float* vals) {
    Column* col = new FloatColumn();
    for (int i = 0; i < size; i++) {
        col->push_back(vals[i]);
    }
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the data in the given string array, adds
 * the DataFrame to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromStringArray(Key* k, KVStore* kv, size_t size, String** vals) {
    Column* col = new StringColumn();
    for (int i = 0; i < size; i++) {
        col->push_back(vals[i]);
    }
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the single given int, adds the DataFrame
 * to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromIntScalar(Key* k, KVStore* kv, int val) {
    Column* col = new IntColumn();
    col->push_back(val);
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the single given bool, adds the DataFrame
 * to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromBoolScalar(Key* k, KVStore* kv, bool val) {
    Column* col = new BoolColumn();
    col->push_back(val);
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the single given float, adds the DataFrame
 * to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromIntScalar(Key* k, KVStore* kv, float val) {
    Column* col = new FloatColumn();
    col->push_back(val);
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}

/**
 * Builds a DataFrame with one column containing the single given string, adds the DataFrame
 * to the given KVStore at the given Key, and then returns the DataFrame.
 */
DataFrame* DataFrame::fromStringScalar(Key* k, KVStore* kv, String* val) {
    Column* col = new StringColumn();
    col->push_back(val);
    DataFrame* res = new DataFrame();
    res->add_column(col);
    kv->put(*k, *res);
    return res;
}