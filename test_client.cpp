#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>
#include <cstring>
#include <ctime>
#include <iostream>
#include <atomic>
#include <mutex>
using namespace std;

int main(int argc, char **argv) {
    std::vector<std::thread> threads;

    if (argc < 4) exit(EXIT_FAILURE);

    unsigned long total_thread = strtoul(argv[3], nullptr, 10);
    struct sockaddr_in remote_addr;
    
    std::mutex avg_mutex;
    std::atomic<double> total_avg(0.0);
    for (int i = 0; i < total_thread; ++ i) {
        auto t = [&] () {
            struct sockaddr_in remote_addr;
            memset(&remote_addr, 0, sizeof(remote_addr));
            remote_addr.sin_family = AF_INET;
            remote_addr.sin_addr.s_addr = inet_addr(argv[1]);
            remote_addr.sin_port = htons(strtoul(argv[2], nullptr, 10));

            int cfd;
            if ((cfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket");
                return;
            }

            if (connect(cfd, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0) {
                perror("connect");
                return;
            }

            char sendbuf[] = "GET / HTTP/1.1\r\n"
                "Content-Length: 0\r\n"
                "Host: 127.0.0.1\r\n"
                "\r\n";
            char recvbuf[102400];

            const int REQ_NUM = 100;
            time_t accu_time = 0;;
            for (int i = 0; i < REQ_NUM; ++ i) {
                int len;

                time_t cur_time = time(NULL);
                if ((len = send(cfd, sendbuf, sizeof(sendbuf) - 1, 0)) <= 0) {
                    perror("send");
                    return;
                }

                if ((len = recv(cfd, recvbuf, sizeof(recvbuf), 0)) <= 0) {
                    perror("recv");
                    return;
                }
                accu_time += time(NULL) - cur_time;
            }

            std::lock_guard<std::mutex> lck(avg_mutex);
            total_avg = total_avg + (double) accu_time / REQ_NUM;

            shutdown(cfd, SHUT_RDWR);
        };

        threads.push_back(thread(t));
    }

    for (auto& t: threads) {
        t.join();
    }

    std::cout << total_avg / total_thread << "s per requests." << std::endl;

    return 0;
}
