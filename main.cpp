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

// Структуры для бинарного обмена
struct BinaryRequest {
    char surname[64];
    double height;
    double weight;
};

struct BinaryResponse {
    char surname[64];
    char result[84];
    int status;  // 0 - норма, 1 - выше, 2 - ниже
};

// Функция для приема сообщений от сервера
void receiveMessages(SOCKET serverSocket) {
    BinaryResponse response;

    try {
        while (true) {
            memset(&response, 0, sizeof(response));
            int bytesReceived = recv(serverSocket, (char*)&response, sizeof(response), 0);

            if (bytesReceived == SOCKET_ERROR) {
                int error = WSAGetLastError();
                if (error == WSAETIMEDOUT) {
                    cout << "Таймаут ожидания ответа от сервера" << endl;
                    closesocket(serverSocket);
                    WSACleanup();
                    exit(-1);
                }
                else {
                    cout << "Ошибка приема данных: " << error << endl;
                }
                break;
            }

            if (bytesReceived == 0) {
                cout << "Соединение с сервером потеряно" << endl;
                break;
            }

            if (bytesReceived != sizeof(response)) {
                cout << "Получен ответ некорректного размера" << endl;
                continue;
            }

            response.surname[49] = '\0';
            response.result[19] = '\0';

            cout << "Ответ от сервера: " << response.surname
                << ": " << response.result;

            cout << endl;
        }
    }
    catch (...) {
        cout << "Ошибка в приеме сообщений" << endl;
    }
}

int main() {
    try {
        SetConsoleOutputCP(1251);
        SetConsoleCP(1251);
        setlocale(LC_ALL, "Russian");

        cout << "Клиент работает в бинарном режиме" << endl;

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

        // Устанавливаем таймауты
        int timeout = 30000; // 30 секунд
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

        // Настройка адреса сервера
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8888);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

        // Подключение к серверу
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cout << "Ошибка подключения к серверу" << endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        cout << "Подключено к серверу!" << endl;
        cout << "Введите фамилию, рост и вес через пробел" << endl;
        cout << "Для выхода введите 'exit'" << endl;

        // Запускаем поток для приема сообщений
        thread receiveThread(receiveMessages, clientSocket);
        receiveThread.detach();

        // Основной цикл отправки сообщений
        string surname;
        double height, weight;
        BinaryRequest request;

        while (true) {
            //cout << "> ";
            cin >> surname;

            if (surname == "exit" || surname == "\\q") {
                break;
            }

            cin >> height >> weight;

            if (!cin.fail()) {
                memset(&request, 0, sizeof(request));
                strncpy_s(request.surname, surname.c_str(), 49);
                request.height = height;
                request.weight = weight;

                int sent = send(clientSocket, (char*)&request, sizeof(request), 0);
                if (sent == SOCKET_ERROR) {
                    cout << "Ошибка отправки данных" << endl;
                    break;
                }
            }
            else {
                cout << "Ошибка ввода! Введите: фамилия рост вес" << endl;
                cin.clear();
                cin.ignore(10000, '\n');
            }
        }

        closesocket(clientSocket);
        WSACleanup();
    }
    catch (...) {
        cout << "Критическая ошибка клиента!" << endl;
    }

    return 0;
}
