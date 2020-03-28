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

        /** Type converters: Return same column under its actual type, or
         *  nullptr if of the wrong type.  */
        virtual Ack* as_ack() = 0;
        virtual Register* as_register() = 0;
        virtual Directory* as_directory() = 0;
        virtual Reply* as_reply() = 0;
        virtual Put* as_put() = 0;
        virtual Get* as_get() = 0;
        virtual WaitAndGet* as_wait_and_get() = 0;
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

        /* Returns this Ack */
        Ack* as_ack() {
            return this;
        }

        /* Returns nullptr because this is not a Register */
        Register* as_register() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Directory */
        Directory* as_directory() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Reply */
        Reply* as_reply() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Put */
        Put* as_put() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Get */
        Get* as_get() {
            return nullptr;
        }

        /* Returns nullptr because this is not a WaitAndGet */
        WaitAndGet* as_wait_and_get() {
            return nullptr;
        }
};

/**
 * This Message is sent when clients want to register with the server and when clients initially
 * connect with each other.
 */
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
            StrBuff buff;
            const char* serial_ip = ip_->serialize();

            buff.c("{type: register, ip: ");
            buff.c(serial_ip);
            buff.c(", sender: ");
            buff.c(sender_);
            buff.c("}");

            String* serial_str = buff.get();
            // Copying the char* so we can delete the String* returned from get()
            char* copy = new char[serial_str->size() + 1];
            strcpy(copy, serial_str->c_str());
            
            delete serial_str;
            delete[] serial_ip;
            return copy;
        }

        /* Returns nullptr because this is not an Ack */
        Ack* as_ack() {
            return nullptr;
        }

        /* Returns this Register */
        Register* as_register() {
            return this;
        }

        /* Returns nullptr because this is not a Directory */
        Directory* as_directory() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Reply */
        Reply* as_reply() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Put */
        Put* as_put() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Get */
        Get* as_get() {
            return nullptr;
        }

        /* Returns nullptr because this is not a WaitAndGet */
        WaitAndGet* as_wait_and_get() {
            return nullptr;
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
            StrBuff buff; // Use this buffer to build serial representation
            const char* serial_vec = addresses_->serialize();
            const char* serial_ivec = indices_->serialize();

            buff.c("{type: directory, addresses: ");
            buff.c(serial_vec);
            buff.c(", indices: ");
            buff.c(serial_ivec);
            buff.c("}");

            String* serial_str = buff.get();
            // Copying the char* so we can delete the String* returned from get()
            char* copy = new char[serial_str->size() + 1];
            strcpy(copy, serial_str->c_str());
            
            delete serial_str;
            delete[] serial_vec;
            delete[] serial_ivec;
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

        /* Returns nullptr because this is not an Ack */
        Ack* as_ack() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Register */
        Register* as_register() {
            return nullptr;
        }

        /* Returns this Directory */
        Directory* as_directory() {
            return this;
        }

        /* Returns nullptr because this is not a Reply */
        Reply* as_reply() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Put */
        Put* as_put() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Get */
        Get* as_get() {
            return nullptr;
        }

        /* Returns nullptr because this is not a WaitAndGet */
        WaitAndGet* as_wait_and_get() {
            return nullptr;
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

        /* Returns nullptr because this is not an Ack */
        Ack* as_ack() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Register */
        Register* as_register() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Directory */
        Directory* as_directory() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Reply */
        Reply* as_reply() {
            return nullptr;
        }

        /* Returns this Put */
        Put* as_put() {
            return this;
        }

        /* Returns nullptr because this is not a Get */
        Get* as_get() {
            return nullptr;
        }

        /* Returns nullptr because this is not a WaitAndGet */
        WaitAndGet* as_wait_and_get() {
            return nullptr;
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

        /* Returns nullptr because this is not an Ack */
        Ack* as_ack() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Register */
        Register* as_register() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Directory */
        Directory* as_directory() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Reply */
        Reply* as_reply() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Put */
        Put* as_put() {
            return nullptr;
        }

        /* Returns this Get */
        Get* as_get() {
            return this;
        }

        /* Returns nullptr because this is not a WaitAndGet */
        WaitAndGet* as_wait_and_get() {
            return nullptr;
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

        /* Returns nullptr because this is not an Ack */
        Ack* as_ack() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Register */
        Register* as_register() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Directory */
        Directory* as_directory() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Reply */
        Reply* as_reply() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Put */
        Put* as_put() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Get */
        Get* as_get() {
            return nullptr;
        }

        /* Returns this WaitAndGet */
        WaitAndGet* as_wait_and_get() {
            return this;
        }
};

/**
 * Reply is a subclass of Message that contains a DataFrame value;
 */
class Reply : public Message {
public:
    DataFrame* v_;

    /* Constructor, DOES NOT take ownership over v */
    Reply(DataFrame* v) {
        kind_ = MsgKind::Reply;
        v_ = v;
    } 

    /* Destructor */
    ~Reply() {
        delete v_;
    }

    /* Return this reply's v_ field */
    DataFrame* get_value() {
        return v_;
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

    /* Checks if this reply equals to the given object */
    bool equals(Object* o) {
        Reply* other = dynamic_cast<Reply*>(o);
        if (other == nullptr) return false;
        return other->get_value()->equals(get_value());
    }

        /* Returns nullptr because this is not an Ack */
        Ack* as_ack() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Register */
        Register* as_register() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Directory */
        Directory* as_directory() {
            return nullptr;
        }

        /* Returns this Reply */
        Reply* as_reply() {
            return this;
        }

        /* Returns nullptr because this is not a Put */
        Put* as_put() {
            return nullptr;
        }

        /* Returns nullptr because this is not a Get */
        Get* as_get() {
            return nullptr;
        }

        /* Returns nullptr because this is not a WaitAndGet */
        WaitAndGet* as_wait_and_get() {
            return nullptr;
        }
};