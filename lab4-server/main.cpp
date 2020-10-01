#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <fcntl.h>

#define FSIZE (int)fSize[0]

using namespace std;

typedef int SOCKET;

typedef struct Book
{
    char name[100];
    char author[100];
    char publisher[100];
    int yearOfPublishing = 0;
    void operator=(const Book& b2)
    {
        strcpy(name, b2.name);
        strcpy(author, b2.author);
        strcpy(publisher, b2.publisher);
        yearOfPublishing = b2.yearOfPublishing;
    }
} *pBook;



void childFunc(SOCKET s);

static Book* sharedMem;

int main()
{
    int fd = open("/home/xleby/data.bin", O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    lseek(fd, 50 * sizeof(Book), SEEK_SET);
    write(fd, "\0", 2);

    sharedMem = (Book*)mmap(NULL, 50 * sizeof(Book), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in local;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(1280);
    local.sin_family = AF_INET;

    int c = bind(listenSocket, (sockaddr*)&local, sizeof(local));

    int r = listen(listenSocket, 5);

    while (true)
    {
        sockaddr_in remoteAddr;
        int remoteAddrSize = sizeof(remoteAddr);
        SOCKET s = accept(listenSocket, (sockaddr*)&remoteAddr, (socklen_t*)&remoteAddrSize);
        pid_t childPid = fork();
        if (childPid == 0)
        {
            cout << getpid() << '\n';
            childFunc(s);
            _exit(EXIT_SUCCESS);
        }
    }
    shutdown(listenSocket, SHUT_RDWR);
    munmap(sharedMem, 50 * sizeof(Book));
    return EXIT_SUCCESS;
}

void childFunc(SOCKET s)
{
    pBook pb = new Book{ "", "", ""}, p = new Book{ "", "", ""};
    char* fSize = (char*)(sharedMem + 50);
    char* buf = (char*)pb;
    int bufSize = sizeof(Book);
    char ch = 66, str[100], cmp;
    int recCount, nRec;
    cout << FSIZE << '\n';
    while (ch != 0)
    {
        recv(s, &ch, 1, 0);
        cout << "Got ch = " << ch + 48 << '\n';
        switch (ch)
        {
        case 1:
            recv(s, buf, bufSize, 0);
            p = sharedMem + FSIZE;
            *p = *pb;
            ++fSize[0];
            cout << FSIZE << '\n';
            break;
        case 2:
            ch = fSize[0];
            send(s, &ch, 1, 0);
            p = sharedMem;
            for (int i = 0; i < FSIZE; i++, ++p)
            {
                //cout << p->yearOfPublishing << '\n';
                //pb = sharedMem + i;
                send(s, (char*)p, bufSize, 0);
            }
            break;
        case 3:
            recv(s, str, 100, 0);
            p = sharedMem;
            for (int i = 0; i < FSIZE; i++, ++p)
            {
                if ((strlen(p->name) != strlen(str))||(strncmp(p->name, str, strlen(str)) != 0))
                {
                    ch = 0;
                }
                else
                {
                    ch = 1;
                    send(s, &ch, 1, 0);
                    recv(s, buf, bufSize, 0);

                    *p = *pb;
                    i = FSIZE;
                }
            }
            if (ch == 0) send(s, &ch, 1, 0);
            ch = 3;
            break;
        case 4:
            recv(s, str, 100, 0);
            p = sharedMem;
            for (int i = 0; i < FSIZE; i++, ++p)
            {
                if ((strlen(p->name) != strlen(str))||(strncmp(p->name, str, strlen(str)) != 0))
                {
                    ch = 0;
                }
                else
                {
                    ch = 1;
                    send(s, &ch, 1, 0);
                    strcpy(p->name, "");
                    i = FSIZE;
                }
            }
            if (ch == 0) send(s, &ch, 1, 0);
            ch = 4;
            break;
        case 5:
            recv(s, str, 100, 0);
            nRec = 0;
            p = sharedMem;
            for (int i = 0; i < FSIZE; i++, ++p)
            {
                if ((strlen(p->author) == strlen(str))&&(strncmp(p->author, str, strlen(str)) == 0))
                        ++nRec;
            }
            ch = nRec;
            send(s, &ch, 1, 0);
            p = sharedMem;
            for (int i = 0; i < FSIZE; i++, ++p)
            {
                if ((strlen(p->author) == strlen(str))&&(strncmp(p->author, str, strlen(str)) == 0))
                    send(s, (char*)p, bufSize, 0);
            }
            ch = 5;
            break;
        /*case 6:

            break;*/
        case 0:
            return;
            break;
        default:
            break;
        }
    }
}
