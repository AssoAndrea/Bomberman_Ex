#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include "client.h"
#include <time.h>
#include <SDL.h>
#include <bomberman.h>

#define SERVER_IP "192.168.1.200"
#define SERVER_PORT 900
#define KA_TIME 3 //keep alive packet timer

#define PACKED __attribute__((__packed__))

#define P_DISCONNECTED "DSCN"
#define P_KEEPALIVE "KEEP"
#define P_POSITION "PSTN"
#define P_COLOR "COLR"
#define P_BOMB "BOMB"
#define P_REQ_AUTH "RQAU"
#define P_SND_AUTH "SEAU"

typedef struct  pk_keep_alive
{
    char type[4];
    int auth;
} pk_keep_alive_t;

typedef struct pk_req_auth
{
    char type[4];
} pk_req_auth_t;

typedef struct pk_snd_auth
{
    char type[4];
    int auth;
} pk_snd_auth_t;

typedef struct pk_disconnected
{
    char type[4];
    int auth;
} pk_disconnected_t;

typedef struct pk_position
{
    char type[4];
    int auth;
    float x;
    float y;
} pk_position_t;

typedef struct pk_bomb
{
    char type[4];
    int auth;
    float x;
    float y;
} pk_bomb_t;

typedef struct pk_color
{
    char type[4];
    int auth;
    int r;
    int g;
    int b;
}pk_color_t;


int sk;
struct sockaddr_in sock_in;
unsigned int auth = -1;

int client_init()
{
    #ifdef _WIN32
    // this part is only required on Windows: it initializes the Winsock2 dll
    WSADATA wsa_data;
    if (WSAStartup(0x0202, &wsa_data))
    {
        printf("unable to initialize winsock2\n");
        return -1;
    }
    #endif
    sk = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sk<0)
    {
        perror("unable to initialize socket\n");
        return -1;
    }
    inet_pton(AF_INET, SERVER_IP, &sock_in.sin_addr);
    sock_in.sin_family = AF_INET;
    sock_in.sin_port = htons(SERVER_PORT);

    _set_timeout();
    return 0;
}

int _set_timeout()
{
    unsigned int timeout = 5;
    if (setsockopt(sk, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(unsigned int)))
    {
        printf("unable to set socket option for receive timeout");
        return -1;
    }

    int current_buffer_size = 0;
    int sockopt_len = sizeof(int);
    getsockopt(sk, SOL_SOCKET, SO_RCVBUF, (char*)&current_buffer_size, &sockopt_len);
    //printf("current buffer size = %d\n", current_buffer_size);

    int buffer_size = 8192;
    if (setsockopt(sk, SOL_SOCKET, SO_RCVBUF, (char *)&buffer_size, sizeof(int)))
    {
        printf("unable to set socket option for receive buffer size");
        return -1;
    }

    if (setsockopt(sk, SOL_SOCKET, SO_SNDBUF, (char *)&buffer_size, sizeof(int)))
    {
        printf("unable to set socket option for send buffer size");
        return -1;
    }

    getsockopt(sk, SOL_SOCKET, SO_RCVBUF, (char*)&current_buffer_size, &sockopt_len);
    //printf("current buffer size = %d\n", current_buffer_size);
    return 0;
}

void send_data(char *data,int len)
{
    int sent_bytes = sendto(sk, data, len, 0, (struct sockaddr *)&sock_in, sizeof(sock_in));
    char addr_as_string[64];
    inet_ntop(AF_INET, &sock_in.sin_addr, addr_as_string, 64);
    //printf("sends %d bytes to %s \n", sent_bytes,addr_as_string);

}

void send_auth_req()
{
    pk_req_auth_t packet = {P_REQ_AUTH};
    send_data(&packet, sizeof(pk_req_auth_t));
}



void send_position(float x, float y)
{
    pk_position_t packet = {P_POSITION, auth, x, y};
    send_data(&packet, sizeof(pk_position_t));
}

char client_is_authorized()
{
    if(auth >0)
        return 0;
    else
        return 1;
}

void keep_alive(clock_t *start)
{
    clock_t current_time = clock();
    float elapsed_time = (float)(current_time - *start) / CLOCKS_PER_SEC;
    if (elapsed_time > KA_TIME)
    {
        pk_keep_alive_t packet = {P_KEEPALIVE, auth};
        send_data(&packet, sizeof(pk_keep_alive_t));
        *start = clock();
    }
}

char *receive_data()
{
    char buffer[8192];
    struct sockaddr_in sender_in;
    int sender_in_size = sizeof(sender_in);
    int len = recvfrom(sk, buffer, 8191, 0, (struct sockaddr*)&sender_in, &sender_in_size);
    if (len > 0)
    {
        char addr_as_string[64];
        inet_ntop(AF_INET, &sender_in.sin_addr, addr_as_string, 64);
        printf("received %d bytes from %s:%d\n", len, addr_as_string, ntohs(sender_in.sin_port));

        char type[5];
        SDL_memcpy(type, buffer, 4);
        type[4] = '\0';
        printf("type = %s\n", type);

        if (SDL_strcmp(P_SND_AUTH,type)==0)
        {
            pk_snd_auth_t auth_packet;
            SDL_memcpy(&auth_packet, buffer, sizeof(pk_snd_auth_t));
            auth = auth_packet.auth;
            if (auth == 1)
            {
                local_player = player0;
                remote_player = player1;
            }else
            {
                local_player = player1;
                remote_player = player0;
            }
            printf("local pos is: %d", local_player.movable.rect.x);

            
        }
        else if(SDL_strcmp(P_POSITION,type)==0)
        {
            pk_position_t pos_packet;
            SDL_memcpy(&pos_packet, buffer, sizeof(pk_position_t));
            remote_player.movable.rect.x = pos_packet.x;
            remote_player.movable.rect.y = pos_packet.y;
        }
        
    }
    else if (len < 0)
    {
        int socket_error = WSAGetLastError();
        if (socket_error == WSAETIMEDOUT)
        {
            return NULL;
        }
        else
        {
            printf("ops: %d\n", socket_error);
        }
    }
    return buffer;
}