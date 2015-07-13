#ifndef __IOSERVICEPOOL_H__
#define __IOSERVICEPOOL_H__

#include <boost/asio.hpp>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <mutex>
#include <condition_variable>

class io_service_pool
    : private boost::noncopyable
{
public:
    /// Construct the io_service pool.
    explicit io_service_pool(std::size_t pool_size);

    /// Run all io_service objects in the pool.
    void run();

    void wait_init()
    {
        std::unique_lock<std::mutex> lk(m_lock);
        m_init_ok.wait(lk, [this]{return m_is_init; });
    }

    /// Stop all io_service objects in the pool.
    void stop();

    /// Get an io_service to use.
    boost::asio::io_service& get_io_service();

private:
    typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
    typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;

    /// The pool of io_services.
    std::vector<io_service_ptr> io_services_;

    /// The work that keeps the io_services running.
    std::vector<work_ptr> work_;

    /// The next io_service to use for a connection.
    std::atomic<std::size_t> next_io_service_;

    std::condition_variable m_init_ok;
    std::mutex m_lock;
    bool m_is_init;
};
#endif