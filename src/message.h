//lang:CwC

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "string.h"
#include "vector.h"

/**
 * An enum for the different kinds of messages.
 * 
 * @author Spencer LaChance <lachance.s@husky.neu.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
enum class MsgKind { Ack, Nack, Put,
                    Reply,  Get, WaitAndGet, Status,
                    Kill,   Register,  Directory };
 
/**
 * An abstract class for messages
 * 
 * @author Spencer LaChance <lachance.s@husky.neu.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */ 
class Message : public Object {
    public:
        MsgKind kind_;  // the message kind
        size_t sender_; // the index of the sender node
        size_t target_; // the index of the receiver node
        size_t id_;     // an id t unique within the node
        
        /* A method for checking message class equality */
        virtual bool equals(Object* other) {
            if (other == this) return true; // Reference equality
            Message* other_m = dynamic_cast<Message*>(other);
            if (other_m == nullptr) return false;
            return (other_m->kind() == kind_) && (other_m->sender() == sender_) && (other_m->target() == target_) && (other_m->id() == id_); // Member fields equality
        }

        /* Returns this message's kind */
        MsgKind kind() { return kind_; }

        /* Returns this message's sender */
        size_t sender() { return sender_; }

        /* Returns this message's target */
        size_t target() { return target_; }

        /* Returns this message's id_ */
        size_t id() { return id_; }

        /* Returns a serialized representation of this message. */
        const char* serialize_msg() {
            char* str = new char[sizeof(size_t)];
            StrBuff* buff = new StrBuff();

            if (kind_ == MsgKind::Ack) {
                buff->c("{type: ack, sender: ");
            } else if (kind_ == MsgKind::Status) {
                buff->c("{type: status, sender: ");
            } else if (kind_ == MsgKind::Directory) {
                buff->c("{type: directory, sender: ");
            } else if (kind_ == MsgKind::Register) {
                buff->c("{type: register, sender: ");
            }
            
            snprintf(str, sizeof(size_t), "%zu", sender_);
            buff->c(str); // Add sender to buff->
            delete[] str;

            str = new char[sizeof(size_t)];
            buff->c(", target: ");
            snprintf(str, sizeof(size_t), "%zu", target_);
            buff->c(str); // Add target to buff->
            delete[] str;

            str = new char[sizeof(size_t)];
            buff->c(", id: ");
            snprintf(str, sizeof(size_t), "%zu", id_);
            buff->c(str); // Add id to buff->
            delete[] str;

            return buff->get()->c_str();
        }
};
 

class Acknowledge : public Message {
    public:
        
        /* Constructor. */
        Acknowledge(size_t sender, size_t target, size_t id) {
            kind_ = MsgKind::Ack;
            id_ = id;
            sender_ = sender;
            target_ = target;
        }

        /* Returns a serialized representation of this acknowledge. */
        const char* serialize() {
            StrBuff* buff = new StrBuff();
            const char* serial_msg = serialize_msg();
            buff->c(serial_msg);
            buff->c("}");
            return buff->get()->c_str();
        }
};
 
class Status : public Message {
    public:
        String* msg_; // owned

        /* Constructor */
        Status(size_t sender, size_t target, size_t id, String* msg) {
            kind_ = MsgKind::Status;
            id_ = id;
            sender_ = sender;
            target_ = target;
            msg_ = msg;
        }

        /* Returns a serialized representation of this status message. */
        // TODO: Use helper.
        const char* serialize() {
            char* str = new char[sizeof(size_t)];
            StrBuff* buff = new StrBuff();

            buff->c("{type: status, sender: ");
            snprintf(str, sizeof(size_t), "%zu", sender_);
            buff->c(str); // Add sender to buff->
            delete[] str;

            str = new char[sizeof(size_t)];
            buff->c(", target: ");
            snprintf(str, sizeof(size_t), "%zu", target_);
            buff->c(str); // Add target to buff->
            delete[] str;

            str = new char[sizeof(size_t)];
            buff->c(", id: ");
            snprintf(str, sizeof(size_t), "%zu", id_);
            buff->c(str); // Add id to buff->
            delete[] str;

            buff->c(", msg: ");
            buff->c(msg_->c_str()); // Add the message to buff->

            buff->c("}");
            return buff->get()->c_str();
        }

        bool equals(Object* other) {
            if (other == this) return true; // Reference equality.
            Status* o = dynamic_cast<Status*>(other);
            if (o == nullptr) return false;
            return (o->get_msg()->equals(get_msg())) && (o->kind() == kind_) && (o->sender() == sender_) && (o->target() == target_) && (o->id() == id_); // Member fields equality
        }

        /* Returns a copy of this messages msg field. */
        String* get_msg() {
            char* c = msg_->c_str();
            String* return_value = new String(c);
            return return_value;
        }
};
 
class Register : public Message {
    public:
        struct sockaddr_in node_;
        size_t port_;

        /* Constructor */
        Register(size_t sender, size_t target, size_t id, struct sockaddr_in node, size_t port) {
            kind_ = MsgKind::Register;
            sender_ = sender;
            target_ = target;
            id_ = id;
            node_ = node;
            port_ = port;
        }

