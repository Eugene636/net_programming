#include <algorithm>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <socket_wrapper/socket_class.h>
#include <socket_wrapper/socket_headers.h>
#include <socket_wrapper/socket_wrapper.h>
struct ClientName
{
    int            client_number;
    std::string    client_ip_address;
    unsigned short client_port;
};
const ClientName* registration_client(std::string    client_ip_address,
                                      unsigned short client_port,
                                      std::vector<ClientName*>& clients)
{
    if (clients.empty())
    {
        ClientName* new_client        = new ClientName;
        new_client->client_number     = 1;
        new_client->client_ip_address = client_ip_address;
        new_client->client_port       = client_port;
        clients.push_back(new_client);
        return new_client;
    }
    for (int i = 0; i < clients.size(); i++)
    {
        if ((clients[i]->client_ip_address == client_ip_address) &&
            (clients[i]->client_port == client_port))
        {
            return clients[i];
        }
    }
    ClientName* new_client        = new ClientName;
    new_client->client_number     = clients.size() + 1;
    new_client->client_ip_address = client_ip_address;
    new_client->client_port       = client_port;
    clients.push_back(new_client);
    return new_client;
}

// Trim from end (in place).
static inline std::string& rtrim(std::string& s)
{
    s.erase(std::find_if(
                s.rbegin(), s.rend(), [](int c) { return !std::isspace(c); })
                .base());
    return s;
}

int main(int argc, char const* argv[])
{

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return EXIT_FAILURE;
    }

    socket_wrapper::SocketWrapper sock_wrap;
    const int                     port{ std::stoi(argv[1]) };

    socket_wrapper::Socket sock = { AF_INET, SOCK_DGRAM, IPPROTO_UDP };

    std::cout << "Starting echo server on the port " << port << "...\n";

    if (!sock)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in addr = {
        .sin_family = PF_INET,
        .sin_port   = htons(port),
    };

    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        std::cerr << sock_wrap.get_last_error_string() << std::endl;
        // Socket will be closed in the Socket destructor.
        return EXIT_FAILURE;
    }

    char buffer[256];

    // socket address used to store client address
    struct sockaddr_in client_address     = { 0 };
    socklen_t          client_address_len = sizeof(sockaddr_in);
    ssize_t            recv_len           = 0;

    std::cout << "Running echo server...\n" << std::endl;
    char                     client_address_buf[INET_ADDRSTRLEN];
    std::vector<ClientName*> client_names;

    while (true)
    {
        // Read content into buffer from an incoming client.
        recv_len = recvfrom(sock,
                            buffer,
                            sizeof(buffer) - 1,
                            0,
                            reinterpret_cast<sockaddr*>(&client_address),
                            &client_address_len);

        const ClientName* client =
            registration_client(inet_ntop(AF_INET,
                                          &client_address.sin_addr,
                                          client_address_buf,
                                          sizeof(client_address_buf) /
                                              sizeof(client_address_buf[0])),
                                ntohs(client_address.sin_port),
                                client_names);
        if (recv_len > 0)
        {
            buffer[recv_len - 1] = '\0';
            std::cout << "Client " << client->client_number << " with address "
                      << client->client_ip_address << ":" << client->client_port
                      << " sent datagram "
                      << "[length = " << recv_len << "]:\n'''\n"
                      << buffer << "\n'''" << std::endl;
            // if ("exit" == command_string) run = false;
            // send(sock, &buf, readden, 0);

            /*            std::string command_string = {buffer, 0, len};
                        rtrim(command_string);
                        std::cout << command_string << std::endl;
            */
            // Send same content back to the client ("echo").
            /*bool exit_flag = false;
            if (buffer[0] == 'e') {
                if (buffer[1] == 'x') {
                    if (buffer[2] == 'i') {
                        if(buffer[3] == 't')
                            if (buffer[4] == '\0') exit_flag = true;
                    }
                }
            }
            if (exit_flag == true) break;*/
            std::string exit_string = buffer;
            if (exit_string == "exit")
                break;
            sendto(sock,
                   buffer,
                   recv_len,
                   0,
                   reinterpret_cast<const sockaddr*>(&client_address),
                   client_address_len);
        }
        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}

/*struct sockaddr_in
{
    short int          sin_family;  // Семейство адресов, AF_INET.
    unsigned short int sin_port;    // Номер порта в сетевом порядке байт.
    struct in_addr     sin_addr;    // Internet address.
    unsigned char      sin_zero[8]; // Заполнение для соответствия размеру
struct sockaddr
};*/
