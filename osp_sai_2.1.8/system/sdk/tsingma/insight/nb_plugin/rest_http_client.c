

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "insight.h"
#include "rest.h"
#include "rest_http.h"

#define  REST_CONNECT_WAIT_TIME_SEC    0
#define  REST_CONNECT_WAIT_TIME_USEC   (5*100*1000)
#define  REST_SEND_WAIT_TIME_SEC       0
#define  REST_SEND_WAIT_TIME_USEC      (5*100*1000)

/*************************************************************************
 * @brief      Performs a TCP connection operation in a non-blocking mode
 *
 * @param[in]  sockfd     Connecting socket identifier.
 * @param[in]  servAddr   Remote peer address
 *
 * @retval     INSIGHT_STATUS_SUCCESS  Connection established successfully.
 *             INSIGHT_STATUS_FAILURE  Failed to establish connection
 * @note
 **************************************************************************/
INSIGHT_STATUS rest_non_block_connect(int sockFd,
                                    const struct sockaddr *servAddr)
{
  fd_set  clientSet;
  struct  timeval timeout;
  int     flags, result, temp;

  flags = fcntl(sockFd, F_GETFL, 0);
  if (flags < 0)
  {
    _REST_LOG(_REST_DEBUG_ERROR, "Error Reading client socket\r \n");
    return INSIGHT_STATUS_FAILURE;
  }

  if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) <0)
  {
    _REST_LOG(_REST_DEBUG_ERROR, "Error Setting client socket to non-blocking \r \n");
    return INSIGHT_STATUS_FAILURE;
  }

  FD_ZERO(&clientSet);
  FD_SET(sockFd, &clientSet);
  timeout.tv_sec = REST_CONNECT_WAIT_TIME_SEC;
  timeout.tv_usec = REST_CONNECT_WAIT_TIME_USEC;

  /* connect to the peer */
  temp = connect(sockFd, (struct sockaddr *) servAddr, sizeof (struct sockaddr));

  if(temp < 0)
  {
    if (errno == EINPROGRESS)
    {
      result = select(sockFd + 1, NULL, &clientSet, NULL, &timeout);

      if (result <= 0)
      {
        _REST_LOG(_REST_DEBUG_ERROR, "Error connecting to client \r \n");
        return INSIGHT_STATUS_FAILURE;
      }
    }
    else
    {
      return INSIGHT_STATUS_FAILURE;
    }
  }

  if (fcntl(sockFd, F_SETFL, (flags & ~(O_NONBLOCK)) < 0))
  {
    _REST_LOG(_REST_DEBUG_ERROR, "Error Setting client socket to blocking \r \n");
  }
  return INSIGHT_STATUS_SUCCESS;
}



/*************************************************************************
 * @brief      Performs a socket send operation in a non-blocking mode
 *
 * @param[in]  sockfd     Connecting socket identifier.
 * @param[in]  buf        Data buffer pointer
 * @param[in]  length     Data buffer length.
 * &param[in]  sendFlags  Send operation flags.
 *
 * @retval     INSIGHT_STATUS_SUCCESS  Connection established successfully.
 *             INSIGHT_STATUS_FAILURE  Failed to establish connection
 * @note
 **************************************************************************/
INSIGHT_STATUS rest_non_block_send(int sockFd, const void *buf, int length, int sendFlags)
{
  fd_set  clientSet;
  struct  timeval timeout;
  int     flags, result, send_len, total_sent;

  total_sent = 0;

  flags = fcntl(sockFd, F_GETFL, 0);
  if (flags < 0)
  {
    _REST_LOG(_REST_DEBUG_ERROR, "Error Reading client socket\r\n");
    return INSIGHT_STATUS_FAILURE;
  }

  if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) < 0)
  {
    _REST_LOG(_REST_DEBUG_ERROR, "Error Setting client socket to non-blocking\r\n");
    return INSIGHT_STATUS_FAILURE;
  }

  FD_ZERO(&clientSet);
  FD_SET(sockFd, &clientSet);
  timeout.tv_sec = REST_SEND_WAIT_TIME_SEC;
  timeout.tv_usec = REST_SEND_WAIT_TIME_USEC;

  /* send to the peer */
  do
  {
    send_len = send(sockFd, (buf + total_sent), length, 0);
    if (send_len < 0)
    {

      /* buffer is full, we may block */
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        /* wait for some time, or get notified when sent */
        result = select(sockFd + 1, NULL, &clientSet, NULL, &timeout);
        if (result <= 0)
        {
          _REST_LOG(_REST_DEBUG_INFO, "Sending failed on socket=%d result=%d\r\n",
                    sockFd, result);
          return INSIGHT_STATUS_FAILURE;
        }
        continue;
      }
      else
      {
        return INSIGHT_STATUS_FAILURE;
      }
    }
    total_sent = send_len + total_sent;
    length = length - send_len;
  } while(length > 0);


  if (fcntl(sockFd, F_SETFL, (flags & ~(O_NONBLOCK)) < 0))
  {
    _REST_LOG(_REST_DEBUG_ERROR, "Error Setting client socket to blocking\r\n");
  }

  return INSIGHT_STATUS_SUCCESS;
}



