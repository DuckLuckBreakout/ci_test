#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <thread>

#include <boost/bind.hpp>




class Client : public std::enable_shared_from_this<Client> {
public:
    Client(boost::asio::io_service& service);

    boost::asio::ip::tcp::socket& getSocket();
    char* getReadBuff();
    char* getSendBuff();

    void send();
    void handleSend(const boost::system::error_code& error,
                    std::size_t bytes);

    void read();
    void handleRead(const boost::system::error_code& error,
                    std::size_t bytes);

private:
    boost::asio::ip::tcp::socket socket;
    char readBuff[1024];
    char sendBuff[1024];
};

Client::Client(boost::asio::io_service& service) : socket(service) {}

boost::asio::ip::tcp::socket& Client::getSocket() {
    return socket;
}

char* Client::getReadBuff() {
    return readBuff;
}

char* Client::getSendBuff() {
    return sendBuff;
}

void Client::read() {
    for (int i = 0; i < 1024; i++) {
        readBuff[i] = 0;
    }

    std::cerr << "input : " << std::endl;
    std::cerr << "thread is " << pthread_self() << std::endl;
    int n = socket.read_some(boost::asio::buffer(readBuff));
    std::cerr << "was read message: [ " << n << " ] ";
    for (int i = 0; i < 1024; i++)
        std::cerr << readBuff[i];
    std::cerr << "\n";
//                           boost::bind(&Client::handleRead, shared_from_this(),
//                                       boost::asio::placeholders::error,
//                                       boost::asio::placeholders::bytes_transferred));
}

void Client::send() {
    std::cerr << "send: " << std::endl;
    std::cerr << "thread is " << pthread_self() << std::endl;
    int n = socket.write_some(boost::asio::buffer(sendBuff));
    std::cerr << "was send message: [ " << n << " ] ";
    for (int i = 0; i < 1024; i++)
        std::cerr << sendBuff[i];
    std::cerr << "\n";
//                           boost::bind(&Client::handleSend, shared_from_this(),
//                                       boost::asio::placeholders::error,
//                                       boost::asio::placeholders::bytes_transferred));
}

void Client::handleRead(const boost::system::error_code &error,
                        std::size_t bytes_transferred) {
    if (error == boost::asio::error::eof)
        std::cerr << "end read client: " << socket.remote_endpoint().address().to_string()
                  << std::endl;

    if (error)
        return;

    std::cerr << "read: [ " << bytes_transferred << " ] bytes" << std::endl;
    std::cerr << "thread is " << pthread_self() << std::endl;
}

void Client::handleSend(const boost::system::error_code &error,
                        std::size_t bytes_transferred) {
    if (error)
        return;

    std::cerr << "send: [ " << bytes_transferred << " ] bytes" << std::endl;
    std::cerr << "thread is " << pthread_self() << std::endl;


}



class CommandHendler {
public:
    void runRequest(std::shared_ptr<Client> client);
};

void CommandHendler::runRequest(std::shared_ptr<Client> client) {
    sleep(5);
    for (int i = 0; i < 1024; i++)
        client->getSendBuff()[i] = '\0';
    memccpy(client->getSendBuff(), client->getReadBuff(), 0, 1024);
}



class Server {
public:
    Server();

    void startServer(int port);

private:
    void onAccept(std::shared_ptr<Client> client, const boost::system::error_code& error);
    void startAccept();
    void run();
    void runTask(std::shared_ptr<Client> client);
    void restart(std::shared_ptr<Client> client);

private:
    boost::asio::io_service service;
    boost::asio::ip::tcp::acceptor acceptor;

};

Server::Server() : acceptor(service) {}

void Server::startServer(int port) {
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);

    acceptor.open(endpoint.protocol());
    acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen(1024);
    startAccept();

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i)
        threads.push_back(std::thread(std::bind(&Server::run, this)));

    for (auto& thread : threads)
        thread.join();
}

void Server::onAccept(std::shared_ptr<Client> client, const boost::system::error_code &error) {
    if (error)
        return;

    std::cerr << "Start read client: " << client->getSocket().remote_endpoint().address().to_string().c_str()
              << std::endl;

//    client->read();
//    std::shared_ptr<CommandHendler> commandHendler(new CommandHendler());
//    commandHendler->runRequest(client);
//    client->send();

    service.post(boost::bind(&Server::runTask, this, client));

    startAccept();
}


void Server::runTask(std::shared_ptr<Client> client) {

    boost::asio::io_service::strand strandOne(service);
    service.post(strandOne.wrap(boost::bind(&Client::read, client)));

    std::shared_ptr<CommandHendler> commandHendler(new CommandHendler());
    service.post(strandOne.wrap(boost::bind(&CommandHendler::runRequest, commandHendler,
                                            client)));

    service.post(strandOne.wrap(boost::bind(&Client::send, client)));
    service.post(strandOne.wrap(boost::bind(&Server::restart, this, client)));
}

void Server::startAccept() {
    std::shared_ptr<Client> client(new Client(service));
    acceptor.async_accept(client->getSocket(), boost::bind(&Server::onAccept, this, client,
                                                        boost::asio::placeholders::error));
}

void Server::run() {
    service.run();
}

void Server::restart(std::shared_ptr<Client> client) {
    runTask(client);
}




int main(int argc, char* argv[]) {

    Server server;
    server.startServer(5000);
    return 0;
}
