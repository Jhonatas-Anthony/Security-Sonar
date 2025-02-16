/**
 * @file wifi.c
 * @brief Implementação das funções de conectividade Wi-Fi e requisições HTTP.
 * 
 * Este arquivo contém funções para conectar a uma rede Wi-Fi, enviar requisições
 * HTTP e gerenciar respostas recebidas do servidor.
 * 
 * @author Jhonatas Anthony Dantas Araújo
 * @date 2025
 */

#include "include/wifi.h"

#define RESPONSE_BUFFER_SIZE 2048                       // Tamanho do buffer de resposta HTTP.

static char response_buffer[RESPONSE_BUFFER_SIZE];      // Buffer global para armazenar a resposta do servidor.

static int response_length      = 0;                    // Comprimento da resposta armazenada no buffer.
volatile bool response_complete = false;                // Flag para indicar se a resposta HTTP foi completamente recebida.

static ip_addr_t server_ip;                             // Endereço IP do servidor após resolução do DNS.
static char stored_http_request[1000];                  // Buffer para armazenar a requisição HTTP antes de ser enviada.                            

/**
 * @brief Callback para processamento da resposta HTTP.
 * 
 * Copia os dados da resposta para o buffer global e identifica o fim da transmissão.
 * 
 * @param arg Argumento passado (não utilizado).
 * @param tpcb Ponteiro para o controle do bloco TCP.
 * @param p Estrutura contendo os dados recebidos.
 * @param err Código de erro, se houver.
 * @return err_t Código de erro da biblioteca lwIP.
 */
err_t http_client_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    printf("Callback HTTP\n");
    if (p == NULL) {
        // O servidor fechou a conexão: resposta completa.
        response_buffer[response_length] = '\0'; // Termina a string

        // Se desejar, extraia o corpo da resposta:
        char *body = strstr(response_buffer, "\r\n\r\n");
        if (body) {
            // DESCOMENTAR CASO SEJA NECESSÁRIO USAR O MONITOR SERIAL PARA DEPURAR O CÓDIGO
            // printf("Corpo da resposta:\n%s\n", body + 4);
        } else {
            // DESCOMENTAR CASO SEJA NECESSÁRIO USAR O MONITOR SERIAL PARA DEPURAR O CÓDIGO
            // printf("Corpo da resposta não encontrado\n");
        }

        // Seta a flag para que o arquivo main.c saiba que a resposta está pronta
        response_complete = true;

        // Fecha a conexão
        tcp_close(tpcb);
        return ERR_OK;
    }

    // Copia os dados de todos os pbufs para o buffer global
    struct pbuf *q;
    for (q = p; q != NULL; q = q->next) {
        if (response_length + q->len < RESPONSE_BUFFER_SIZE) {
            memcpy(response_buffer + response_length, q->payload, q->len);
            response_length += q->len;
        } else {
            printf("Buffer de resposta cheio\n");
            break;
        }
    }

    pbuf_free(p);
    return ERR_OK;
}

/**
 * @brief Callback chamado ao estabelecer conexão TCP com o servidor.
 * 
 * Envia a requisição HTTP armazenada no buffer `stored_http_request`.
 * 
 * @param arg Argumento passado (não utilizado).
 * @param tpcb Ponteiro para o controle do bloco TCP.
 * @param err Código de erro da conexão.
 * @return err_t Código de erro da biblioteca lwIP.
 */
err_t custom_tcp_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        printf("Erro ao conectar ao servidor: %d\n", err);
        return err;
    }

    printf("Conexão TCP estabelecida. Enviando requisição...\n");

    if (tcp_write(tpcb, stored_http_request, strlen(stored_http_request), TCP_WRITE_FLAG_COPY) != ERR_OK) {
        printf("Erro ao enviar a requisição HTTP\n");
        return ERR_VAL;
    }
    tcp_output(tpcb); // Envia imediatamente
    printf("Requisição HTTP enviada\n");

    return ERR_OK;

}

/**
 * @brief Callback chamado após a resolução do DNS.
 * 
 * Obtém o endereço IP do servidor e inicia a conexão TCP.
 * 
 * @param name Nome do host consultado.
 * @param ipaddr Endereço IP resolvido.
 * @param callback_arg Argumento opcional passado ao resolver DNS.
 */
