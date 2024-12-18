#include <client.h>

Client::Client(const uint16_t port)
    : port_(port) {
    // Файловый дескриптор сокета
    sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    // AF_INET указывает семейство протоколов IPv4
    // SOCK_STREAM опрделяет тип сокета TCP
    // 0 - протокол по умолчанию

    if (sock_fd_ == -1) {
        std::cerr << Time() << "[ERROR] Socket()" << std::endl;
    }
    // Определяем адрес сервера
    addr_.sin_family = AF_INET; // AF_INET указывает семейство протоколов IPv4
    addr_.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr_.sin_port = htons(5000); // htons(): эта функция используется для преобразования беззнакового целого числа
    // из машинного порядка байтов в сетевой порядок байтов.
    // 5000 - порт, который клиент будет искать на сервере, чтобы подключиться

}

void Client::Run() const {
    // Соеденение с сервером
    if (const int ret = connect(sock_fd_, reinterpret_cast<const struct sockaddr*>(&addr_),sizeof(addr_)); ret == -1) {
        std::cerr << Time() << "[ERROR] Connection ERROR to " << inet_ntoa(addr_.sin_addr) << std::endl;
    } else {
        std::cout << Time() << "[CLIENT] Connection to " << inet_ntoa(addr_.sin_addr) << ":" << ntohs(addr_.sin_port)
        << " established!" << std::endl;
        Conection connect(sock_fd_);
    }
}

