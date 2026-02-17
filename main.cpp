#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WIN32_WINNT 0x0600

#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <thread>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Функция для приема сообщений от сервера
void receiveMessages(SOCKET serverSocket) {
    char buffer[1024];

    try {
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(serverSocket, buffer, sizeof(buffer), 0);

            if (bytesReceived <= 0) {
                cout << "Соединение с сервером потеряно" << endl;
                break;
            }

            cout << "Ответ от сервера: " << buffer;
        }
    }
    catch (...) {
        std::cout << "Something went wrong!";
    }
}

int main() {
    try {
        SetConsoleOutputCP(1251);
        SetConsoleCP(1251);
        setlocale(LC_ALL, "Russian");

        // Инициализация Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cout << "Ошибка инициализации Winsock" << endl;
            return 1;
        }

        // Создание сокета
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            cout << "Ошибка создания сокета" << endl;
            WSACleanup();
            return 1;
        }

        // Настройка адреса сервера
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8888);

        // IP сервера (для локальной работы - 127.0.0.1)
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        // Подключение к серверу
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cout << "Ошибка подключения к серверу" << endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        cout << "Подключено к серверу!" << endl;
        cout << "Введите данные в формате: Фамилия рост вес" << endl;
        cout << "Для выхода введите 'exit'" << endl;

        // Запускаем поток для приема сообщений
        thread receiveThread(receiveMessages, clientSocket);
        receiveThread.detach();

        // Основной цикл отправки сообщений
        string input;
        while (true) {
            getline(cin, input);

            if (input == "exit" || input == "\\q") {
                break;
            }

            if (!input.empty()) {
                // Отправляем данные на сервер
                send(clientSocket, input.c_str(), input.length(), 0);
            }
        }

        closesocket(clientSocket);
        WSACleanup();
    }
    catch (...) {
        std::cout << "Something went wrong!";
    }

    return 0;
}