void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr == NULL) {
        printf("Erro ao resolver o endereço do servidor\n");
        return;
    }
    printf("DNS resolvido: %s\n", ipaddr_ntoa(ipaddr));
    // Cria o PCB e conecta ao servidor
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Erro ao criar PCB\n");
        return;
    }
    tcp_recv(pcb, http_client_callback);
    if (tcp_connect(pcb, ipaddr, 80, custom_tcp_connected_callback) != ERR_OK) {
        printf("Erro ao conectar ao servidor\n");
        return;
    }
}

/**
 * @brief Envia uma requisição HTTP personalizada.
 * 
 * Formata a requisição HTTP e inicia a resolução de DNS para envio ao servidor.
 * 
 * @param method Método HTTP (GET, POST, etc.).
 * @param endpoint URL do recurso requisitado.
 * @param body Corpo da requisição (caso aplicável).
 */
void send_custom_http_request(const char *method, const char *endpoint, const char *body) {
    response_length = 0;
    memset(response_buffer, 0, RESPONSE_BUFFER_SIZE);

    snprintf(stored_http_request, sizeof(stored_http_request),
             "%s %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Security-Sonar/1.0\r\n"
             "Accept: */*\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n"
             "Cache-Control: no-cache\r\n\r\n"
             "%s",
             method, endpoint, SERVER_URL, strlen(body), body);

    //printf("Preparando requisição HTTP:\n%s\n", stored_http_request);

    err_t err = dns_gethostbyname(SERVER_URL, &server_ip, dns_callback, NULL);
    if (err == ERR_OK) {
        // printf("DNS já resolvido. Conectando...\n");
        dns_callback(SERVER_URL, &server_ip, NULL);  // Chama diretamente o DNS callback para iniciar a conexão
    } else if (err == ERR_INPROGRESS) {
        // printf("Resolução do DNS em andamento...\n");
    } else {
        // printf("Erro ao iniciar a resolução do DNS\n");
    }
}

/**
 * @brief Conecta a uma rede Wi-Fi.
 * 
 * Inicializa o chip Wi-Fi, conecta ao SSID fornecido e exibe o endereço IP obtido.
 * 
 * @param ssid Nome da rede Wi-Fi.
 * @param pass Senha da rede Wi-Fi.
 */
void wifi_connect(char *ssid, char *pass) {
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar o Wi-Fi\n");
    }
    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Falha ao conectar ao Wi-Fi\n");
    } else {
        printf("Conectado.\n");
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        printf("Endereço IP %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }
    printf("Wi-Fi conectado!\n");
}

/**
 * @brief Retorna se a resposta HTTP foi completamente recebida.
 * @return true se a resposta estiver completa, false caso contrário.
 */
bool is_response_complete() {
    return response_complete;
}

/**
 * @brief Define o status de resposta completa.
 * @param status Novo valor da flag de resposta completa.
 */
void set_response_complete(bool status) {
    response_complete = status;
}

/**
 * @brief Retorna o buffer contendo a resposta do servidor.
 * @return Ponteiro para o buffer de resposta.
 */
char *get_response_buffer() {
    return response_buffer;
}

/**
 * @brief Define um novo buffer de resposta ou reseta o buffer atual.
 * @param buffer Novo buffer de resposta ou NULL para resetar.
 * @return Ponteiro para o buffer atualizado.
 */
char *set_response_buffer(char *buffer) {
    if (buffer != NULL) {
        memcpy(response_buffer, buffer, RESPONSE_BUFFER_SIZE);
    } else {
        memset(response_buffer, 0, RESPONSE_BUFFER_SIZE);
    }
    return response_buffer;
}

/**
 * @brief Desativa o Wi-Fi e libera recursos.
 */
void wifi_cleanup() {
    cyw43_arch_deinit();
}

/**
 * @brief Envia uma requisição HTTP para alterar um status no servidor.
 * @param status Novo status a ser enviado.
 */
void send_request_to_change_status(int status) {
        
    // Cria um buffer para armazenar a URL formatada
    char url[50];  // Tamanho suficiente para a URL
    sprintf(url, "/log/status/1/%d", status);
        
    // Manda a requisição HTTP
    printf(url);
    send_custom_http_request("GET", url, "{}");
}