/******************************************************************
 * @brief  sends a HTTP 200 message to the client
 *
 * @param[in]   fd    socket for sending message
 *
 * @retval   INSIGHT_STATUS_SUCCESS
 *
 * @note
 *********************************************************************/
INSIGHT_STATUS rest_send_200_with_data(int fd, char *buffer, int length)
{
    char *response = "HTTP/1.1 200 OK \r\n"
            "Server: BroadViewAgent (Unix) (Linux) \r\n"
            "Content-Type: text/json \r\n\r\n";
    int bytes_sent =0;
    INSIGHT_STATUS  rv = INSIGHT_STATUS_SUCCESS;
    int sendbuff =0;

    if (0 > send(fd, response, strlen(response), MSG_MORE))
    {
      rv = INSIGHT_STATUS_FAILURE;
    }
    bytes_sent = send(fd, buffer, length, MSG_MORE);
    if (0 > bytes_sent)
    {
      rv = INSIGHT_STATUS_FAILURE;
    }


    while (length - bytes_sent > 0)
    {
       /* Calling continues send with buffer size more than 8K will result in ERROR from
        * kernel so increase kernel buffer size for this FD.
        */
       sendbuff = length;
       if (0 > setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff)))
       {
         return INSIGHT_STATUS_FAILURE;
       }

       buffer = buffer + bytes_sent;
       length = length - bytes_sent;
       bytes_sent = send(fd, buffer, length, 0);
       if (0 > bytes_sent)
       {
         return INSIGHT_STATUS_FAILURE;
       }
    }
    return rv;
}

/******************************************************************
 * @brief  sends a HTTP 200 message to the client
 *
 * @param[in]   fd    socket for sending message
 *
 * @retval   INSIGHT_STATUS_SUCCESS
 *
 * @note
 *********************************************************************/
INSIGHT_STATUS rest_send_200(int fd)
{
    char *response = "HTTP/1.1 200 OK \r\n"
            "Server: BroadViewAgent (Unix) (Linux) \r\n\r\n";

    if (0 > send(fd, response, strlen(response), 0))
        return INSIGHT_STATUS_FAILURE;

    return INSIGHT_STATUS_SUCCESS;
}

/******************************************************************
 * @brief  sends a HTTP 404 message to the client
 *
 * @param[in]   fd    socket for sending message
 *
 * @retval   INSIGHT_STATUS_SUCCESS
 *
 * @note
 *********************************************************************/
INSIGHT_STATUS rest_send_404(int fd)
{
    char *response = "HTTP/1.1 404 Not Found \r\n"
            "Server: BroadViewAgent (Unix) (Linux) \r\n\r\n "
            "<html> <body> Unsupported </body> </html>";

    if (0 > send(fd, response, strlen(response), 0))
        return INSIGHT_STATUS_FAILURE;

    return INSIGHT_STATUS_SUCCESS;
}

/******************************************************************
 * @brief  sends a HTTP 400 message to the client
 *
 * @param[in]   fd    socket for sending message
 *
 * @retval   INSIGHT_STATUS_SUCCESS
 *
 * @note
 *********************************************************************/
INSIGHT_STATUS rest_send_400(int fd)
{
    char *response = "HTTP/1.1 400 Bad Request \r\n"
            "Server: BroadViewAgent (Unix) (Linux) \r\n\r\n"
            "<html> <body> Bad Request </body> </html>";

   if (0 > send(fd, response, strlen(response), 0))
        return INSIGHT_STATUS_FAILURE;

    return INSIGHT_STATUS_SUCCESS;
}

