#include "requests.h"

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = asio::ip::tcp;

// для одной сессии
// Генератор на основе random_device (версии 4) --> Получаем UUID
boost::uuids::random_generator gen;
boost::uuids::uuid id = gen();
// номер посылки
unsigned int num = 1;

bool HTTP::S3_PutJsonObject(std::string_view file_content)
{
    try
    {
        std::string object_name = boost::uuids::to_string(id) + "__" + std::to_string(num++) + ".json";

        asio::io_context ioc;
        tcp::resolver resolver(ioc);

        auto const results = resolver.resolve(this->host, this->port);
        beast::tcp_stream stream(ioc);
        stream.connect(results);

        http::request<http::string_body> req(http::verb::put, "/" + this->bucket_name + "/" + object_name, this->http_ver);
        if (this->auth)
        {
            req.set(beast::http::field::authorization, "Basic " + this->login_password_base64);
        }
        req.set(http::field::host, this->host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(beast::http::field::content_type, "application/json");
        req.set(http::field::content_length, std::to_string(file_content.length()));
        req.body() = file_content;
        req.prepare_payload();

        http::write(stream, req);

        beast::flat_buffer buffer;
        //http::response<http::string_body> res;
        http::response<http::dynamic_body> res;

        http::read(stream, buffer, res);

        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);
        if(ec && ec != beast::errc::not_connected)
        {
            throw beast::system_error{ec};
        }
        if(res.result() != beast::http::status::ok)
        {
            return false;
        }

        return true;
    }
    catch (const boost::system::system_error& e)
    {
        BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error("Ошибка boost::system.")));
    }
    catch(std::exception const& e)
    {
        BOOST_THROW_EXCEPTION(boost::enable_error_info(std::runtime_error("Стандартное исключение.")));
    }
}
