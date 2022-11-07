// Server side C/C++ program to demonstrate Socket programming

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pwd.h>
#include <sys/wait.h>

#define PORT 8080
int main(int argc, char const *argv[]) {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[102] = {0};
    char *hello = "Hello from server";
    char *pname = NULL;
    
    printf("execve=0x%p\n", execve);

    //check which version of the process this is
    if(strcmp(argv[0], "lower-priv")==0) {
        pname = "lower-priv";
    }

    if (pname == NULL) { //initial process...
        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Attaching socket to port 80
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                      &opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons( PORT );

        // Forcefully attaching socket to the port 80
        if (bind(server_fd, (struct sockaddr *)&address,
                                     sizeof(address))<0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(server_fd, 3) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        //fork and exec
        pid_t pid = fork();
        if (pid < 0) { 
            perror("error with fork");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0) { //child
            //duplicate socket
            int duplicate_socket;
            if (dup2(new_socket, duplicate_socket) == -1) {
                perror("dup2 error");
                exit(EXIT_FAILURE);
            }

            //use socket_str to pass the socket value
            char socket_str[10]; 
            sprintf(socket_str, "%i", duplicate_socket);

            char *nargs[] = {"lower-priv", socket_str, NULL}; //note that arg list has to be null-terminated
            execvp(argv[0], nargs);

            exit(0);
        }
        else { //parent
            wait(NULL); //wait for child process to exit
            exit(0);
        }
    }
    else if (strcmp( pname,"lower-priv" )==0) { //inside exec-ed process
        //drop privilege to "nobody"
        int nobody_uid = getpwnam("nobody")->pw_uid;
        setuid(nobody_uid);

        //read from socket
        int dup_socket = atoi(argv[1]);
        valread = read(dup_socket , buffer, 1024);
        printf("%s\n", buffer);

        //reply
        send(new_socket , hello , strlen(hello) , 0 );
        printf("Hello message sent\n");
    }
    return 0;
}
