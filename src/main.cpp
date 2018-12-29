/*  This file is part of compilescript.
 *  Copyright © 2018 tastytea <tastytea@tastytea.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if __cplusplus >= 201703L
    #include <filesystem>
#else
    #include <experimental/filesystem>
#endif
#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <libconfig.h++>
#include <basedir.h>
#include "xdgcfg.hpp"
#include "version.hpp"

#if __cplusplus >= 201703L
    namespace fs = std::filesystem;
#else
    namespace fs = std::experimental::filesystem;
#endif
using std::cerr;
using std::endl;

string compiler = "g++";
fs::path cache_dir;

void read_settings()
{
    bool need_save = false;
    xdgcfg config("compilescript.cfg");
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
        cache_dir /= "compilescript";
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

    if (argc <= 1)
    {
        cerr << "usage: " << argv[0] << " file [arguments]\n";
        return 1;
    }

    const fs::path original(argv[1]);
    const fs::path source = cache_dir / original.filename();
    const fs::path binary = (source.string() + ".bin");
    string compiler_arguments;

    if (!fs::exists(binary) ||
        fs::last_write_time(original) > fs::last_write_time(binary))
    {
        std::ifstream in(original);
        if (in.is_open())
        {
            std::ofstream out(source);
            if (out.is_open())
            {
                string buf;
                std::getline(in, buf);  // Shebang
                std::getline(in, buf);
                if (buf.substr(0, 16).compare("//compilescript:") == 0)
                {
                    compiler_arguments = buf.substr(16);
                }
                else
                {
                    out << buf << endl;
                }
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
                std::exit(1);
            }
        }
        else
        {
            cerr << "ERROR: Could not open file: " << original << endl;
            std::exit(1);
        }

        std::system((compiler + " " + source.string() + " " + compiler_arguments
                     + " -o " + binary.string()).c_str());
    }

    execv(binary.c_str(), &argv[1]);

    return 0;
}