/******************************************************************
 * @brief  sends a HTTP 500 message to the client
 *
 * @param[in]   fd    socket for sending message
 *
 * @retval   INSIGHT_STATUS_SUCCESS
 *
 * @note
 *********************************************************************/
INSIGHT_STATUS rest_send_500(int fd)
{
    char *response = "HTTP/1.1 500 Internal Server Error \r\n"
            "Server: BroadViewAgent (Unix) (Linux) \r\n\r\n"
            "<html> <body> Internal Server Error </body> </html>";

   if (0 > send(fd, response, strlen(response), 0))
        return INSIGHT_STATUS_FAILURE;

    return INSIGHT_STATUS_SUCCESS;
}

/******************************************************************
 * @brief  sends an asynchronous report to the client
 *
 * @param[in]   rest    context for reading configuration
 * @param[in]   buffer  Buffer containing data to be sent
 * @param[in]   length  number of bytes to be sent
 *
 * @retval   INSIGHT_STATUS_SUCCESS if send is successful
 *
 * @note
 *********************************************************************/
INSIGHT_STATUS rest_send_async_report(REST_CONTEXT_t *rest, char *buffer, int length)
{
    char *header = "POST /agent_response HTTP/1.1\r\n"
            "Host: BVIEW Client\r\n"
            "User-Agent: BroadViewAgent\r\n"
            "Accept: text/html,application/xhtml+xml,application/xml\r\n"
            "Content-Length: %d\r\n"
            "\r\n";

    char buf[REST_MAX_HTTP_BUFFER_LENGTH] = { 0 };
    int clientFd;
    struct sockaddr_in clientAddr;
    int temp = 0;
    INSIGHT_STATUS rv = INSIGHT_STATUS_SUCCESS;

    snprintf(buf, REST_MAX_HTTP_BUFFER_LENGTH - 1, header, length);

    /* create socket to send data to */
    clientFd = socket(AF_INET, SOCK_STREAM, 0);
    _REST_ASSERT_NET_ERROR((clientFd != -1), "Error Creating server socket");

    /* take lock while copying the info */
    if (0 != pthread_mutex_lock (&rest->config_mutex))
    {
      LOG_POST (INSIGHT_LOG_ERROR,
          "Failed to take the lock for rest config \r\n");
      close(clientFd);
      return INSIGHT_STATUS_FAILURE;
    }

    /* setup the socket */
    memset(&clientAddr, 0, sizeof (struct sockaddr_in));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(rest->config.clientPort);

    temp = inet_pton(AF_INET, &rest->config.clientIp[0], &clientAddr.sin_addr);
    /* release the lock */
    if (0 != pthread_mutex_unlock (&rest->config_mutex))
    {
      LOG_POST (INSIGHT_LOG_ERROR,
          "Failed to release the lock for rest config \r\n");
      close(clientFd);
      return INSIGHT_STATUS_FAILURE;
    }
    _REST_ASSERT_NET_SOCKET_ERROR((temp > 0), "Error Creating server socket",clientFd);


    temp = rest_non_block_connect(clientFd, (struct sockaddr *) &clientAddr);
    if (temp != INSIGHT_STATUS_SUCCESS)
    {
      close(clientFd);
      /* Do not return error if the client is default client */
      if ((SYSTEM_CONFIG_PROPERTY_CLIENT_PORT_DEFAULT == rest->config.clientPort) &&
          (0 == strcmp(&rest->config.clientIp[0], SYSTEM_CONFIG_PROPERTY_CLIENT_IP_DEFAULT)))
      {
        return INSIGHT_STATUS_SUCCESS;
      }
      else
      {
        return INSIGHT_STATUS_FAILURE;
      }
    }
    rv = rest_non_block_send(clientFd, buf, strlen(buf), MSG_MORE);
    if (rv != INSIGHT_STATUS_SUCCESS)
    {
      close(clientFd);
      /* Do not return error if the client is default client */
      if ((SYSTEM_CONFIG_PROPERTY_CLIENT_PORT_DEFAULT == rest->config.clientPort) &&
          (0 == strcmp(&rest->config.clientIp[0], SYSTEM_CONFIG_PROPERTY_CLIENT_IP_DEFAULT)))
      {
        return INSIGHT_STATUS_SUCCESS;
      }
      else
      {
        return INSIGHT_STATUS_FAILURE;
      }
    }

    rv = rest_non_block_send(clientFd, buffer, length, 0);
    if (rv != INSIGHT_STATUS_SUCCESS)
    {
      close(clientFd);
      return INSIGHT_STATUS_FAILURE;
    }


    /* close down session */
    close(clientFd);

    return rv;

}

