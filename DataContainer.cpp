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
#include <typeinfo>
namespace DotNetPELib
{
    void DataContainer::Add(Field *field)
    {
        field->SetContainer(this);
        fields_.push_back(field);
    }
    size_t DataContainer::ParentNamespace() const
    {
        DataContainer *current = this->Parent();
        while (current && typeid(*current) != typeid(Namespace))
            current = current->Parent();
        if (current)
            return current->PEIndex();
        return 0;
    }
    size_t DataContainer::ParentClass() const
    {
        DataContainer *current = Parent();
        if (current && typeid(*current) == typeid(Class))
        {
            return current->PEIndex();
        }
        return 0;
    }
    bool DataContainer::ILSrcDump(PELib &peLib) const
    {
        for (std::list<Field *>::const_iterator it = fields_.begin(); it != fields_.end(); ++it)
            (*it)->ILSrcDump(peLib);
        for (std::list<CodeContainer *>::const_iterator it = methods_.begin(); it != methods_.end(); ++it)
            (*it)->ILSrcDump(peLib);
        for (std::list<DataContainer *>::const_iterator it = children_.begin(); it != children_.end(); ++it)
            (*it)->ILSrcDump(peLib);
        return true;
    }
    bool DataContainer::PEDump(PELib &peLib)
    {
        for (auto field : fields_)
            field->PEDump(peLib);
        for (auto method : methods_)
            method->PEDump(peLib);
        for (auto child : children_)
            child->PEDump(peLib);
        return true;
    }
    void DataContainer::Number(int &n)
    {
        if (typeid(*this) != typeid(Namespace))
            peIndex_ = n++;
        for (auto child : children_)
            child->Number(n);
    }
    void DataContainer::Compile(PELib &peLib)
    {
        for (auto method : methods_)
            method->Compile(peLib);
        for (auto child : children_)
            child->Compile(peLib);
    }
    void DataContainer::BaseTypes(int &types) const
    {
        for (auto method : methods_)
            method->BaseTypes(types);
        for (auto child : children_)
            child->BaseTypes(types);
        if (typeid(*this) != typeid(PELib))
            if (typeid(*this) == typeid(Enum))
                types |= basetypeEnum;
            else if (typeid(*this) != typeid(Namespace))
                if (flags_.Flags() & Qualifiers::Value)
                    types |= basetypeValue;
                else
                    types |= basetypeObject;
    }
}