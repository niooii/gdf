#pragma once

#include <constants.h>
#include <ser20/ser20.hpp>
#include <ser20/types/polymorphic.hpp>
#include <serde.h>
#include <vector>
#include <game/types.h>
#include <services/fwd.h>

/*
 * This file contains all the network specific components of the game,
 * including specific events (packets) meant for use over the net only
 */

namespace Net {
    struct Packet {
        Packet() {
            source = CURR_PROGRAM_TYPE;
            creation_time = Services::Time::unix_millis();
        }
        virtual ~Packet() = default;

        /// TODO! maybe collapse this and other fields into a bitfield or something.
        /// this is true if and only if the packet is a ClientConnectionEvent type, although
        /// it is not enforced anywhere.
        bool connect_event = false;

        ProgramType source = ProgramType::Client;
        /// The unix timestamp of when the event was created from the source,
        /// not deserialized. In UTC.
        u64 creation_time;

        /// Additional context for the event.
        /// In server code, this contains a ConnectedClient pointer, unless the event is
        /// an instance of ClientDisconnectEvent.
        /// In client code, this field is unused for now, do not read or write to it.
        void* data;

        /// Dispatches the packet's true type as an "event" to local handlers.
        virtual void dispatch() const = 0;

        /// Queues dispatching the packet's true type as an "event" to local handlers.
        virtual void queue_dispatch() const = 0;

        template<class Archive>
        void serialize(Archive& ar) {
            ar(source, creation_time, connect_event);
        }
    };

    // Event template type
    template<typename Derived>
    struct PacketT : Packet {
        void dispatch() const override {
            Services::Events::dispatch(static_cast<const Derived&>(*this));
        }

        void queue_dispatch() const override {
            Services::Events::queue_dispatch(static_cast<const Derived&>(*this));
        }
    };

    FORCEINLINE std::string serialize(const std::unique_ptr<Packet>& packet) {
        std::ostringstream os;

        ser20::BinaryOutputArchive archive{os};
        archive(packet);

        return os.str();
    }

    FORCEINLINE std::unique_ptr<Packet> deserialize(const std::span<char>& data) {
        std::unique_ptr<Packet> packet;

        std::ispanstream stream{data};

        ser20::BinaryInputArchive archive{stream};
        archive(packet);

        return packet;
    }
}

SER20_REGISTER_TYPE(Net::Packet);

/* Macros for declaring event types */

// Helper macro for serializing fields on a packet type
#define SERIALIZE_PACKET_FIELDS(...) \
template<class Archive> \
void serialize(Archive& ar) { \
ar(ser20::base_class<Net::Packet>(this) __VA_OPT__(,) __VA_ARGS__); \
}

// Every serializable event must contain SERIALIZE_PACKET_FIELDS somewhere in it's declaration.
#define DECL_PACKET(name) struct name; \
SER20_REGISTER_TYPE(name) \
SER20_REGISTER_POLYMORPHIC_RELATION(Net::Packet, name) \
struct name : Net::PacketT<name>

/// To send a message to the server for testing purposes.
DECL_PACKET(TestMsgEvent)
{
    std::string message;
    SERIALIZE_PACKET_FIELDS(message)
};

/// Contains everything the server needs to know about a client upon connection.
DECL_PACKET(ClientConnectionEvent)
{
    /// TODO! This is unused for now. For development purposes, the only field that matters is
    /// the UUID. Change before production.
    std::string auth;
    /// The UUID of the player. Do not rely on this field, it will be removed once proper authentication
    /// is implemented.
    std::string uuid;
    SERIALIZE_PACKET_FIELDS(auth, uuid)
};

DECL_PACKET(ClientDisconnectEvent)
{
    /// TODO! This is unused for now. For development purposes, the only field that matters is
    /// the UUID. Change before production.
    std::string auth;
    /// The UUID of the player who has disconnected
    std::string uuid;
};

