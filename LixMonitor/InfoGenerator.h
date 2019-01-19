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
			system("rm *.info");
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
			info = msg.getString();
			std::this_thread::sleep_for(100ms);
		}
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
			for(int i=10;i<result.size();i++) {
				command += result[i]+" ";
			}
		}

		std::string user;
		std::string PID;
		std::string cpu;
		std::string mem;
		std::string command;
	};

	std::vector<ProcInfo> pInfo;
	std::string buf;
	std::string info;
};
