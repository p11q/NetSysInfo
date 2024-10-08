#pragma once
#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>    // Для использования Socket()
#include <netinet/in.h>    // Для использования sockaddr_in
#include <ctime>           // Для определения даты и времени
#include <fstream>          // Чтобы работать с файлами
#include <nlohmann/json.hpp> // Для использования JSON
#include "file_handler.h" // Для использования БД

// Для использования даты и времени
inline std::string m_time(const time_t now = time(nullptr)) {
    const tm *ltm {localtime(&now)};
    return "[" + std::to_string(ltm->tm_mday) + ":" + std::to_string(ltm->tm_mon) + ":" + std::to_string(1900 + ltm->tm_year) +
        " " + std::to_string(ltm->tm_hour) + ":" + std::to_string(ltm->tm_min) + ":" + std::to_string(ltm->tm_sec) + "] ";
}

struct Person {
    int m_cli_id; // Присваевается посе записи в БД
    std::string m_cli_name{}; // Передается от клиента
    std::string m_cli_serial{}; // Передается от клиента
};

inline void Writing_to_the_database(const nlohmann::json &content) // Запись базу данных
{
    FileDB f_db(content["serial"]);
    f_db.write(content);
    std:: cout << m_time() << f_db.check() << std::endl;
}

inline int start_server() {
    // Файловый дескриптор сокета
    const int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    // AF_INET указывает семейство протоколов IPv4
    // SOCK_STREAM опрделяет тип сокета TCP
    // 0 - протокол по умолчанию

    if (sock_fd == -1) {
        std::cerr << m_time() << "[ERROR] Socket()" << std::endl;
        return 1;
    }

    // Чтобы сервер не привязывался к адресу (который может быть в состоянии ожидания, SO_REUSEADDR, сразу после отключения
    //отвязывает сервер от адреса и следовательно его можно повторно запустить по тому же адресу)
    constexpr int optval = 1;
    int ret = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (ret == -1) {
        std::cerr << m_time() << "[ERROR] Setsockopt error" << std::endl;
        return 1;
    }

    // Определяем адрес клиента
    struct sockaddr_in addr{}; // sockaddr_in - это тип данных, который используется для хранения адреса сокета
    addr.sin_family = AF_INET;     // AF_INET указывает семейство протоколов IPv4
    addr.sin_port = htons(5000); // htons(): эта функция используется для преобразования беззнакового целого числа
    // из машинного порядка байтов в сетевой порядок байтов.
    // 5000 - порт, который будет слушать наш сервер
    addr.sin_addr.s_addr = htonl(INADDR_ANY);    //INADDR_ANY: используется, когда мы не хотим привязывать наш сокет
    // к какому-либо конкретному IP-адресу и вместо этого заставляем его
    // прослушивать все доступные IP-адреса.

    // Чтобы привязать файловой дескриптор сокета к выброному домену
    ret = bind(sock_fd, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr));
    // reinterpret_cast - для преобразования указателя на sockaddr_in в указатель sockaddr
    if (ret == -1) {
        std::cerr << m_time() << "[ERROR] Bind error" << std::endl;
        return 1;
    }

    // Для прослушивания сокета
    ret = listen(sock_fd, 5);
    // sock_fd - какой сокет слушать
    // 5 - сколько запросов на подключение может быть помещены в очередь, прежде чем будут отклонены дальнейшие запросы
    if (ret == -1) {
        std::cerr << m_time() << "[ERROR] Listen error" << std::endl;
        return 1;
    }

    // Работа с клиентом
    int client_socket = 0;
    socklen_t addr_len = sizeof addr;
    while ((client_socket = accept(sock_fd, reinterpret_cast<struct sockaddr *>(&addr), &addr_len))) {
        // Accept() используется для принятия запроса на соединение,
        // полученного в сокете, который прослушивало приложение.
        if (client_socket == -1) {
            std::cerr << m_time() << "[ERROR] Accept error" << std::endl;
            return 1;
        }

        // Если подключится клиент, выводим уведомление
        std::cout << m_time() << "[SERVER] Accepted new connection from client with an " << inet_ntoa(addr.sin_addr)
                  << ":" << ntohs(addr.sin_port) << std::endl;

        // Для отправки данных клиенту
        std::string msg{"Hello! I,m Server"};
        if (const size_t transmitted = send(client_socket, msg.data(), msg.size(), 0); transmitted != msg.size()) {
            std::cerr << m_time() << "[ERROR] not all data transmitted" << std::endl;
            return 1;
        }

        // Для получения данных от клиента
        unsigned char buffer[512] = {'\0'};
        if (const ssize_t received = recv(client_socket, buffer, 512, 0); received <= 0) {
            std::cerr << m_time() << "[ERROR] Error recv()" << std::endl;
        }
        // Парс JSON
        nlohmann::json _json = nlohmann::json::parse(buffer);

        Writing_to_the_database(_json);

    }
    return 0;
}
