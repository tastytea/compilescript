/*  Public Domain / CC-0
 *  Author: tastytea <tastytea@tastytea.de>
 */

#if __cplusplus >= 201703L
    #include <filesystem>
#else
    #include <experimental/filesystem>
#endif
#include <iostream>
#include <string>
#include <fstream>
#include <libconfig.h++>
#include <basedir.h>
#include <cstdlib>
#include <unistd.h>
#include "xdgcfg.hpp"
#include "version.hpp"

#if __cplusplus >= 201703L
    namespace fs = std::filesystem;
#else
    namespace fs = std::experimental::filesystem;
#endif
using std::cout;
using std::cerr;
using std::endl;

bool need_save = false;
string compiler = "g++";
fs::path cache_dir;

void read_settings()
{
    xdgcfg config("cppscript.cfg");
    if (config.read() != 0)
    {
        config.write();
    }
    libconfig::Setting &cfg = config.get_cfg().getRoot();

    if (cfg.exists("compiler"))
    {
        compiler = cfg["compiler"].c_str();
    }
    else
    {
        cfg.add("compiler", libconfig::Setting::TypeString) = compiler;
        need_save = true;
    }

    if (cfg.exists("cache_dir"))
    {
        cache_dir = cfg["cache_dir"].c_str();
    }
    else
    {
        xdgHandle xdg;
        xdgInitHandle(&xdg);
        cache_dir = xdgCacheHome(&xdg);
        cache_dir /= "cppscript";
        xdgWipeHandle(&xdg);
    }
    if (!fs::is_directory(cache_dir))
    {
        fs::create_directories(cache_dir);
    }

    if (need_save)
    {
        config.write();
    }
}

int main(int argc, char *argv[])
{
    read_settings();

    if (argc > 1)
    {
        const fs::path path(argv[1]);
        const fs::path binary = cache_dir / path.stem();
        const fs::path source = binary.string() + ".cpp";

        std::ifstream in(path);
        if (in.is_open())
        {
            std::ofstream out(source);
            if (out.is_open())
            {
                string buf;
                std::getline(in, buf);
                while (!in.eof())
                {
                    std::getline(in, buf);
                    out << buf << endl;
                }
                in.close();
                out.close();
            }
            else
            {
                cerr << "ERROR: Could not open file: " << source << endl;
            }
        }
        else
        {
            cerr << "ERROR: Could not open file: " << path << endl;
        }

        std::system((compiler + " " + source.string()
                     + " -o " + binary.string()).c_str());
        execv(binary.c_str(), argv);
    }
    return 0;
}
