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
#include <mutex>
#include <vector>

#include "map.h"
// #include "dataframe.h"
#include "serial.h"

#define PORT "8080"
// The fixed number of nodes that this network supports (1 server, the rest are clients)
#define BACKLOG 6
// The size of the string buffer used to send messages
#define BUF_SIZE (size_t)100000

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
    // Data returned in a Reply message after a Get message is sent
    DataFrame* reply_data_;
    // Data returned in a Reply message after a WaitAndGet message is sent
    // WaitAndGet gets its own variable so that there is no confusion between threads running both
    // get operations
    DataFrame* wag_reply_data_;
    // Vector of threads that process messages
    std::vector<std::thread>* threads_;
    // The lock that prevents data races
    std::mutex mtx_;

    /**
     * Constructor that initializes an empty KVStore.
     */
    KVStore(size_t idx) : idx_(idx), ack_recvd_(false), reply_data_(nullptr), 
        wag_reply_data_(nullptr) {
        map_ = new Map();
        threads_ = new std::vector<std::thread>();
        startup();
        // Wait a second for client registration to finish
        sleep(1);
    }

    /**
     * Destructor
     */
    ~KVStore() {
        for (std::thread& th : *threads_) {
            if (th.joinable()) {
                th.join();
            }
        }
        delete threads_;
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
        // Check if the key corresponds to this node
        if (dst_node == idx_) {
            // If so, put the data in this KVStore's map
            const char* serial_df = v.serialize();
            mtx_.lock();
            map_->put(k.get_keystring(), new String(serial_df));
            mtx_.unlock();
            // printf("\033[1;3%zumNode %zu: Put was successful\033[0m\n", idx_, idx_);
            delete[] serial_df;
        } else {
            // If not, send a Put message to the correct node
            Put p(&k, &v);
            // printf("\033[1;3%zumNode %zu: Sending Put\033[0m\n", idx_, idx_);
            send_to_node(p.serialize(), dst_node);
            // Wait for an Ack confirming that the data was stored successfully
            // printf("\033[1;3%zumNode %zu: Waiting for Ack\033[0m\n", idx_, idx_);
            while (!ack_recvd_) sleep(1);
            // printf("\033[1;3%zumNode %zu: Done waiting for Ack\033[0m\n", idx_, idx_);
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
        DataFrame* res;
        // Check if this key corresponds to this node
        if (dst_node == idx_) {
            // If so, get the data from this KVStore's map
            mtx_.lock();
            String* serialized_df = dynamic_cast<String*>(map_->get(k.get_keystring()));
            mtx_.unlock();
            assert(serialized_df != nullptr);
            
            Deserializer ds(serialized_df->c_str());
            res = dynamic_cast<DataFrame*>(ds.deserialize());
            assert(res != nullptr);
        } else {
            // If not, send a Get message to the correct node
            Get g(&k);
            // printf("\033[1;3%zumNode %zu: Sending Get\033[0m\n", idx_, idx_);
            send_to_node(g.serialize(), dst_node);
            // Wait for a reply with the desired data
            // printf("\033[1;3%zumNode %zu: Waiting for Reply\033[0m\n", idx_, idx_);
            while (reply_data_ == nullptr) sleep(1);
            // printf("\033[1;3%zumNode %zu: Done waiting for Reply\033[0m\n", idx_, idx_);
            res = reply_data_;
            reply_data_ = nullptr;
        }
        return res;
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
            String* key_string = k.get_keystring();
            bool contains_key = map_->containsKey(key_string);
            // printf("\033[1;3%zumNode %zu: Waiting for put\033[0m\n", idx_, idx_);
            while (!contains_key) {
                sleep(1);
                contains_key = map_->containsKey(key_string);
            }
            // printf("\033[1;3%zumNode %zu: Done waiting for put\033[0m\n", idx_, idx_);
            return get(k);
        } else {
            // If not, send a WaitAndGet message to the correct node
            WaitAndGet wag(&k);
            // printf("\033[1;3%zumNode %zu: Sending WaitAndGet\033[0m\n", idx_, idx_);
            send_to_node(wag.serialize(), dst_node);
            // Wait for a reply with the desired data
            // printf("\033[1;3%zumNode %zu: Waiting for Reply\033[0m\n", idx_, idx_);
            while (wag_reply_data_ == nullptr) sleep(1);
            // printf("\033[1;3%zumNode %zu: Done waiting for Reply\033[0m\n", idx_, idx_);
            DataFrame* ret = wag_reply_data_;
            wag_reply_data_ = nullptr;
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
    // has this node shut down?
    bool has_shutdown;

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
        has_shutdown = false;
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
            // printf("\033[1;3%zumNode %zu: Server is up.\033[0m\n", idx_, idx_);
        } else {
            // Wait a second for the server to start up
            sleep(1);
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
            // Send IP to server in a Register message
            Register reg(new String(ip_), idx_);
            const char* msg = reg.serialize();
            // printf("\033[1;3%zumNode %zu: Sending my IP \"%s\" to the server for registration.\033[0m\n", idx_, idx_, ip_);
            exit_if_not(send(servfd_, msg, strlen(msg) + 1, 0) > 0, "Sending IP to server failed");
        }
        // Start listening for incoming messages
        threads_->push_back(std::thread(&KVStore::monitor_sockets_, this));
    }

    /**
     * Shutdown protocol for a Client.
     * Closes the Client's sockets and deletes all fields.
     */
    void shutdown() {
        // printf("\033[1;3%zumNode %zu: Shutting down\033[0m\n", idx_, idx_);
        has_shutdown = true;
        if (is_server()) {
            delete directory_;
        } else {
            close(servfd_);
        }
        clear_map_();
        close(fd_);
        delete[] ip_;
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
                // printf("\033[1;3%zumNode %zu: Sending msg to node %d\033[0m\n", idx_, idx_, i);
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
        int fd = nodes_[dst];
        if (fd == -1) {
            // printf("\033[1;3%zumNode %zu: Could not find a node with the given index\033[0m\n", idx_, idx_);
        } else {
            exit_if_not(send(fd, msg, strlen(msg) + 1, 0) > 0, "Sending msg to other client failed");
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
        // The timeout argument for select()
        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;
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
            if (select(fdmax_ + 1, &read_fds_, NULL, NULL, &tv) < 0) return;
            for (int i = 0; i <= fdmax_; i++) {
                // In case shutdown() was called from the other thread
                if (has_shutdown) return;
                if (FD_ISSET(i, &read_fds_)) {
                    // Found a connection
                    if (i == fd_) {
                        if (num_nodes + 1 > BACKLOG) {
                            // The network is full, do not accept this connection
                            // printf("\033[1;3%zumNode %zu: Network is full, cannot accept new connection.\033[0m\n", idx_, idx_);
                            continue;
                        }
                        num_nodes++;
                        // The node is receiving a new connection from another one
                        addrlen = sizeof(their_addr_);
                        // Accept the connection and add the new socket fd to the master list
                        exit_if_not((their_fd_ = accept(fd_, (struct sockaddr*)&their_addr_, &addrlen)) >= 0, 
                            "Call to accept() failed");
                        FD_SET(their_fd_, &master_);
                        // Update the max fd value
                        if (their_fd_ > fdmax_) {
                            fdmax_ = their_fd_;
                        }
                        // printf("\033[1;3%zumNode %zu: New connection on socket %d\033[0m\n", idx_, idx_, their_fd_);
                    } else {
                        // The Client is receving a message
                        if ((nbytes = recv(i, buffer_, BUF_SIZE, 0)) <= 0) {
                            // Connection to the other node was closed or there was an error,
                            // so shut down
                            shutdown();
                            return;
                        } else {
                            // Figure out what kind of message was received
                            Deserializer ds(buffer_);
                            Message* m = dynamic_cast<Message*>(ds.deserialize());
                            assert(m != nullptr);
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
                // printf("\033[1;3%zumNode %zu: Received Directory\033[0m\n", idx_, idx_);
                // Client received directory update from server
                process_directory_(m->as_directory());
                break;
            }
            case MsgKind::Register: {
                // printf("\033[1;3%zumNode %zu: Received Register\033[0m\n", idx_, idx_);
                Register* r = m->as_register();
                char* new_ip = r->get_ip()->c_str();
                size_t new_idx = r->get_sender();
                if (is_server()) {
                    // A client is registering with the server
                    // printf("\033[1;3%zumNode %zu: Received IP \"%s\" from new client at socket %d\033[0m\n", idx_, idx_, new_ip, fd);
                    // Add the new IP to the directory
                    directory_->add_client(new_ip, new_idx);
                    // Keep track of the new client's node index and socket fd
                    nodes_[new_idx] = fd;
                    // Send the updated directory back to the client
                    const char* serial_directory = directory_->serialize();
                    // printf("\033[1;3%zumNode %zu: Sending updated directory back to client\033[0m\n", idx_, idx_);
                    exit_if_not(send(fd, serial_directory, strlen(serial_directory) + 1, 0) > 0,
                        "Call to send() failed");
                    delete[] serial_directory;
                } else {
                    // Another client sent a Register, so keep track of its fd 
                    // and node index
                    nodes_[new_idx] = fd;
                }
                // print_map_();
                break;
            }
            case MsgKind::Ack: {
                // printf("\033[1;3%zumNode %zu: Received Ack\033[0m\n", idx_, idx_);
                // Set the value that put() is waiting for above
                ack_recvd_ = true;
                break;
            }
            case MsgKind::Reply: {
                // printf("\033[1;3%zumNode %zu: Received Reply\033[0m\n", idx_, idx_);
                Reply* rep = m->as_reply();
                MsgKind req = rep->get_request();
                // Set the value that get() and wait_and_get() wait for above
                if (req == MsgKind::WaitAndGet) {
                    wag_reply_data_ = rep->get_value();
                } else {
                    reply_data_ = rep->get_value();
                }
                break;
            }
            case MsgKind::Put: {
                threads_->push_back(
                    std::thread(&KVStore::process_put_, this, m->as_put(), std::ref(fd)));
                break;
            }
            case MsgKind::Get: {
                threads_->push_back(
                    std::thread(&KVStore::process_get_, this, m->as_get(), std::ref(fd)));
                break;
            }
            case MsgKind::WaitAndGet: {
                threads_->push_back(
                    std::thread(&KVStore::process_wag_, this, m->as_wait_and_get(), std::ref(fd)));
                // printf("\033[1;3%zumNode %zu: Continuing\033[0m\n", idx_, idx_);
                break;
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
            char* ip = ips->get(i)->c_str();
            if (strcmp(ip, ip_) != 0) connect_to_client_(ip, indices->get(i));
        }
    }

    /**
     * Processes the given Put message.
     * 
     * @param p  The message
     * @param fd The socket fd to send the Ack back to
     */
    void process_put_(Put* p, int fd) {
        // printf("\033[1;3%zumNode %zu: Received Put\033[0m\n", idx_, idx_);
        // Ensure that this message was sent to the right node
        exit_if_not(p->get_key()->get_home_node() == idx_, "Put was sent to incorrect node");
        put(*(p->get_key()), *(p->get_value()));

        // Reply with an Ack confirming that the put operation was successful
        Ack* a = new Ack();
        const char* msg = a->serialize();
        exit_if_not(send(fd, msg, strlen(msg) + 1, 0) > 0, "Call to send() failed");
        delete a;
    }

    /**
     * Starts the get operation in a separate thread
     */
    void process_get_(Get* g, int fd) {
        // printf("\033[1;3%zumNode %zu: Received Get\033[0m\n", idx_, idx_);
        // Ensure that this message was sent to the right node
        exit_if_not(g->get_key()->get_home_node() == idx_, "Put was sent to incorrect node");
        DataFrame* res = get(*(g->get_key()));

        // Send back a Reply with the data
        Reply r(res, MsgKind::Get);
        const char* msg = r.serialize();
        exit_if_not(send(fd, msg, strlen(msg) + 1, 0) > 0, "Call to send() failed");
        delete[] msg;
    }

    /**
     * Starts the wait_and_get operation in a separate thread
     */
    void process_wag_(WaitAndGet* wag, int fd) {
        // printf("\033[1;3%zumNode %zu: Received WaitAndGet\033[0m\n", idx_, idx_);
        // Ensure that this message was sent to the right node
        exit_if_not(wag->get_key()->get_home_node() == idx_, "Put was sent to incorrect node");
        DataFrame* res = wait_and_get(*(wag->get_key()));

        // Send back a Reply with the data
        Reply r(res, MsgKind::WaitAndGet);
        const char* msg = r.serialize();
        exit_if_not(send(fd, msg, strlen(msg) + 1, 0) > 0, "Call to send() failed");
        delete[] msg;
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
        const char* msg = reg.serialize();
        // printf("\033[1;3%zumNode %zu: Sending Register to other client at %s, socket %d\033[0m\n", idx_, idx_, ip, client_fd);
        exit_if_not(send(client_fd, msg, strlen(msg) + 1, 0) > 0, "Sending greeting to other client failed");
        // Add the fd to the master list
        FD_SET(client_fd, &master_);
        // Update the max fd value
        if (client_fd > fdmax_) {
            fdmax_ = client_fd;
        }
        // Keep track of the client's fd and node index
        nodes_[idx] = client_fd;
        // print_map_();
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
        // printf("\033[1;3%zumNode %zu: Map: \033[0m", idx_, idx_);
        for (int i = 0; i < BACKLOG; i++) {
            int fd = nodes_[i];
            if (fd != -1) {
                // printf("\033[1;3%zum{fd: %d, idx: %d} \033[0m", idx_, fd, i);
            }
        }
        // printf("\n");
    }

    /**
     * The IP address of a node is 127.0.0.x where x is the node index + 1.
     * So given a node index, this function returns the corresponding IP address.
     */
    char* idx_to_ip_(size_t idx) {
        char* ip = new char[INET_ADDRSTRLEN + 1];
        int bytes = sprintf(ip, "127.0.0.%zu", idx + 1);
        exit_if_not(bytes <= INET_ADDRSTRLEN + 1, "Invalid index");
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