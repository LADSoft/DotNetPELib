/*
    Software License Agreement (BSD License)
    
    Copyright (c) 1997-2016, David Lindauer, (LADSoft).
    All rights reserved.
    
    Redistribution and use of this software in source and binary forms, 
    with or without modification, are permitted provided that the following 
    conditions are met:
    
    * Redistributions of source code must retain the above
      copyright notice, this list of conditions and the
      following disclaimer.
    
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the
      following disclaimer in the documentation and/or other
      materials provided with the distribution.
    
    * Neither the name of LADSoft nor the names of its
      contributors may be used to endorse or promote products
      derived from this software without specific prior
      written permission of LADSoft.
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
    OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    contact information:
        email: TouchStone222@runbox.com <David Lindauer>
*/
#ifndef CMDFILES_H
#define CMDFILES_H

#undef min
#undef max
#include <vector>
#include <string>

class CmdFiles
{
    typedef std::vector<std::string *> FileName;
    
    public:
        
        CmdFiles() { }
        CmdFiles(char **fileList, bool recurseSubdirs = false) { Add(fileList, recurseSubdirs); }
        CmdFiles(const std::string &name, bool recurseSubdirs = false) { Add(name, recurseSubdirs); }
        ~CmdFiles();
        
        bool Add(const std::string &name, bool recurseSubdirs = false);
        bool AddFromPath(const std::string &name, const std::string &path);
        bool Add(char **fileList, bool recurseSubdirs = false);
        
        typedef FileName::iterator FileNameIterator;

        size_t GetSize() const { return names.size(); }		
        FileNameIterator FileNameBegin () { return names.begin(); }
        FileNameIterator FileNameEnd () { return names.end(); }
        static const char *DIR_SEP;
        static const char *PATH_SEP;
    protected:		
        bool RecurseDirs(const std::string &path, const std::string &name, bool recurseDirs);
    private:
        FileName names;
        
};
#endif
