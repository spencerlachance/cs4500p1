//lang:CwC

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "string.h"
#include "vector.h"
#include "key.h"

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
            StrBuff buff;
            const char* serial_ip = ip_->serialize();
            buff.c("{type: register, ip: ");
            buff.c(serial_ip);
            buff.c("}");

            String* serial_str = buff.get();
            // Copying the char* so we can delete the String* returned from get()
            char* copy = new char[serial_str->size() + 1];
            strcpy(copy, serial_str->c_str());
            
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
            StrBuff buff; // Use this buffer to build serial representation
            const char* serial_vec = addresses_->serialize();

            buff.c("{type: directory, addresses: ");
            buff.c(serial_vec);
            buff.c("}");

            String* serial_str = buff.get();
            // Copying the char* so we can delete the String* returned from get()
            char* copy = new char[serial_str->size() + 1];
            strcpy(copy, serial_str->c_str());
            
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

/* Put is a message subclass used to store a (DataFrame) at a key. */
class Put : public Message {
    public:
        Key* k_;
        DataFrame* v_;

        /* Constructor, takes ownership of k and v. */
        Put(Key* k, DataFrame* v) {
            kind_ = MsgKind::Put;
            k_ = k;
            v_ = v;
        }

        /* Destructor */
        ~Put() {
            delete k_;
            delete v_;
        }

        /* Returns this put message's key */
        Key* get_key() { return k_; }

        /* Returns this put message's value (DataFrame) */
        DataFrame* get_value() { return v_; }

        /* Returns a serialized representation of this put message */
        const char* serialize() {
            StrBuff buff;
            buff.c("{type: put, key: ");
            const char* serial_k = k_->serialize();
            buff.c(serial_k);
            buff.c(", value: ");
            const char* serial_v = v_->serialize();
            buff.c(serial_v);
            buff.c("}");
            String* serial_str = buff.get();
            // Copying the char* so we can delete the String* returned from get()
            char* copy = new char[serial_str->size() + 1];
            strcpy(copy, serial_str->c_str());

            delete serial_str;
            delete[] serial_k;
            delete[] serial_v;
            return copy;
        }

        /* Return true if this put message equals the given object, and false if not. */
        bool equals(Object* o) {
            Put* other = dynamic_cast<Put*>(o);
            if (other == nullptr) return false;
            return (other->get_key()->equals(k_)) && (other->get_value()->equals(v_));
        }
};

/**
 *  Get is a Message subclass that is used to request a value using a key. 
 */
class Get : public Message {
    public:
        Key* k_;

        /* Constructor, takes ownership of the given Key */
        Get(Key* k) {
            kind_ = MsgKind::Get;
            k_ = k;
        }

        /* Desrtuctor */
        ~Get() {
            delete k_;
        }

        /* Return this Get message's key */
        Key* get_key() {
            return k_;
        }

        /* Returns a serialized representation of this get message */
        const char* serialize() {
            StrBuff buff;
            buff.c("{type: get, key: ");
            const char* serialized_k = k_->serialize();
            buff.c(serialized_k);
            buff.c("}");
            String* serial_str = buff.get();
            // Copying the char* so we can delete the String* returned from get()
            char* copy = new char[serial_str->size() + 1];
            strcpy(copy, serial_str->c_str());

            delete[] serialized_k;
            delete serial_str;
            return copy;
        }

        /* Return true if this get message equals the given object, and false if not. */
        bool equals(Object* o) {
            Get* other = dynamic_cast<Get*>(o);
            if (other == nullptr) return false;
            return (other->get_key()->equals(k_));
        }
};

/**
 * WaitAndGet is a Message subclass that is used to request a value using a key.
 */
class WaitAndGet : public Message {
    public:
        Key* k_;

        /* Constructor, takes ownership of the given Key */
        WaitAndGet(Key* k) {
            kind_ = MsgKind::WaitAndGet;
            k_ = k;
        }

        /* Desrtuctor */
        ~WaitAndGet() {
            delete k_;
        }

        /* Return this Get message's key */
        Key* get_key() {
            return k_;
        }

        /* Returns a serialized representation of this WaitAndGet message */
        const char* serialize() {
            StrBuff buff;
            buff.c("{type: wait_get, key: ");
            const char* serialized_k = k_->serialize();
            buff.c(serialized_k);
            buff.c("}");
            String* serial_str = buff.get();
            // Copying the char* so we can delete the String* returned from get()
            char* copy = new char[serial_str->size() + 1];
            strcpy(copy, serial_str->c_str());

            delete[] serialized_k;
            delete serial_str;
            return copy;
        }

        /* Return true if this get message equals the given object, and false if not. */
        bool equals(Object* o) {
            WaitAndGet* other = dynamic_cast<WaitAndGet*>(o);
            if (other == nullptr) return false;
            return (other->get_key()->equals(k_));
        }
    
};

/**
 * Reply is a subclass of Message that contains a DataFrame value;
 */
class Reply : public Message {
public:
    DataFrame* v_;

    /* Constructor, takes ownership over v */
    Reply(DataFrame* v) {
        kind_ = MsgKind::Reply;
        v_ = v;
    } 

    ~Reply() {
        delete v_;
    }

    /* Returns a serialized representation of this reply message */
    const char* serialize() {
        StrBuff buff;
        buff.c("{type: reply, value: ");
        const char* serialized_v = v_->serialize();
        buff.c(serialized_v);
        buff.c("}");
        String* serial_str = buff.get();
        // Copying the char* so we can delete the String* returned from get()
        char* copy = new char[serial_str->size() + 1];
        strcpy(copy, serial_str->c_str());

        delete[] serialized_v;
        delete serial_str;
        return copy;
    }

    /* Return this reply's v_ field */
    DataFrame* get_value() {
        return v_;
    }

    /* Checks if this reply equals to the given object */
    bool equals(Object* o) {
        Reply* other = dynamic_cast<Reply*>(o);
        if (other == nullptr) return false;
        return other->get_value()->equals(get_value());
    }
};