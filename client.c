#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>

#define SERVICE "7777"

int main(int argc, char const *argv[]) {
  int sock_fd;
  struct sockaddr_in6 server_addres;
  int ret;
  char nick[50];
  char buffer[1052];
  struct addrinfo hints, *result, *rp;
  char str_addr[INET6_ADDRSTRLEN];
  int len6 = sizeof(struct sockaddr_in6);
  int len4 = sizeof(struct sockaddr_in);

  setlocale(LC_CTYPE,"en_US.UTF-8");

  if (argc != 3) {
    printf("Syntax: %s server_address nick_name\n", argv[0]);
    return EXIT_FAILURE;
  }

  if (strlen(argv[2]) > 50) {
    printf("Nick name is tool long (max 50 characters)\n");
    return EXIT_FAILURE;
  } else {
    strcpy(nick, argv[2]);
    // mbstowcs(nick, argv[2], sizeof(argv[2]));
  }

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* Allow stream protocol */
  hints.ai_flags = 0;              /* No flags required */
  hints.ai_protocol = IPPROTO_TCP; /* Allow TCP protocol only */

  if ((ret = getaddrinfo(argv[1], SERVICE, &hints, &result)) != 0) {
    perror("getaddrinfo(): ");
    return EXIT_FAILURE;
  }

  /* Try to use addrinfo from getaddrinfo() */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (rp->ai_family == AF_INET) {
      if (inet_ntop(AF_INET, &((struct sockaddr_in *)rp->ai_addr)->sin_addr,
                    str_addr, len4) != NULL)
        printf("Trying IPv4 address: %s:%s ...\n", str_addr, SERVICE);
      else
        printf("Not valid IPv4 address.\n");
    } else if (rp->ai_family == AF_INET6) {
      if (inet_ntop(AF_INET6, &((struct sockaddr_in6 *)rp->ai_addr)->sin6_addr,
                    str_addr, len6) != NULL)
        printf("Trying IPv6 address: [%s]:%s ...\n", str_addr, SERVICE);
      else
        printf("Not valid IPv6 address\n");
    }
    /* Do TCP handshake */
    if (connect(sock_fd, rp->ai_addr, rp->ai_addrlen) != -1)
      break;
    else
      printf("Failed\n");
    close(sock_fd);
    sock_fd = -1;
  }
  if (rp == NULL) {
    printf("Could not connect to the [%s]:%s\n", argv[1], SERVICE);
    freeaddrinfo(result);
    return EXIT_FAILURE;
  }

  printf("Connected\n");

  write(sock_fd, nick, sizeof(nick));
//  printf("jmeno pred odeslanim: %s\n", nick);
  read(sock_fd, &buffer, sizeof(buffer));
  printf("You are connected under name %s\n",
         buffer);

  while (1) {
    char *line = NULL;
    size_t size = 0;
    size_t pocet_znaku = 0;
    fd_set rfds;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(sock_fd, &rfds);
    FD_SET(0,&rfds);
    int retval = 1;
  //  while (retval > 0) {
      // strcpy(buffer, "");
      memset(buffer, 0, 1000); // try
      retval = select(4, &rfds, NULL, NULL, &tv);
      // printf("%d\n", retval);
      // printf("%d\n", sock_fd);
      if (retval == -1)
        perror("select()");
      else if (retval > 0) {
               if (FD_ISSET(0, &rfds)){
                   free(line);
                 if ((pocet_znaku = getline(&line, &size, stdin)) == -1) {
                 //  printf("nic \n");
                 } else {
                   if (pocet_znaku > 1000) {
                     printf("Text is too long, max allowed number of characters is 1000 per "
                            "line\n");
                   } else {
                     if (strlen(line)>1)
                       write(sock_fd, line, pocet_znaku);
                      //printf("Odeslan text: %s, pocet znaku %lu\n", line,pocet_znaku);
                   }
                 }
               }
               if (FD_ISSET(sock_fd, &rfds)){
                 read(sock_fd, &buffer, sizeof(buffer));
                 printf("%s", buffer);
               }
      }
  //  }
    /* FD_ISSET(0, &rfds) will be true. */
    //  else
    //      printf("No data within five seconds.\n");
    //  int red = 1;
    //  while (red > 0)
    //    red = read(sock_fd, &buffer, sizeof(buffer));
    //  printf("%s\n", buffer);
  }

  close(sock_fd);
  return EXIT_SUCCESS;
}
