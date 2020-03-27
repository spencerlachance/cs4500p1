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
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */
enum class MsgKind { Ack, Put, Reply, Get, WaitAndGet, Register, Directory };
 
/**
 * An abstract class for messages
 * 
 * @author Spencer LaChance <lachance.s@northeastern.edu>
 * @author David Mberingabo <mberingabo.d@husky.neu.edu>
 */ 
class Message : public Object {
    public:
        MsgKind kind_;  // the message kind
        
        /* A method for checking message class equality */
        virtual bool equals(Object* other) {
            Message* other_m = dynamic_cast<Message*>(other);
            if (other_m == nullptr) return false;
            return other_m->kind() == kind_; // Member fields equality
        }

        /* Returns this message's kind */
        MsgKind kind() { return kind_; }
};
 

class Ack : public Message {
    public:
        
        /* Constructor. */
        Ack() {
            kind_ = MsgKind::Ack;
        }

        /* Returns a serialized representation of this acknowledge. */
        const char* serialize() {
            return "{type: ack}";
        }
};
 
class Register : public Message {
    public:
        // The IP address of the node that sent this message
        String* ip_;
        // The index of the node that sent this message
        size_t sender_;

        /* Constructor */
        Register(String* ip, size_t sender) : ip_(ip), sender_(sender) {
            kind_ = MsgKind::Register;
        }

        /* Destructor */
        ~Register() {
            delete ip_;
        }

        bool equals(Object* other) {
            Register* o = dynamic_cast<Register*>(other);
            if (o == nullptr) return false;
            return o->get_ip()->equals(ip_) && o->kind() == kind_ && o->get_sender() == sender_;
        }

        /* Returns this register's ip */
        String* get_ip() { return ip_; }

        /* Returns this register's sender idx */
        size_t get_sender() { return sender_; }

        /* Returns a serial representation of this register. */
        const char* serialize() {
            StrBuff* buff = new StrBuff();
            const char* serial_ip = ip_->serialize();

            buff->c("{type: register, ip: ");
            buff->c(serial_ip);
            buff->c(", sender: ");
            buff->c(sender_);
            buff->c("}");

            String* serial_str = buff->get();
            // Copying the char* so we can delete the String* returned from get()
            char* copy = new char[serial_str->size() + 1];
            strcpy(copy, serial_str->c_str());
            delete buff;
            delete serial_str;
            delete[] serial_ip;
            return copy;
        }
};
 
class Directory : public Message {
   public:
        // Clients' IP addresses
        Vector* addresses_; //owned
        // Clients' node indices
        IntVector* indices_;
        
        /* Constructor */
        Directory(Vector* addresses, IntVector* indices) : 
            addresses_(addresses), indices_(indices) {
            kind_ = MsgKind::Directory;
        }

        /* Constructor that assigns empty array for ports and addresses */
        Directory() {
            kind_ = MsgKind::Directory;
            addresses_ = new Vector();
            indices_ = new IntVector();
        }

        /* Destructor */
        ~Directory() {
            delete addresses_;
            delete indices_;
        }

        /**
         * Adds a client to the directory.
         * 
         * @param adr The client's IP address
         * @param idx The client's node index
         */
        void add_client(const char* adr, size_t idx) {
            String* s = new String(adr);
            addresses_->append(s);
            indices_->append(idx);
        }

        /* Is this directory equal to the given object? */
        bool equals(Object* other) {
            Directory* o = dynamic_cast<Directory*>(other);
            if (o == nullptr) return false;
            return o->get_addresses()->equals(addresses_) && o->kind() == kind_ &&
                o->get_indices()->equals(indices_);
        }

        /* Returns this directory's addresses field */
        Vector* get_addresses() { return addresses_; }

        /* Returns this directory's indices field */
        IntVector* get_indices() { return indices_; }

        /* Returns a serialized representation of this directory message */
        const char* serialize() {
            StrBuff* buff = new StrBuff(); // Use this buffer to build serial representation
            const char* serial_vec = addresses_->serialize();
            const char* serial_ivec = indices_->serialize();

            buff->c("{type: directory, addresses: ");
            buff->c(serial_vec);
            buff->c(", indices: ");
            buff->c(serial_ivec);
            buff->c("}");

            String* serial_str = buff->get();
            // Copying the char* so we can delete the String* returned from get()
            char* copy = new char[serial_str->size() + 1];
            strcpy(copy, serial_str->c_str());
            delete buff;
            delete serial_str;
            delete[] serial_vec;
            return copy;
        }

        /**
         * Empties the directory
         */
        void clear() {
            delete addresses_;
            delete indices_;
            addresses_ = new Vector();
            indices_ = new IntVector();
        }
};

// class Put : public Message {
// public:
//     Key k_;
//     DataFrame v_;
// };

// class Get : public Message {
// public:
//     Key k_;
// };

// class WaitAndGet : public Message {
// public:
//     Key k_;
// };

// class Reply : public Message {
// public:
//     DataFrame v_;
// };