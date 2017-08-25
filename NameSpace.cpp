/*
    Software License Agreement (BSD License)

    Copyright (c) 2016, David Lindauer, (LADSoft).
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
#include "DotNetPELib.h"
#include "PEFile.h"
namespace DotNetPELib
{

    bool Namespace::ILSrcDump(PELib &peLib) const
    {

        peLib.Out() << ".namespace '" << name_ << "' {" << std::endl;
        DataContainer::ILSrcDump(peLib);
        peLib.Out() << "}" << std::endl;
        return true;
    }
    void Namespace::ObjOut(PELib &peLib, int pass) const
    {
        peLib.Out() << std::endl << "$nb" << peLib.FormatName(name_);
        DataContainer::ObjOut(peLib, pass);
        peLib.Out() << std::endl << "$ne";
    }
    Namespace *Namespace::ObjIn(PELib &peLib, bool definition)
    {
        // always a definition (never used as an operand)
        std::string name = peLib.UnformatName();
        Namespace *temp, *rv = nullptr;
        temp = (Namespace *)peLib.GetContainer()->FindContainer(name);
        if (temp && typeid(*temp) != typeid(Namespace))
            peLib.ObjError(oe_nonamespace);
        if (!temp)
            rv = temp = peLib.AllocateNamespace(name);
        ((DataContainer *)temp)->ObjIn(peLib);
        if (peLib.ObjEnd() != 'n')
            peLib.ObjError(oe_syntax);
        return rv;
    }
    std::string Namespace::ReverseName(DataContainer *child)
    {
        std::string rv;
        if (child->Parent())
        {
            if (child->Parent()->Parent())
                rv = ReverseName(child->Parent()) + ".";
            rv += child->Name();
        }
        return rv;
    }
    bool Namespace::PEDump(PELib &peLib)
    {
        if (!InAssemblyRef() || !PEIndex())
        {
            std::string fullName = ReverseName(this);
            peIndex_ = peLib.PEOut().HashString(fullName);
        }
        if (!InAssemblyRef())
            DataContainer::PEDump(peLib);
        return true;
    }
}