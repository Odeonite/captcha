#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

class ProxySession : public std::enable_shared_from_this<ProxySession> {
public:
    ProxySession(tcp::socket browser_socket)
        : browser_socket_(std::move(browser_socket)),
        server_socket_(io_context) {}

    void start() {
        do_resolve();
    }

private:
    void do_resolve() {
        tcp::resolver resolver(io_context);
        tcp::resolver::query query("batman_server.com", "http");

        resolver.async_resolve(query,
            [self = shared_from_this()](const boost::system::error_code& ec,
                                        tcp::resolver::iterator endpoint_iterator) {
                if (!ec) {
                    self->do_connect(endpoint_iterator);
                } else {
                    std::cerr << "Failed to resolve: " << ec.message() << std::endl;
                }
            });
    }

    void do_connect(tcp::resolver::iterator endpoint_iterator) {
        async_connect(server_socket_, endpoint_iterator,
            [self = shared_from_this()](const boost::system::error_code& ec, tcp::endpoint) {
                if (!ec) {
                    self->do_read_browser();
                    self->do_read_server();
                } else {
                    std::cerr << "Failed to connect: " << ec.message() << std::endl;
                }
            });
    }

    void do_read_browser() {
        async_read_until(browser_socket_, browser_request_, "\r\n\r\n",
            [self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    // Modify headers if necessary
                    // e.g., browser_request_.consume(bytes_transferred);
                    self->do_write_server();
                } else {
                    std::cerr << "Failed to read browser: " << ec.message() << std::endl;
                }
            });
    }

    void do_write_server() {
        async_write(server_socket_, browser_request_,
            [self = shared_from_this()](const boost::system::error_code& ec, std::size_t) {
                if (!ec) {
                    self->do_read_browser();
                } else {
                    std::cerr << "Failed to write to server: " << ec.message() << std::endl;
                }
            });
    }

    void do_read_server() {
        async_read(server_socket_, server_response_,
            [self = shared_from_this()](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                if (!ec) {
                    // Analyze server response, check for CAPTCHA, and modify if necessary
                    // e.g., server_response_.consume(bytes_transferred);
                    self->do_write_browser();
                } else {
                    std::cerr << "Failed to read server: " << ec.message() << std::endl;
                }
            });
    }

    void do_write_browser() {
        async_write(browser_socket_, server_response_,
            [self = shared_from_this()](const boost::system::error_code& ec, std::size_t) {
                if (!ec) {
                    self->do_read_server();
                } else {
                    std::cerr << "Failed to write to browser: " << ec.message() << std::endl;
                }
            });
    }

    tcp::socket browser_socket_;
    tcp::socket server_socket_;
    io_context io_context;
    streambuf browser_request_;
    streambuf server_response_;
};

class ProxyServer {
public:
    ProxyServer(io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
        browser_socket_(io_context) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(browser_socket_,
            [this](const boost::system::error_code& ec) {
                if (!ec) {
                    std::make_shared<ProxySession>(std::move(browser_socket_))->start();
                }
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
    tcp::socket browser_socket_;
};

int main() {
    try {
        boost::asio::io_context io_context;
        ProxyServer server(io_context, 8888);  // Use the desired local port

        io_context.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
