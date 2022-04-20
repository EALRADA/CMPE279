// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pwd.h>
#include <sys/wait.h>
#define PORT 80

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
    
    // Show ASLR
    //printf("argv=%p\n", *argv);

    if (strcmp(argv[0], "exec_flag") == 0) {
        printf("This is child exec process \n");
        
        // Drop privilege on new child process
        new_socket = atoi(argv[1]);

        // find nobody user ID on this operating system 
        struct passwd *nobodyStruc = getpwnam("nobody");
        printf("Nobody_ID on this OS is: %d \n", nobodyStruc->pw_uid);

        //dropping previleges
        if (setuid(nobodyStruc->pw_uid) == -1) {
            perror("Dropping the user previlege failed! \n");
            exit(EXIT_FAILURE);
        } else {
            printf("Dropping the user privilege was successful! \n");
            valread = read(new_socket, buffer, 1024);
            printf("%s\n",buffer );
            send(new_socket, hello, strlen(hello), 0);
            printf("Hello message sent\n");

        }
    } else {
        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
        &opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons( PORT );

        if (bind(server_fd, (struct sockaddr *)&address,
        sizeof(address)) < 0)
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

        pid_t pid = fork();
        int status = 0;

        if (pid < 0) {
            perror("Creating child process using fork(), failed\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { 
            
            // Create a new process of child
            char address[10];
            snprintf(address,10,"%d",new_socket);            
            char *argvNew[] = {"exec_flag", address, NULL};

            int status = execvp("./server", argvNew);
            if(status < 0){
                perror("Creating Child process using execvp failed\n");
                exit(EXIT_FAILURE);
            }
        } else {
            // wait for the termination of child
            waitpid(pid, &status, 0);
            exit(0);        
        }
    }

    


    return 0;
}
