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
        String* ip_;

        /* Constructor */
        Register(String* ip) {
            kind_ = MsgKind::Register;
            ip_ = ip;
        }

        /* Destructor */
        ~Register() {
            delete ip_;
        }

        bool equals(Object* other) {
            Register* o = dynamic_cast<Register*>(other);
            if (o == nullptr) return false;
            return o->get_ip()->equals(ip_) && o->kind() == kind_;
        }

        /* Returns this register's ip */
        String* get_ip() { return ip_; }

        /* Returns a serial representation of this register. */
        const char* serialize() {
            StrBuff* buff = new StrBuff();
            const char* serial_ip = ip_->serialize();
            buff->c("{type: register, ip: ");
            buff->c(serial_ip);
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
        Vector* addresses_; //owned
        
        /* Constructor */
        Directory(Vector* addresses) {
            kind_ = MsgKind::Directory;
            addresses_ = addresses;
        }

        /* Constructor that assigns empty array for ports and addresses */
        Directory() {
            kind_ = MsgKind::Directory;
            addresses_ = new Vector();
        }

        /* Destructor */
        ~Directory() {
            delete addresses_;
        }

        /* Adds an address to the array of addresses */
        void add_adr(const char* adr) {
            String* s = new String(adr);
            addresses_->append(s);
        }

        /* Is this directory equal to the given object? */
        bool equals(Object* other) {
            Directory* o = dynamic_cast<Directory*>(other);
            if (o == nullptr) return false;
            return (o->get_addresses()->equals(addresses_)) && (o->kind() == kind_);
        }

        /* Returns this directory's addresses field */
        Vector* get_addresses() { return addresses_; }

        /* Returns a serialized representation of this directory message */
        const char* serialize() {
            StrBuff* buff = new StrBuff(); // Use this buffer to build serial representation
            const char* serial_vec = addresses_->serialize();

            buff->c("{type: directory, addresses: ");
            buff->c(serial_vec);
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
            addresses_ = new Vector();
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