/******************************************************************
 * @brief  sends a HTTP 404 message to the client
 *
 * @param[in]   fd    socket for sending message
 * @param[in]   buffer  Buffer containing data to be sent
 * @param[in]   length  number of bytes to be sent
 *
 * @retval   INSIGHT_STATUS_SUCCESS if send is successful
 *
 * @note
 *********************************************************************/
INSIGHT_STATUS rest_send_404_with_data(int fd, char *buffer, int length)
{
    char *response = "HTTP/1.1 404 Not Found \r\n"
            "Server: BroadViewAgent (Unix) (Linux) \r\n\r\n "
             "Content-Type: text/json";

    if (0 > send(fd, response, strlen(response), MSG_MORE))
      return INSIGHT_STATUS_FAILURE;

    if (0 > send(fd, buffer, length, 0))
      return INSIGHT_STATUS_FAILURE;

    return INSIGHT_STATUS_SUCCESS;
}

/******************************************************************
 * @brief  sends a HTTP 400 message to the client
 *
 * @param[in]   fd    socket for sending message
 * @param[in]   buffer  Buffer containing data to be sent
 * @param[in]   length  number of bytes to be sent
 *
 * @retval   INSIGHT_STATUS_SUCCESS if send is successful
 *
 * @note
 *********************************************************************/
INSIGHT_STATUS rest_send_400_with_data(int fd, char *buffer, int length)
{
    char *response = "HTTP/1.1 400 Bad Request \r\n"
            "Server: BroadViewAgent (Unix) (Linux) \r\n\r\n"
             "Content-Type: text/json";

    if (0 > send(fd, response, strlen(response), MSG_MORE))
      return INSIGHT_STATUS_FAILURE;

    if (0 > send(fd, buffer, length, 0))
      return INSIGHT_STATUS_FAILURE;

    return INSIGHT_STATUS_SUCCESS;
}

/******************************************************************
 * @brief  sends a HTTP 403 message to the client
 *
 * @param[in]   fd    socket for sending message
 * @param[in]   buffer  Buffer containing data to be sent
 * @param[in]   length  number of bytes to be sent
 *
 * @retval   INSIGHT_STATUS_SUCCESS if send is successful
 *
 * @note
 *********************************************************************/
INSIGHT_STATUS rest_send_403_with_data(int fd, char *buffer, int length)
{
    char *response = "HTTP/1.1 400 Forbidden \r\n"
            "Server: BroadViewAgent (Unix) (Linux) \r\n\r\n"
             "Content-Type: text/json";

    if (0 > send(fd, response, strlen(response), MSG_MORE))
      return INSIGHT_STATUS_FAILURE;

    if (0 > send(fd, buffer, length, 0))
      return INSIGHT_STATUS_FAILURE;

    return INSIGHT_STATUS_SUCCESS;
}
/******************************************************************
 * @brief  sends a HTTP 500 message to the client
 *
 * @param[in]   fd    socket for sending message
 * @param[in]   buffer  Buffer containing data to be sent
 * @param[in]   length  number of bytes to be sent
 *
 * @retval   INSIGHT_STATUS_SUCCESS if send is successful
 *
 * @note
 *********************************************************************/
INSIGHT_STATUS rest_send_500_with_data(int fd, char *buffer, int length)
{
    char *response = "HTTP/1.1 500 Internal Server Error \r\n"
            "Server: BroadViewAgent (Unix) (Linux) \r\n\r\n"
             "Content-Type: text/json";

    if (0 > send(fd, response, strlen(response), MSG_MORE))
      return INSIGHT_STATUS_FAILURE;

    if (0 > send(fd, buffer, length, 0))
      return INSIGHT_STATUS_FAILURE;

    return INSIGHT_STATUS_SUCCESS;
}


