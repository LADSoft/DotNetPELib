/* Software License Agreement
 *
 *     Copyright(C) 1994-2019 David Lindauer, (LADSoft)
 *
 *     This file is part of the Orange C Compiler package.
 *
 *     The Orange C Compiler package is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     The Orange C Compiler package is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with Orange C.  If not, see <http://www.gnu.org/licenses/>.
 *
 *     contact information:
 *         email: TouchStone222@runbox.com <David Lindauer>
 *
 */

#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <fstream>
#include <cstdlib>
#include <cstring>
#ifdef _WIN32
#    include <windows.h>
#endif
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#else
#    include <io.h>
extern "C" char* getcwd(char*, int);
#endif


#include "Utils.h"
#include "CmdFiles.h"

#include <sstream>
#include "version.h"
#include <iostream>

void (*Utils::cleanup)();

char* Utils::ShortName(const char* v)
{
    static char prog_name[260], *short_name, *extension;
    strcpy(prog_name, v);
    short_name = strrchr(prog_name, '\\');
    if (short_name == nullptr)
        short_name = strrchr(prog_name, '/');
    if (short_name == nullptr)
        short_name = strrchr(prog_name, ':');
    if (short_name)
        short_name++;
    else
        short_name = prog_name;

    extension = strrchr(short_name, '.');
    if (extension != nullptr)
        *extension = '\0';
    return short_name;
}
void Utils::banner(const char* progName)
{
    bool have_version = false;
    // no banner if they specify -!, this is also caught in the cmd switch module
    // so it is transparent to the proggy
#ifndef HAVE_UNISTD_H
    // handle /V switch
    for (int i = 1; i < __argc && __argv[i]; i++)
        if (__argv[i] && (__argv[i][0] == '/' || __argv[i][0] == '-'))
            if ((__argv[i][1] == 'V' && __argv[i][2] == 0) || !strcmp(__argv[i], "--version"))
            {
                have_version = true;
            }
    if (!have_version)
        for (int i = 1; i < __argc && __argv[i]; i++)
            if (__argv[i] && (__argv[i][0] == '/' || __argv[i][0] == '-'))
                if (__argv[i][1] == '!' || !strcmp(__argv[i], "--nologo"))
                    return;
#endif
    fprintf(stderr, "%s Version " STRING_VERSION " " COPYRIGHT "\n", ShortName(progName));
    if (have_version)
    {
        fprintf(stderr, "\nCompile date: " __DATE__ " time: " __TIME__ "\n");
        exit(0);
    }
}
void Utils::usage(const char* prog_name, const char* text)
{
    fprintf(stderr, "\nUsage: %s %s", ShortName(prog_name), text);
    exit(1);
}
char* Utils::GetModuleName()
{
    static char buf[256];
#if defined(_WIN32)
    GetModuleFileNameA(nullptr, buf, sizeof(buf));
#else
#    ifdef HAVE_UNISTD_H
    strcpy(buf, "unknown");
#    else
    strcpy(buf, __argv[0]);
#    endif
#endif
    return buf;
}
void Utils::SetEnvironmentToPathParent(const char* name)
{
    if (!getenv(name))
    {
        char buf[512];
        strcpy(buf, GetModuleName());
        char* p = strrchr(buf, '\\');
        if (p)
        {
            *p = 0;
            p = strrchr(buf, '\\');
            if (p)
            {
                *p = 0;
                char* buf1 = (char*)calloc(1, strlen(name) + strlen(buf) + 2);
                strcpy(buf1, name);
                strcat(buf1, "=");
                strcat(buf1, buf);
                putenv(buf1);
            }
        }
    }
}
std::string Utils::FullPath(const std::string& path, const std::string& name)
{
    std::fstream in(name, std::ios::in);
    if (!in.fail())
    {
        return name;
    }
    if (!path.empty())
    {
        std::string fpath = path;
        //        if (path.c_str()[strlen(path.c_str())-1] != '\\')
        if (path[path.length() - 1] != '\\')
        {
            fpath += "\\";
        }
        fpath += name;
        std::fstream in(fpath, std::ios::in);
        if (!in.fail())
        {
            return fpath;
        }
    }
    return name;
}
std::string Utils::AbsolutePath(const std::string& name)
{
    std::string rv = name;
#if defined(WIN32)
    if (name.size() >= 3 && (name[1] != ':' || name[2] != CmdFiles::DIR_SEP[0]))
    {
        char buf[MAX_PATH];
        GetCurrentDirectoryA(sizeof(buf), buf);
        if (name[1] == ':')
            rv = name.substr(0,2) + (buf +2) + CmdFiles::DIR_SEP + name.substr(2);
        else if (name[0] == CmdFiles::DIR_SEP[0])
            rv = std::string(buf).substr(0,2) + name;
        else
            rv = std::string(buf) + CmdFiles::DIR_SEP + name;
    }
#endif
    return rv;
}
std::string Utils::QualifiedFile(const char* path, const char* ext)
{
    char buf[260];
    strcpy(buf, path);
    char* p = strrchr(buf, '.');
    if (!p || p[-1] == '.' || p[1] == '\\')
        p = buf + strlen(buf);
    strcpy(p, ext);
    return std::string(buf);
}
char* Utils::FullQualify(char* string)
{
    static char buf[265];
    if (string[0] == '\\')
    {
        getcwd(buf, 265);
        strcpy(buf + 2, string);
        return buf;
    }
    else if (string[1] != ':')
    {
        char *p, *q = string;
        getcwd(buf, 265);
        p = buf + strlen(buf);
        if (!strncmp(q, ".\\", 2))
            q += 2;
        p--;
        while (!strncmp(q, "..\\", 3))
        {
            q += 3;
            while (p > buf && *p != '\\')
                p--;
            if (p > buf)
                p--;
        }
        p++;
        *p++ = '\\';
        strcpy(p, q);
        return buf;
    }
    return string;
}
std::string Utils::SearchForFile(const std::string& path, const std::string& name)
{
    FILE* fil = fopen(name.c_str(), "rb");
    if (fil)
    {
        fclose(fil);
        return name;
    }
    std::string fpath = path;
    while (!fpath.empty())
    {
        int npos = fpath.find_first_of(CmdFiles::PATH_SEP[0]);
        std::string current;
        if (npos == std::string::npos)
        {
            current = fpath;
            fpath = "";
        }
        else
        {
            current = fpath.substr(0, npos);
            fpath.erase(0, npos + 1);
        }
        if (current[current.size()] != CmdFiles::DIR_SEP[0])
        {
            current += CmdFiles::DIR_SEP;
        }
        current += name;
        fil = fopen(current.c_str(), "rb");
        if (fil)
        {
            fclose(fil);
            return current;
        }
    }
    return name;
}
std::string Utils::NumberToString(int num) { return std::to_string(num); }
std::string Utils::NumberToStringHex(int num)
{
    std::stringstream aa;
    aa << std::hex << num;
    return aa.str();
}
int Utils::StringToNumber(std::string str) { return std::stoi(str); }
int Utils::StringToNumberHex(std::string str) { return std::stoi(str, 0, 16); }

bool Utils::iequal(const std::string& a, const std::string& b, int sz)
{
    if (sz >= 0)
    {
        if (a.size() < sz || b.size() < sz)
            return false;
    }
    else
    {
        sz = a.size();
        if (sz != b.size())
            return false;
    }
    for (size_t i = 0; i < sz; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}