        bool equals(Object* other) {
            if (other == this) return true; // Reference equality.
            Register* o = dynamic_cast<Register*>(other);
            if (o == nullptr) return false;
            struct sockaddr_in other_node = o->get_node();
            if (other_node.sin_addr.s_addr != node_.sin_addr.s_addr) { return false; }
            if (other_node.sin_family != node_.sin_family) { return false; }
            if (other_node.sin_port != node_.sin_port) { return false; }
            return (o->get_port() == port_) && (o->kind() == kind_) && (o->sender() == sender_) && (o->target() == target_);
        }

        /* Returns this register's node */
        sockaddr_in get_node() { return node_; }

        /* Returns this register's port */
        size_t get_port() { return port_; }

        /* Returns a serial representation of this register's node */
        StrBuff* serialize_node(StrBuff* buff) {
            buff->c("[");
            char x [sizeof(int)*8+1];
            sprintf(x, "%hi", node_.sin_family);
            buff->c(x);

            buff->c(",");
            x [sizeof(int)*8+1];
            sprintf(x, "%u", node_.sin_port);
            buff->c(x);

            buff->c(",");
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(node_.sin_addr), ip, INET_ADDRSTRLEN);
            buff->c(ip);

            buff->c("]");

            return buff;
        }

        /* Returns a serial representation of this register. */
        const char* serialize() {
            const char* serial_msg = serialize_msg(); // serialize the type, sender, target and id.
            StrBuff* buff = new StrBuff();
            buff->c(serial_msg);
            buff->c(", node: ");
            buff = serialize_node(buff);
            buff->c(", port: ");
            char temp[sizeof(size_t) + 1];
            sprintf(temp, "%zu", port_);
            buff->c(temp);
            buff->c("}");

            return buff->get()->c_str();
        }
};
 
class Directory : public Message {
   public:
        size_t nodes_;
        IntVector* ports_;  // owned
        // String ** addresses_;  // owned; strings
        Vector* addresses_; //owned; strings
        
        /* Constructor */
        Directory(size_t sender, size_t target, size_t id, size_t nodes, IntVector* ports, Vector* addresses){
            kind_ = MsgKind::Directory;
            id_ = id;
            sender_ = sender;
            target_ = target;
            nodes_ = nodes;
            ports_ = ports;
            addresses_ = addresses;
        }

        /* Constructor that assigns empty array for ports and addresses */
        Directory(size_t sender, size_t target, size_t id, size_t nodes){
            kind_ = MsgKind::Directory;
            id_ = id;
            sender_ = sender;
            target_ = target;
            nodes_ = nodes;
            ports_ = new IntVector();
            addresses_ = new Vector();
        }

        /* Destructor */
        ~Directory() {
            delete ports_;
            delete addresses_;
        }

        /* Adds an address to the array of addresses */
        void add_adr(const char* adr) {
            String* s = new String(adr);
            addresses_->append(s);
        }

        /* Adds a port to the array of ports */
        void add_port(int port) {
            ports_->append(port);
        }

        /* Is this directory equal to the given object? */
        bool equals(Object* other) {
            if (other == this) return true; // Reference equality.
            Directory* o = dynamic_cast<Directory*>(other);
            if (o == nullptr) return false;
            return (o->get_addresses()->equals(addresses_)) && (o->get_ports()->equals(ports_)) && (o->get_nodes() == nodes_) && (o->kind() == kind_) && (o->sender() == sender_) && (o->target() == target_);
        }

        /* Returns this directory's nodes field */
        size_t get_nodes() { return nodes_; }

        /* Returns this directory's ports field */
        IntVector* get_ports() { return ports_; }

        /* Returns this directory's addresses field */
        Vector* get_addresses() { return addresses_; }

        /* Returns a serialized representation of this directory message */
        const char* serialize() {
            StrBuff* buff = new StrBuff(); // Use this buffer to build serial representation

            char* str = new char[sizeof(size_t)];
            buff->c("{type: directory, sender: ");
            snprintf(str, sizeof(size_t), "%zu", sender_);
            buff->c(str); // Add sender to buff->
            delete[] str;

            str = new char[sizeof(size_t)];
            buff->c(", target: ");
            snprintf(str, sizeof(size_t), "%zu", target_);
            buff->c(str); // Add target to buff->
            delete[] str;

            str = new char[sizeof(size_t)];
            buff->c(", id: ");
            snprintf(str, sizeof(size_t), "%zu", id_);
            buff->c(str); // Add id to buff->
            delete[] str;

            str = new char[sizeof(size_t)];
            buff->c(", nodes: ");
            snprintf(str, sizeof(size_t), "%zu", nodes_);
            buff->c(str); // Add nodes to buff->
            delete[] str;
            
            buff->c(", ports: ["); 
            // Adding each port to buff
            for (int i = 0; i < ports_->size(); i++) {
                str = new char[6];
                size_t port = ports_->get(i);
                buff->c(port); // Add port to buff->
                delete[] str;
                if (i != (ports_->size() - 1)) {
                    buff->c(",");
                }
            }
            buff->c("]"); // ports end here

            buff->c(", addresses: [");
            // Adding each address to buff
            for (int i = 0; i < addresses_->size(); i++) {
                String* s_adr = dynamic_cast<String*>(addresses_->get(i));
                assert(s_adr != nullptr);
                char* adr = s_adr->c_str();
                buff->c(adr);
                if (i != (addresses_->size() - 1)) {
                    buff->c(",");
                }
            }

            buff->c("]}");
            return buff->get()->c_str();
        }

};