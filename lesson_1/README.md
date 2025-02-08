# Lesson 1: 8.02.2025

```C++
int socket(int family, int type, int protocol);
```
* `family`: `AF_INET` или `PF_INET`;
* `type`: `SOCK_DGRAM` (UDP) или `SOCK_STREAM` (TCP)
* `protocol`: по умолчанию 0


```C++
int bind(int sockd, struct sockaddr *addr, int addrlen);
```
* `addr`: чаще всего будем использовать  (для TCP и UDP); 
* `addrlen` - длина структуры; res 0 - ok, <0 - error
* `res`: `== 0: ok`, `< 0: !error!`

```C
struct sockaddr_in {
	short sin_family;
	unsigned short sin_port;
	struct in_addr {
		unsigned long s_addr;
	}
}
```
* `sin_family`: (AF_INET)
  * `1-1023`: системные приложения
  * `1024-49151`: user-space приложения
  * `0` - ОС автоматически выделит порт
* `s_addr`: `inet_aton(char*)`

```C++
int sendto(int sockd, void *addr, int len, int flags,  
           struct sockaddr *to, int token);
```
* `res` : `>= 0` - реально отправлено; `< 0` - ошибка

```C++
int recvto(int sockd, void *addr, int maxlen,  int flags,              
           struct sockaddr *from, int *maxfromlen);
```
* `maxfromlen`: максимальная длина адреса
* `res` : `>= 0` - принято; `< 0` - ошибка

```
/tmp/FALT
10.55.162.165
93.175.30.74
/sbin/ipconfig
```
