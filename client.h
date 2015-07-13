#pragma once
#include <json/json.h>
#include <memory>
#include "LogWrapper.h"
#include "error_code.h"
#include "client_manager.h"
#include "client_interface.h"

class client : public client_interface
{
public:
    client(boost::asio::io_service &ios, std::weak_ptr<client_manager> wp)
        : io_service_(ios)
        , wpcm_(wp)
        , timer_(io_service_, boost::posix_time::seconds(7200))
    {
        
    }
    ~client();

    ERROR_CODE init_user_info(int id)
    {
        start_timer();

        m_session_data.clear();
        //�����
        m_session_data["Value"] = rand();
        m_session_data["Name"] = "ZX";
        m_session_data["UsrID"] = id;

        Json::Value v;
        v["subObj"] = 100;

        m_session_data["sub1"] = v;

        id_ = id;

        return USER_NOT_EXIST;
    }

    ERROR_CODE do_some(Json::Value &root, Json::Value &respons)
    {
        start_timer();
        return TEST_ERROR_CODE;
    }

    //������ͳһ�ظ�����
    void get_session_data(Json::Value &respons)
    {
        //����һ�����ݣ�Ȼ������swap��respons��
        Json::Value copyed = m_session_data;
        for (auto it = copyed.begin(); it != copyed.end(); it++)
        {
            respons[it.name()].swap(*it);
        }
    }

    void clear_client()
    {
        auto sp(wpcm_.lock());
        if (sp)
        {
            sp->erase(id_, shared_from_this());
        }
    }

private:
    void start_timer()
    {
        //����ͻ�������2Сʱ��ҵ�����������ͻ���
        std::weak_ptr<client> wp(shared_Derived_from_this<client>());
        timer_.async_wait([wp](const boost::system::error_code& ec){
            if (ec != boost::asio::error::operation_aborted)
            {
                auto sp(wp.lock());
                if (sp)
                {
                    sp->clear_client();
                }
            }
        });
    }
    

private:
    Json::Value m_session_data;
    boost::asio::io_service &io_service_;
    std::weak_ptr<client_manager> wpcm_;
    boost::asio::deadline_timer timer_;
    int id_;
};

