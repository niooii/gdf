#include <client/net.h>
#include <prelude.h>

static unsigned long io_thread(void* args)
{
    ServerConnection* conn = (ServerConnection*)args;
    GDF_InitThreadLogging("Client:Net");

    const GDF_Stopwatch throttle_timer = GDF_StopwatchCreate();
    // Make loop happen at a fixed rate of 2ms per iteration
    constexpr f64 throttle_secs = 0.002;

    LOG_INFO("Listening for server events..");

    ENetEvent event;
    do
    {
        GDF_StopwatchReset(throttle_timer);

        GDF_LockMutex(conn->outgoing_mutex);
        for (auto& outgoing : conn->outgoing_queue)
        {
            std::string serialized{ Net::serialize(outgoing) };

            ENetPacket* packet = enet_packet_create(
                serialized.c_str(), serialized.length() + 1, ENET_PACKET_FLAG_RELIABLE);

            enet_peer_send(conn->peer, 0, packet);
            // LOG_WARN("Packets sent!!!");
        }
        conn->outgoing_queue.clear();
        GDF_ReleaseMutex(conn->outgoing_mutex);

        while (enet_host_service(conn->client, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                {
                    try
                    {
                        auto recv_event = Net::deserialize(
                            { (char*)event.packet->data, event.packet->dataLength });

                        if (recv_event->source != ProgramType::Server)
                        {
                            LOG_WARN("Rejected packet for invalid packet source.");
                            goto CN_DESTROY_PACKET;
                        }

                        GDF_LockMutex(conn->incoming_mutex);
                        conn->incoming_queue.push_back(std::move(recv_event));
                        GDF_ReleaseMutex(conn->incoming_mutex);
                    }
                    catch (const ser20::Exception& e)
                    {
                        LOG_ERR("Failed to deserialize a packet from server");
                    }

                CN_DESTROY_PACKET:
                    enet_packet_destroy(event.packet);
                }
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                LOG_DEBUG("Server disconnected.\n");
                return 0;

            default:
                break;
            }
        }
        if (const f64 overflow = GDF_StopwatchSleepUntil(throttle_timer, throttle_secs);
            overflow > 0)
            LOG_WARN("Server fell behind by %lf secs :(", overflow);
    }
    while (conn->io_active);

    GDF_StopwatchDestroy(throttle_timer);

    return 0;
}

ServerConnection::ServerConnection(const char* addr, u16 port)
{
    io_active  = true;
    this->port = port;
    this->addr = addr;

    incoming_mutex = GDF_CreateMutex();
    outgoing_mutex = GDF_CreateMutex();

    constexpr u32 channels = 2;

    client = enet_host_create(NULL, 1, channels, 0, 0);
    if (client == NULL)
    {
        LOG_FATAL("An error occurred while creating the client")
    }

    ENetAddress address{};
    enet_address_set_host(&address, addr);
    address.port = port;

    this->peer = enet_host_connect(client, &address, channels, 0);

    if (this->peer == NULL)
    {
        LOG_ERR("No available peers for initiating an ENet connection");
        enet_host_destroy(client);
        throw std::runtime_error("Connection to server failed");
    }

    ENetEvent event;
    if (enet_host_service(client, &event, 250) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
    {
        LOG_INFO("Connection to server succeeded");
    }
    else
    {
        enet_peer_reset(peer);
        enet_host_destroy(client);
        throw std::runtime_error("Connection to server failed");
    }

    recv_thread = GDF_CreateThread(io_thread, this);
}

ServerConnection::~ServerConnection()
{
    io_active = false;

    GDF_JoinThread(recv_thread);
    GDF_DestroyThread(recv_thread);

    GDF_DestroyMutex(incoming_mutex);
    GDF_DestroyMutex(outgoing_mutex);

    enet_peer_reset(peer);
    enet_peer_disconnect_now(peer, 0);

    enet_host_destroy(client);
}

void ServerConnection::send(std::unique_ptr<Net::Packet> unique_ptr)
{
    unique_ptr->source = ProgramType::Client;
    GDF_LockMutex(outgoing_mutex);
    outgoing_queue.push_back(std::move(unique_ptr));
    GDF_ReleaseMutex(outgoing_mutex);
}

void ServerConnection::dispatch_incoming()
{
    GDF_LockMutex(incoming_mutex);
    for (auto& event : incoming_queue)
    {
        event->queue_dispatch();
    }

    incoming_queue.clear();
    GDF_ReleaseMutex(incoming_mutex);
}
