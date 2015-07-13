#pragma once

#include <iostream>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <json/json.h>
#include <boost/asio.hpp>
#include "request_respons.h"
#include "LogWrapper.h"
#include "client_interface.h"
#include "public_define.h"

class client_manager : public std::enable_shared_from_this<client_manager>
{
public:
    client_manager();
    ~client_manager();

    bool do_some(boost::asio::io_service &io_, request &request_, response &response_)
    {
        //Ҳ����Ҫ���ܣ���ѹ��request_.get_body(), request_.get_body() + request_.get_body_len()
        Json::Reader reader;
        Json::Value root;

        Json::Value &res = response_.getValue();

        res["errorcode"] = SUCCEEDED;

        do
        {
            if (!reader.parse(request_.get_body(), request_.get_body() + request_.get_body_len(), root))
            {
                LOG("request_.get_body() use Json::Reader::parse failed, contex is: %s \n",
                    std::string(request_.get_body(), request_.get_body_len()).c_str());
                res["errorcode"] = JSON_PARSER_ERROR;
                break;
            }

            std::string ClientVersion;
            std::string SceneId;
            int iUsrID = 0;

            try
            {
                ClientVersion = root["ClientVersion"].asString();
                SceneId = root["SceneId"].asString();
                iUsrID = root["UsrID"].asInt();
            }
            catch (const std::exception &e)
            {
                LOG("Json::Value has a exception: %s\n", e.what());
                LOG("Use jsoncpp failed, contex is: %s \n",
                    std::string(request_.get_body(), request_.get_body_len()).c_str());
                res["errorcode"] = JSON_GETVALUE_EXCEPTION;
                break;
            }

            //�������һ����½��Ϣ
            if (ClientVersion != "0.0.1")
            {
                process_logic(root, res);
            }
            else
            {
                process_login(io_, iUsrID, res);
            }

        } while (0);

        //ͳһ����ͨ��errorcode������errormsg
        res["errormsg"] = res["errorcode"].asString();

        return true;
    }

    void erase(int id, std::shared_ptr<client_interface> p)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        auto it = m_client.find(id);
        if (it != m_client.end())
            m_client.erase(it);
    }
private:

    /*
     * ����Ѿ���¼����ô�����µ�¼
     * ���û�е�¼���Ǿ͵�¼
     * Ȼ�󷵻ص�½�ɹ�
    */
    void process_login(boost::asio::io_service &ios, int value, Json::Value &respons);

    /*
     * ���û�е�¼������ʧ�ܡ�
     * �����¼�������߼�������client->do_some()
    */
    void process_logic(Json::Value &root, Json::Value &respons);
    

    bool insert(int id, std::shared_ptr<client_interface> &p)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        auto it = m_client.insert(make_pair(id, p));
        return it.second;
    }

private:
    ERROR_CODE find_client_use_id(int id, std::shared_ptr<client_interface> &sc);

    ERROR_CODE insert_client_use_id(boost::asio::io_service &ios, int id, std::shared_ptr<client_interface> &sc);

private:
    std::unordered_map < int, std::shared_ptr<client_interface> > m_client;
    std::mutex m_lock;
};

