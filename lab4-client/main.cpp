#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

typedef int SOCKET;

typedef struct Book
{
    char name[100];
    char author[100];
    char publisher[100];
    int yearOfPublishing = 0;
} *pBook;

istream& operator>>(istream& in, Book& s)
{
    cout << "Введите название книги:\n";
    in.getline(s.name, 100, '\n');
    cout << "Введите автора:\n";
    in.getline(s.author, 100, '\n');
    cout << "Введите издательство:\n";
    in.getline(s.publisher, 100, '\n');
    cout << "Введите год издания:\n";
    in >> s.yearOfPublishing;
    return in;
}

ostream& operator<<(ostream& out, Book& s)
{
    out << "Книга: " << s.name << '\n'
        << "Автор: " << s.author << '\n'
        << "Издатель: " << s.publisher << '\n'
        << "Год издания: " << s.yearOfPublishing << "\n\n";
    return out;
}

int main()
{
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in ad;
    ad.sin_addr.s_addr = inet_addr("127.0.0.1");
    ad.sin_port = htons(1280);
    ad.sin_family = AF_INET;

    int connectFailed = connect(s, (sockaddr*)&ad, sizeof(ad));
    if (!connectFailed)
    {
        pBook pb = new Book{ "", "", ""};
        char* buf = (char*)pb;
        int bufSize = sizeof(Book);
        int choice = 666;
        while (choice)
        {
            cout << "1. Добавить книгу\n"
                << "2. Вывести список книг на экран\n"
                << "3. Редактировать информацию о книге\n"
                << "4. Удалить книгу\n"
                << "5. Поиск книги по фамилии автора\n"
                << "6. Сортировка книг по году издания (не работает)\n"
                << "0. Disconnect\n";
            scanf("%d%*c", &choice);
            char ch;
            char str[100];
            ch = choice;
            send(s, &ch, 1, 0);
            system("CLS");
            switch (choice)
            {
            case 1:
                cin >> *pb;
                send(s, buf, bufSize, 0);
                break;
            case 2:
                recv(s, &ch, 1, 0);
                cout << "Список книг:\n";
                for (int i = 0; i < (int)ch; i++)
                {
                    recv(s, buf, bufSize, 0);
                    if (pb->name[0] != '\0')
                        cout << *pb;
                }
                break;
            case 3:
                cout << "Введите название книги:\n";
                cin.getline(str, 100);
                send(s, str, 100, 0);
                recv(s, &ch, 1, 0);
                if (ch == 0)
                    cout << "Книга не найдена.\n";
                else
                {
                    cin >> *pb;
                    send(s, buf, bufSize, 0);
                    cout << "Информация обновлена.\n";
                }
                break;
            case 4:
                cout << "Введите название книги:\n";
                cin.getline(str, 100);
                send(s, str, 100, 0);
                recv(s, &ch, 1, 0);
                if (ch == 0)
                    cout << "Книга не найдена.\n";
                else
                {
                    cout << "Информация удалена.\n";
                }
                break;
            case 5:
                cout << "Введите фамилию автора:\n";
                cin.getline(str, 100);
                send(s, str, 100, 0);
                recv(s, &ch, 1, 0);
                cout << "Список книг:\n";
                for (int i = 0; i < ch; i++)
                {
                    recv(s, buf, bufSize, 0);
                    cout << *pb;
                }
                break;
            /*case 6:

                break; */
            case 0:
                break;
            default:
                cout << "Wrong choice! Try again\n";
                system("CLS");
                break;
            }
        }
    }
    shutdown(s, SHUT_RDWR);
    return EXIT_SUCCESS;
}
