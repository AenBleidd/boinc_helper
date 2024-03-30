// boinc_helper.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <array>
#include <regex>
#include <thread>
#include <chrono>

std::string run_command(const std::string& command)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"), _pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

int main()
{
    while (true)
    {
        std::string output = run_command("boinccmd.exe --get_tasks");
        bool gpu_tasks_found = false;
        const std::string delimiter = "\n";
        size_t pos = 0;
        std::string token;
        while ((pos = output.find(delimiter)) != std::string::npos) {
            token = output.substr(0, pos);

            // parse the token using regex
            const std::regex re(".?resources:.*(GPU).?");
            std::smatch match;
            if (std::regex_search(token, match, re)) {
                std::cout << std::chrono::system_clock::now() << " GPU tasks found" << std::endl;
                gpu_tasks_found = true;
                break;
            }

            output.erase(0, pos + delimiter.length());
        }

        if (!gpu_tasks_found) {
            std::cout << std::chrono::system_clock::now() << " No GPU tasks found, updating..." << std::endl;
            run_command("boinccmd.exe --project https://einstein.phys.uwm.edu/ update");
        }

        std::this_thread::sleep_for(std::chrono::seconds(60));        
    }
    return 0;
}
