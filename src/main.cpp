/*  This file is part of compilescript.
 *  Copyright © 2018, 2019 tastytea <tastytea@tastytea.de>
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

#include "version.hpp"
#include "xdgcfg.hpp"
#include <libconfig.h++>
#include <basedir.h>
#include <unistd.h>
#include <chrono>
#include <cstdlib>
#include <exception>
#include <experimental/filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::experimental::filesystem;
using std::cout;
using std::cerr;
using std::endl;
using std::chrono::system_clock;
using std::chrono::hours;
using std::chrono::duration_cast;
using std::string;
using std::vector;

class Compilescript
{
private:
    string _compiler;
    fs::path _cache_dir;
    int _clean_after_hours;

public:
    Compilescript();

    void read_settings();
    void cleanup();
    bool compile(const string &filename, char *argv[]);
    void print_version();
};

Compilescript::Compilescript()
    : _compiler("g++ -x c++")
    , _clean_after_hours(30 * 24)
{}


void Compilescript::read_settings()
{
    bool need_save = false;
    xdgcfg config("compilescript.cfg");
    config.read();
    libconfig::Setting &cfg = config.get_cfg().getRoot();

    if (cfg.exists("compiler"))
    {
        _compiler = cfg["compiler"].c_str();
    }
    else
    {
        cfg.add("compiler", libconfig::Setting::TypeString) = _compiler;
        need_save = true;
    }

    if (cfg.exists("clean_after_hours"))
    {
        cfg.lookupValue("clean_after_hours", _clean_after_hours);
    }
    else
    {
        cfg.add("clean_after_hours",
                libconfig::Setting::TypeInt) = _clean_after_hours;
        need_save = true;
    }

    if (cfg.exists("cache_dir"))
    {
        _cache_dir = cfg["cache_dir"].c_str();
    }
    else
    {
        xdgHandle xdg;
        xdgInitHandle(&xdg);
        _cache_dir = xdgCacheHome(&xdg);
        _cache_dir /= "compilescript";
        xdgWipeHandle(&xdg);
    }
    if (!fs::is_directory(_cache_dir))
    {
        fs::create_directories(_cache_dir);
    }

    if (need_save)
    {
        config.write();
    }
}

void Compilescript::cleanup()
{
    for (const auto &entry : fs::recursive_directory_iterator(_cache_dir))
    {
        if (!fs::is_regular_file(entry))
        {
            continue;
        }

        const auto diff = system_clock::now() - fs::last_write_time(entry);
        if (duration_cast<hours>(diff).count() > _clean_after_hours)
        {
            fs::path current_path = entry.path();
            std::error_code e;

            while (fs::remove(current_path, e))
            {
                current_path = current_path.parent_path();
                if (current_path == _cache_dir)
                {
                    break;
                }
            }
        }
    }
}

bool Compilescript::compile(const string &filename, char *argv[])
{
    const fs::path original = fs::canonical(filename);
    const fs::path source = _cache_dir / original;
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
                if (buf.substr(0, 17) == "// compilescript:")
                {
                    compiler_arguments = buf.substr(17);
                }
                else if (buf.substr(0, 16) == "//compilescript:")
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
                return false;
            }
        }
        else
        {
            cerr << "ERROR: Could not open file: " << original << endl;
            return false;
        }

        const string command = _compiler + " " + source.string() + " "
            + compiler_arguments + " -o " + binary.string();
        int ret = std::system(command.c_str()); // NOLINT Doesn't apply here.
        if (ret != 0)
        {
            cerr << "ERROR: Compilation failed.\n";
            return false;
        }
    }

    execvp(binary.c_str(), &argv[1]); // NOLINT We know that argv[1] exists.

    return true;
}

void Compilescript::print_version()
{
    cout << "compilescript " << global::version << '\n' <<
        "Copyright (C) 2018, 2019 tastytea <tastytea@tastytea.de>\n"
        "License GPLv3: GNU GPL version 3 "
        "<https://www.gnu.org/licenses/gpl-3.0.html>.\n"
        "This program comes with ABSOLUTELY NO WARRANTY. This is free software,"
        "\nand you are welcome to redistribute it under certain conditions.\n";
}

int main(int argc, char *argv[])
{
    const vector<string> args(argv, argv + argc);

    Compilescript App;
    App.read_settings();

    if (args.size() <= 1)
    {
        cerr << "usage: " << args[0]
             << " [file|--cleanup|--version] [arguments]\n";
        return 1;
    }
    if (args[1] == "--cleanup")
    {
        App.cleanup();
        return 0;
    }
    if (args[1] == "--version")
    {
        App.print_version();
        return 0;
    }

    try
    {
        App.compile(args[1], argv);
    }
    catch (const std::exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
