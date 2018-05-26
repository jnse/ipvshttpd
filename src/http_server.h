#ifndef HTTP_SERVER_H_INCLUDE
#define HTTP_SERVER_H_INCLUDE

#define BOOST_NETWORK_ENABLE_HTTPS

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/thread_pool.hpp>
#include <boost/network/protocol/http/server.hpp>
#include <json/value.h> 
#include "ipvs_interface.h"

// Forward declarations.
class http_connection_handler;
class http_server;
class async_http_server;

// Typedef insane boost types
typedef boost::network::http::server<async_http_server> server_t;
typedef server_t::request::headers_container_type request_headers_t;

/// Mutex for accessing configuration object from within connection threads.
extern boost::mutex configuration_mutex;

/// IPVS kernel interface.
extern ipvs_interface ipvs;

/// IPVS kernel interface mutex.
extern boost::mutex ipvs_mutex;

/// This object handles a single incomming connection.
class connection_handler 
    : public boost::enable_shared_from_this<connection_handler>
{

    /// Holds a reference to the server request object.
    server_t::request const& m_request;

    /// Contains the request body data sent by the client.
    std::string m_request_body;

    public:

        /// Constructor.
        connection_handler(server_t::request const& request);

        /// Destructor
        ~connection_handler();

        /// Handles an incomming connection.
        void operator()(server_t::connection_ptr connection);

        /// Reads a chunk of POST data.
        void read_chunk(size_t left_to_read, 
            server_t::connection_ptr connection);

        /// Read POST data callback.
        void handle_post_read(
            server_t::connection::input_range range,
            boost::system::error_code error,
            size_t size,
            server_t::connection_ptr connection,
            size_t left_to_read
        );

        /// This is where a POST request is handled after all POST data has
        /// been read into the buffer.
        void handle_post_request(server_t::connection_ptr connection);

        /// Logs an error to the log, and reports it back to the client.
        void report_error(
            const std::string& message, 
            server_t::connection_ptr connection,
            server_t::connection::status_t status
                = server_t::connection::internal_server_error
        );

};

/// Our flavor of http server.
struct async_http_server 
{
    void operator()(
        server_t::request const& request,
        server_t::connection_ptr connection
    );
};

/// This is the main http server object.
class http_server
{
    /// Address to listen on.
    std::string m_listen_address;

    /// Port number to listen on.
    int m_port;

    /// Number of server threads to run.
    int m_num_threads;

    /// Server instance.
    server_t* m_server;

    /// Server handler instance.
    async_http_server m_server_handler;

    public:

        /// Constructor
        /// @param listen_address IP to listen on.
        /// @param port Port to listen on.
        /// @param (optional) number of threads in the thread pool to serve
        ///        connections from. (default: 2)
        http_server(const std::string& listen_address,
            int port, int threads=2);

        /// Destructor
        ~http_server();

        /// Starts the server.
        /// @return Returns true on success.
        bool start();

        /// Stops the server.
        void stop();

};

#endif
