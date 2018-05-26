#include <string.h>
#include <string>
#include <iostream>
#include <boost/make_shared.hpp>
#include <boost/network/utils/thread_pool.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/io_service.hpp> 
#include <boost/asio/ssl/context.hpp> 
#include <json/json.h>
#include <json/value.h> 
#include "string_tools.h"
#include "logging.h"
#include "http_server.h"
#include "configuration.h"
#include "ipvs_interface.h"

// globals --------------------------------------------------------------------

// Mutex to access configuration object.
boost::mutex configuration_mutex;

// IPVS kernel interface.
ipvs_interface ipvs;

// IPVS kernel interface mutex.
boost::mutex ipvs_mutex;

// http_server ----------------------------------------------------------------

// Constructor.
http_server::http_server(
    const std::string& listen_address,
    int port, 
    int threads) 
    : m_listen_address(listen_address), m_port(port), 
      m_num_threads(threads), m_server(0)
{

}

// Destructor.
http_server::~http_server()
{
    stop();
}

// Starts the server.
bool http_server::start()
{
    // Initialize the IPVS interface.
    ipvs_mutex.lock();
    if (ipvs.init() != true) 
    {
        ipvs_mutex.unlock();
        return false;
    }
    ipvs_mutex.unlock();
    // Set up IO service.
    std::shared_ptr<boost::asio::io_service> io_service(
        std::make_shared<boost::asio::io_service>());
    // Make sure we are starting from a stopped state.
    if (m_server) stop();
    // Set up SSL context if ssl is enabled.
    std::shared_ptr<boost::asio::ssl::context> ssl_context = 0;
    if (config.get_ssl() == true)
    {
        log_message("Enabling SSL.");
        ssl_context = std::make_shared<boost::asio::ssl::context>(
            boost::asio::ssl::context::sslv23);
        ssl_context->set_options(
            boost::asio::ssl::context::default_workarounds | 
            boost::asio::ssl::context::no_sslv2 );
        ssl_context->use_certificate_chain_file(
            config.get_ssl_cert_chain_file());
        ssl_context->use_private_key_file(config.get_ssl_key_file(),
            boost::asio::ssl::context::pem);
        ssl_context->use_tmp_dh_file(config.get_ssl_dh_file());
    }
    // Create server and set options.
    m_server = new server_t(
        server_t::options(m_server_handler)
            .address(m_listen_address)
            .port(str_from_number(m_port))
            .io_service(io_service)
            .reuse_address(true)
            .thread_pool(
                std::make_shared<boost::network::utils::thread_pool>(
                    m_num_threads))
            .context(ssl_context)
    );
    // Setup clean shutdown signal to a lambda that calls stop().
    boost::asio::signal_set signals(*io_service, SIGINT, SIGTERM);
    signals.async_wait(
        [=](boost::system::error_code const& ec, int signal){
            stop();
        }
    );
    log_message("Server start.");
    // Run server.
    m_server->run();
    // Cleanup
    io_service->stop();
    return true;
}

// Stops the server.
void http_server::stop()
{
    if (m_server)
    {
        log_message("Server stop.");
        m_server->stop();
        delete m_server;
        m_server = 0;
    }
}

// async_http_server ----------------------------------------------------------

// HTTP connection handler functor.
void async_http_server::operator()(
    server_t::request const& request,
    server_t::connection_ptr connection)
{
    boost::shared_ptr<connection_handler> h(new connection_handler(request));
    (*h)(connection);
}

// connection_handler ---------------------------------------------------------

// Constructor.
connection_handler::connection_handler(server_t::request const& request)
    : m_request(request)
{
}

// Destructor.
connection_handler::~connection_handler()
{

}

// Handles an incomming connection.
void connection_handler::operator()(server_t::connection_ptr connection)
{
    // Parse content-length from headers.
    int content_length=0;
    request_headers_t const& headers = m_request.headers;
    for(request_headers_t::const_iterator it = headers.begin(); 
        it!= headers.end(); ++it)
    {
        if (str_to_lower(it->name) == "content-length")
        {
            content_length = atoi(it->value.c_str());
            break;
        }
    }
    // Read data.
    read_chunk(content_length,connection);
}


void connection_handler::read_chunk(
    size_t left_to_read, 
    server_t::connection_ptr connection)
{
    connection->read(
        boost::bind(
            &connection_handler::handle_post_read,
            connection_handler::shared_from_this(),
            _1, _2, _3, connection, left_to_read));
}

void connection_handler::handle_post_read(
    server_t::connection::input_range range,
    boost::system::error_code error,
    size_t size,
    server_t::connection_ptr connection,
    size_t left_to_read)
{
    if (error) return;
    // Append to buffer.
    m_request_body.append(boost::begin(range),size);
    // If we have more left to read, recurse.
    size_t remaining = left_to_read - size;
    if (remaining > 0)
    {
        read_chunk(remaining,connection);
        return;
    }
    // If we get to this point, we're done reading.
    handle_post_request(connection);
}

// Logs an error and reports it back to the client.
void connection_handler::report_error(
    const std::string& message, 
    server_t::connection_ptr connection,
    server_t::connection::status_t status)
{
    server_t::response_header headers[] = { 
        {"Connection","close"},
        {"Content-Type", "application/json"} 
    };
    connection->set_status(status);
    connection->set_headers(boost::make_iterator_range(headers,headers+2));    
    connection->write(
        "\r"
        "{\n"
        "\t\"error\": true,\n"
        "\t\"error_reason\": \""+message+"\"\n"
        "}\n");
    log_error(message);
}

void connection_handler::handle_post_request(
    server_t::connection_ptr connection)
{
    Json::Reader reader;
    Json::Value root;
    // Set response headers.
    server_t::response_header headers[] = { 
        {"Connection","close"},
        {"Content-Type", "application/json"} 
    };
    // Get config values.
    configuration_mutex.lock();
    std::string c_api_key = config.get_api_key();
    std::string c_vip = config.get_vip();
    log_message("api key : " + c_api_key);
    log_message("vip : " + c_vip);
    configuration_mutex.unlock();
    // Parse json input.
    if (!reader.parse(m_request_body,root))
    {
        report_error("Could not parse JSON.",connection,
            server_t::connection::bad_request);
        return;
    }
    const Json::Value api_key = root["api_key"];
    if (!api_key)
    {
        report_error("Missing API key.",connection,
            server_t::connection::unauthorized);
        return;
    }
    if (api_key != c_api_key)
    {
        report_error("Invalid API key.",connection,
            server_t::connection::unauthorized);
        return;
    }
    const Json::Value hostname = root["host"];
    if (!hostname)
    {
        report_error("No host specified.",connection,
            server_t::connection::bad_request);
        return;
    }
    const Json::Value kernels = root["available_kernels"];
    if (!kernels)
    {
        report_error("No available_kernels specified.",connection,
            server_t::connection::bad_request);
        return;
    }
    if (kernels.isInt() != true)
    {
        report_error("available_kernels needs to be an integer value."
            ,connection, server_t::connection::bad_request);
        return;
    }
    bool success = false;
    ipvs_mutex.lock();
    success = ipvs.set_weight(c_vip.c_str(),hostname.asString(),kernels.asInt());
    if (success != true)
    {
        report_error("Could not set weight.",connection);
    }
    else
    {
        connection->set_status(server_t::connection::ok);
        connection->set_headers(boost::make_iterator_range(headers,headers+2));    
        connection->write("\r{\n\t\"error\": false\n}\n");
    }
    ipvs_mutex.unlock();
}


