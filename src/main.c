#include <stdio.h>
#include <string.h>
#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART

void leitura(char codigo) {
    int uart0_filestream = -1;

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);      //Open in non blocking read/write mode
    if (uart0_filestream == -1) {
        printf("Erro - Não foi possível iniciar a UART.\n");
        return;
    }
       
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;     //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    char padrao[] = {codigo, 0, 3, 9, 4};

    int count = write(uart0_filestream, &padrao[0], 5);

    if(count < 0) {
        printf("Erro - Não foi possível enviar a mensagem de solicitação para a UART.\n");
        close(uart0_filestream);
        return;
    }

    sleep(1);

    int num_int;
    float num_float;

    // (size + str) 1 + 255 = 256
    char str[256];

    // 255 bytes + \0
    char msg[256];
    int size;

    switch (codigo) {
    case 0xA1:
        count = read(uart0_filestream, (void*)num_int, 4);
        break;
    case 0xA2:
        count = read(uart0_filestream, (void*)((int)num_float), 4);
        break;
    case 0xA3:
        count = read(uart0_filestream, (void*)str, 256);
        size = str[0] < 0 ? 256 + str[0] : str[0];
        strncpy(msg, str+1, size);
        break;
    default:
        break;
    }

    if(count < 0) {
        printf("Erro na leitura.\n");
    }
    else if(count == 0) {
        printf("Nenhum dado disponível.\n");
    }
    else {
        switch (codigo) {
            case 0xA1:
                printf("%d\n", num_int);
                break;
            case 0xA2:
                printf("%f\n", num_float);
                break;
            case 0xA3:
                printf("%s\n", msg);
                break;
            default:
                break;
        }
    }

    close(uart0_filestream);
}

void escrita(char codigo, int num_int, float num_float, char str[]) {
    int uart0_filestream = -1;

    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY);      //Open in non blocking read/write mode
    if (uart0_filestream == -1) {
        printf("Erro - Não foi possível iniciar a UART.\n");
        return;
    }
       
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD;     //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    // comando + dado + matricula = 1 + 4 + 4 = 9
    char msg_number[9];

    // comando + size + string + matricula = 1 + 1 + 255 + 4
    char msg_string[261];
    int str_size;

    int count;

    switch (codigo) {
    case 0xB1:
        msg_number[0] = codigo;
        memcpy(&str[1], &num_int, 4);
        msg_number[5] = 0;
        msg_number[6] = 3;
        msg_number[7] = 9;
        msg_number[8] = 4;
        count = write(uart0_filestream, &msg_number[0], 9);
        break;
    case 0xB2:
        msg_number[0] = codigo;
        memcpy(&str[1], &num_float, 4);
        msg_number[5] = 0;
        msg_number[6] = 3;
        msg_number[7] = 9;
        msg_number[8] = 4;
        count = write(uart0_filestream, &msg_number[0], 9);
        break;
    case 0xB3:
        str_size = strlen(str);
        msg_string[0] = codigo;
        msg_string[1] = str_size;
        memcpy(&msg_string[2], str, str_size);
        msg_string[str_size+2] = 0;
        msg_string[str_size+3] = 3;
        msg_string[str_size+4] = 9;
        msg_string[str_size+5] = 4;
        count = write(uart0_filestream, &msg_string[0], 6+str_size);
        break;
    default:
        break;
    }

    if(count < 0) {
        printf("Erro - Não foi possível enviar a mensagem para a UART.\n");
    }

    close(uart0_filestream);
}

int main(int argc, const char * argv[]) {
    int opt = -1;

    printf("MENU\n\n");

    printf("1. Solicitar Inteiro\n");
    printf("2. Solicitar Float\n");
    printf("3. Solicitar String\n");
    printf("4. Enviar Inteiro\n");
    printf("5. Enviar Float\n");
    printf("6. Enviar String\n\n");

    printf("0. Sair\n\n");

    printf("Escolha: ");

    scanf(" %d", &opt);

    while(opt < 0 || opt > 6) {
        printf("Escolha deve estar entre 0 e 6: ");
        scanf(" %d", &opt);
    }

    int num_int;
    float num_float;

    // 255 bytes + \0
    char str[256];

    switch (opt) {
    case 1:
        leitura(0xA1);
        break;
    case 2:
        leitura(0xA2);
        break;
    case 3:
        leitura(0xA3);
        break;
    case 4:
        printf("Digite o inteiro que deseja enviar: ");
        scanf(" %d", &num_int);
        escrita(0xB1, num_int, 0, "");
        break;
    case 5:
        printf("Digite o float que deseja enviar: ");
        scanf(" %f", &num_float);
        escrita(0xB2, 0, num_float, "");
        break;
    case 6:
        printf("Digite a string que deseja enviar (máximo de 255 caracteres): ");
        scanf("%255s", str);
        escrita(0xB3, 0, 0, str);
        break;
    default:
        break;
    }

   return 0;
}