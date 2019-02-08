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
#include <chrono>
#include <system_error>
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
using std::chrono::system_clock;
using std::chrono::hours;
using std::chrono::duration_cast;

string compiler = "g++ -x c++";
fs::path cache_dir;
int clean_after_hours = 30 * 24;

bool read_settings()
{
    bool need_save = false;
    xdgcfg config("compilescript.cfg");
    config.read();
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

    if (cfg.exists("clean_after_hours"))
    {
        cfg.lookupValue("clean_after_hours", clean_after_hours);
    }
    else
    {
        cfg.add("clean_after_hours",
                libconfig::Setting::TypeInt) = clean_after_hours;
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

    return true;
}

void cleanup()
{
    for (const fs::directory_entry &entry
         : fs::recursive_directory_iterator(cache_dir))
    {
        if (fs::is_regular_file(entry))
        {
            auto diff = system_clock::now() - fs::last_write_time(entry);
            if (duration_cast<hours>(diff).count() > clean_after_hours)
            {
                fs::path current_path = entry.path();
                std::error_code e;

                while (fs::remove(current_path, e))
                {
                    current_path = current_path.parent_path();
                    if (current_path == cache_dir)
                    {
                        break;
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (!read_settings())
    {
        return 1;
    }

    if (argc <= 1)
    {
        cerr << "usage: " << argv[0] << " [file|--cleanup] [arguments]\n";
        return 1;
    }
    if (string(argv[1]) == "--cleanup")
    {
        cleanup();
        return 0;
    }

    const fs::path original = fs::canonical(argv[1]);
    const fs::path source = cache_dir / original;
    const fs::path binary = (source.string() + ".bin");
    string compiler_arguments;
    fs::create_directories(binary.parent_path());

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

                std::getline(in, buf);
                if (!(buf.substr(0, 2) == "#!"))
                {
                    out << buf << endl;
                }

                std::getline(in, buf);
                if (buf.substr(0, 16) == "//compilescript:")
                {
                    compiler_arguments = buf.substr(16);
                }
                else if ((buf.substr(0, 15) == "#compilescript:") ||
                         (buf.substr(0, 15) == ";compilescript:"))
                {
                    compiler_arguments = buf.substr(15);
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
                return 1;
            }
        }
        else
        {
            cerr << "ERROR: Could not open file: " << original << endl;
            return 1;
        }

        int ret = std::system((compiler + " " + source.string() + " "
                      + compiler_arguments + " -o " + binary.string()).c_str());
        if (ret != 0)
        {
            cerr << "ERROR: Compilation failed.\n";
            return 1;
        }
    }

    execv(binary.c_str(), &argv[1]);

    return 0;
}
