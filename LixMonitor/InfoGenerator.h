#pragma once
#include <fcntl.h>
#include <string>
#include <array>
#include <fstream>
#include <unistd.h>
#include <regex>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <thread>
#include "msg.h"
#include <boost/algorithm/string/classification.hpp>
#include "LogInfo.h"

class InfoGenerator
{
public:
	InfoGenerator() {

	}

	std::string msg() const{
		return info;
	}

	void get_pinfo() {
		system("ps -aux >> cpu.info");
		std::ifstream in("cpu.info");
		std::getline(in, buf);
		while(std::getline(in,buf)) {
			pInfo.push_back(ProcInfo(buf));
		}
	}

	void generate() {
		while(true) {
			using namespace std::chrono_literals;
			system("rm cpu.info");
			pInfo.clear();
			get_pinfo();
			json_message msg;
			msg.add("type", 0);
			rapidjson::Value arr(rapidjson::kArrayType);
			for(auto &info:pInfo) {
				rapidjson::Value a(rapidjson::kArrayType);
				a.PushBack(rapidjson::Value(info.user.c_str(),msg.getAllocator()), msg.getAllocator());
				a.PushBack(rapidjson::Value(info.PID.c_str(), msg.getAllocator()), msg.getAllocator());
				a.PushBack(rapidjson::Value(info.cpu.c_str(), msg.getAllocator()), msg.getAllocator());
				a.PushBack(rapidjson::Value(info.mem.c_str(), msg.getAllocator()), msg.getAllocator());
				a.PushBack(rapidjson::Value(info.command.c_str(), msg.getAllocator()), msg.getAllocator());
				arr.PushBack(a,msg.getAllocator());
			}
			msg.add("ProcInfo", arr);
            auto io=getIOinfo();
            rapidjson::Value iopair(rapidjson::kArrayType);
            iopair.PushBack(rapidjson::Value(io.first.c_str(),msg.getAllocator()),msg.getAllocator());
            iopair.PushBack(rapidjson::Value(io.second.c_str(),msg.getAllocator()),msg.getAllocator());
            msg.add("IOInfo",iopair);
			auto net = getNetInfo();
			rapidjson::Value netpair(rapidjson::kArrayType);
			netpair.PushBack(rapidjson::Value(net.first.c_str(), msg.getAllocator()), msg.getAllocator());
			netpair.PushBack(rapidjson::Value(net.second.c_str(), msg.getAllocator()), msg.getAllocator());
			msg.add("NetInfo", netpair);
			info = msg.getString();
			std::this_thread::sleep_for(100ms);
		}
	}

    std::string getAppInfo(){
        system("rm app.info");
        system("(dpkg -l|grep ii) >> app.info");
        std::string buf;
        std::ifstream in("app.info");
        std::vector<AppInfo> applist;
        while(std::getline(in,buf)){
            applist.push_back(AppInfo(buf));
        }
        json_message msg;
        msg.add("type",1);
        rapidjson::Value arr(rapidjson::kArrayType);
        for(std::size_t i=0;i<100&&i<applist.size();i++){
            auto& app=applist[i];
            rapidjson::Value a(rapidjson::kArrayType);
            a.PushBack(rapidjson::Value(app.name,msg.getAllocator()),msg.getAllocator());
            a.PushBack(rapidjson::Value(app.version,msg.getAllocator()),msg.getAllocator());
            a.PushBack(rapidjson::Value(app.architecture,msg.getAllocator()),msg.getAllocator());
            a.PushBack(rapidjson::Value(app.description,msg.getAllocator()),msg.getAllocator());
            arr.PushBack(a,msg.getAllocator());
        }
        msg.add("AppInfo",arr);
        return msg.getString();
    }

    std::pair<std::string,std::string> getIOinfo(){
        system("rm io.info");
        system("(iostat -x|grep vda)>>io.info");
        std::string s;
        std::ifstream in("io.info");
        std::getline(in,s);
        std::vector<std::string> result;
        split_regex(result, s,boost::regex(" +"));
        return std::make_pair(result[5],result[6]);
    }

	std::pair<std::string,std::string> getNetInfo()
	{
		system("rm net.info");
		system("(ifstat 1 1) >> net.info");
		std::string s;
		std::ifstream in("net.info");
		std::getline(in, s);
		std::getline(in, s);
		std::getline(in, s);
		std::vector < std::string > result;
		split_regex(result, s, boost::regex(" +"));
		return std::make_pair(result[1], result[2]);
	}

	void start() {
		std::thread t(std::bind(&InfoGenerator::generate,this));
		t.detach();
	}

private:
	struct ProcInfo
	{
		ProcInfo(const std::string& s) {
			std::vector<std::string> result;
			split_regex(result, s, boost::regex(" +"));
			user = result[0];
			PID = result[1];
			cpu = result[2];
			mem = result[3];
			for(std::size_t i=10;i<result.size();i++) {
				command += result[i]+" ";
			}
		}

		std::string user;
		std::string PID;
		std::string cpu;
		std::string mem;
		std::string command;
	};

    struct AppInfo
    {
        AppInfo(const std::string& s){
            std::vector<std::string> result;
            split_regex(result,s,boost::regex(" +"));
            name = result[1];
            version=result[2];
            architecture=result[3];
            for(std::size_t i=4;i<result.size();i++){
                description+=result[i]+" ";
            }
        }

        std::string name;
        std::string version;
        std::string architecture;
        std::string description;
    };

	std::vector<ProcInfo> pInfo;
	std::string buf;
	std::string info;
};
