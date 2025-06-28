#include <iostream>
#include <string>
#include<sys/wait.h>
#include <filesystem>
#include <cstdlib>
#include <sstream>
#include <fstream> 
#include<vector>
#include<limits.h>
#include<unistd.h>
std::vector<std::string> tokenize(const std::string &input) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_single = false, in_double = false, escape = false;

    for (char c : input) {
        if (escape) {
            current += c;
            escape = false;
        } else if (c == '\'' && !in_double) {
            in_single = !in_single;
        } else if (c == '"' && !in_single) {
            in_double = !in_double;
        } else if (!in_single && !in_double && isspace(c)) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}
int main(int argc, char* argv[]) {
    // Flush after every std::cout / std:cerr
	std::cout << std::unitbuf;
  	std::cerr << std::unitbuf;

 	while(1){
		std::cout << "$ ";
  		std::string input;
  		std::getline(std::cin, input);
		std::string trimmed = input;
		std::string s = "";
		int i = 0;
		int n = input.size();
		while(i<n and input[i] != ' ' ){
			s+=input[i];
			i++;
		}
		if(s == "exit"){
			return 0;
		}
		else if(s == "echo"){
			std::vector<std::string> op;
			std::string temp = "";
			bool in_quotes = false;
			char quote_char = '\0';
			int i = 5;
			while (i < n) {
				char ch = input[i];
				if (!in_quotes && (ch == '"' || ch == '\'')) {
					in_quotes = true;
					quote_char = ch;
					i++;
					continue;
				}
				if (in_quotes && ch == quote_char) {
					in_quotes = false;
					i++;
					continue;
				}
				if (in_quotes) {
					if(quote_char == '\"' and ch == '\\'){
						i++;//next char
						temp+=input[i];
						i++;
						continue;
					}
					temp += ch;
					i++;
					continue;
				}
				if (!in_quotes && (ch == ' ' || ch == '\t')) {
					if (!temp.empty()) {
						op.push_back(temp);
						temp = "";
					}
					while (i < n && (input[i] == ' ' || input[i] == '\t')) i++;
					continue;
				}
				if((!in_quotes) and ch =='\\'){
					i++;
					temp +=input[i];
					i++;
					continue;
				}
				temp += ch;
				i++;
			}
			if (!temp.empty())
				op.push_back(temp);
			for (int j = 0; j < op.size(); ++j) {
				std::cout << op[j];
				if (j != op.size() - 1)
					std::cout << ' ';
			}
			std::cout << '\n';
		}
		else if(s == "type"){
			std::string type = "";
			i = 4;
			while(input[i] == ' ' and i<n)
			 	i++;
			type = input.substr(i);
			if(type == "exit"){
				std::cout<<"exit is a shell builtin\n";
			}else if(type == "echo"){
				std::cout<<"echo is a shell builtin\n";
			}else if(type == "type"){
				std::cout<<"type is a shell builtin\n";
			}else if(type == "pwd"){
				std::cout<<"pwd is a shell builtin\n";
			}
			else if(type == "cd"){
				std::cout<<"cd is a shell builtin\n";
			}
			else {
				std::string cmd = argv[0];
				const char* path_env = std::getenv("PATH");
				std::istringstream paths(path_env);
				std::string dir;
				bool f = false;
				while(std::getline(paths,dir,':')){
				    std::filesystem::path full = std::filesystem::path(dir)/type;
					auto st = std::filesystem::status(full);
					if (std::filesystem::exists(full))
       				{
						std::cout << type << " is " << std::filesystem::absolute(full).string() << "\n";
						f = true;
						break;
        			}
				}
				if(!f)
					std::cout << type << ": not found" << std::endl;
			}
		}
		else if(s == "pwd"){
			char pwd[PATH_MAX];	
			getcwd(pwd,sizeof(pwd));
			std::cout<<pwd<<'\n';
		}
		else if(s == "cd"){
			std::string direct = input.substr(3);
			if("~" == direct)
				direct = getenv("HOME");
			int res = chdir(direct.c_str());
			if(res == -1){
				std::cout<<"cd: "<<direct<<": "<<"No such file or directory\n";
				continue;
			}
		}
		else {
			std::vector<std::string> args = tokenize(input);
			if (args.empty()) continue;

			std::vector<char*> argv_exec;
			for (auto &s : args) argv_exec.push_back(&s[0]);
			argv_exec.push_back(nullptr);

			if (fork() == 0) {
				std::string cmd = args[0];
				if (cmd.find('/') != std::string::npos) {
					execv(cmd.c_str(), argv_exec.data());
				} 
				else {
					execvp(argv_exec[0], argv_exec.data());
					std::string local_cmd = "./" + cmd;
					if (std::filesystem::exists(local_cmd)) {
						execv(local_cmd.c_str(), argv_exec.data());
					}
					std::cout << cmd << ": command not found\n";
				}
				exit(1);
			} else {	
				int status;
				wait(&status);
			}
		}
   }
}