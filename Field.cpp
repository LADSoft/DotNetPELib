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
#include <iomanip>
namespace DotNetPELib
{
    void Field::AddEnumValue(longlong Value, ValueSize Size)
    {
        if (mode_ == None)
        {
            mode_ = Enum;
            enumValue_ = Value;
            size_ = Size;
        }
    }
    void Field::AddInitializer(Byte *bytes, int len)
    {
        if (mode_ == None)
        {
            mode_ = Bytes;
            byteValue_ = bytes;
            byteLength_ = len;
        }
    }
    bool Field::ILSrcDumpTypeName(PELib &peLib, Field::ValueSize size)
    {
        switch (size)
        {
        case Field::i8:
            peLib.Out() << " int8";
            break;
        case Field::i16:
            peLib.Out() << " int16";
            break;
        case Field::i32:
        default:
            peLib.Out() << " int32";
            break;
        case Field::i64:
            peLib.Out() << " int64";
            break;
        }
        return true;
    }
    bool Field::ILSrcDump(PELib &peLib) const
    {
        peLib.Out() << ".field";
        flags_.ILSrcDumpBeforeFlags(peLib);
        flags_.ILSrcDumpAfterFlags(peLib);
        if (FieldType()->GetBasicType() == Type::cls)
            if (FieldType()->GetClass()->Flags().Flags() & Qualifiers::Value)
                peLib.Out() << " valuetype";
            else
                peLib.Out() << " class";
        peLib.Out() << " ";
        type_->ILSrcDump(peLib);
        peLib.Out() << " '" << name_ << "'";
        switch (mode_)
        {
        case None:
            break;
        case Enum:
            peLib.Out() << " = ";
            ILSrcDumpTypeName(peLib, size_);
            peLib.Out() << "(" << (int)enumValue_ << ")";

            break;
        case Bytes:
            if (byteValue_ && byteLength_)
            {
                peLib.Out() << " at $" << name_ << std::endl;
                peLib.Out() << ".data cil $" << name_ << " = bytearray (" << std::endl << std::hex;
                int i;
                for (i = 0; i < byteLength_; i++)
                {
                    peLib.Out() << std::setw(2) << std::setfill('0') << (int)byteValue_[i] << " ";
                    if (i % 8 == 7 && i != byteLength_ - 1)
                        peLib.Out() << std::endl << "\t";
                }
                peLib.Out() << ")" << std::dec;
            }
            break;
        }
        peLib.Out() << std::endl;
        return true;
    }
    bool Field::PEDump(PELib &peLib)
    {
        size_t sz;
        Byte *sig = SignatureGenerator::FieldSig(this, sz);
        size_t sigindex = peLib.PEOut().HashBlob(sig, sz);
        size_t nameindex = peIndex_ = peLib.PEOut().HashString(Name());
        int peflags = 0;
        if (flags_.Flags() & Qualifiers::Public)
            peflags |= FieldTableEntry::Public;
        else if (flags_.Flags() & Qualifiers::Private)
            peflags |= FieldTableEntry::Private;

        if (flags_.Flags() & Qualifiers::Static)
            peflags |= FieldTableEntry::Static;
        if (flags_.Flags() & Qualifiers::Literal)
            peflags |= FieldTableEntry::Literal;
        switch (mode_)
        {
        case Enum:
            peflags |= FieldTableEntry::HasDefault; // in the blob;
            break;
        case Bytes:
            if (byteValue_ && byteLength_)
            {
                peflags |= FieldTableEntry::HasFieldRVA; // in separate memory
            }
            break;
        }
        TableEntryBase *table = new FieldTableEntry(peflags, nameindex, sigindex);
        peIndex_ = peLib.PEOut().AddTableEntry(table);
        delete[] sig;

        Byte buf[8];
        *(longlong *)(buf) = enumValue_;
        int type;
        switch (mode_)
        {
        case Enum:
        {
            switch (size_)
            {
            case Field::i8:
                sz = 1;
                type = ELEMENT_TYPE_I1;
                break;
            case Field::i16:
                sz = 2;
                type = ELEMENT_TYPE_I2;
                break;
            case Field::i32:
            default:
                sz = 4;
                type = ELEMENT_TYPE_I4;
                break;
            case Field::i64:
                sz = 8;
                type = ELEMENT_TYPE_I8;
                break;
            }
            // this is NOT compressed like the sigs are...
            size_t valueIndex = peLib.PEOut().HashBlob(&buf[0], sz);
            Constant constant(Constant::FieldDef, peIndex_);
            table = new ConstantTableEntry(type, constant, valueIndex);
            peLib.PEOut().AddTableEntry(table);

        }
        case Bytes:
            if (byteValue_ && byteLength_)
            {
                size_t valueIndex = peLib.PEOut().RVABytes(byteValue_, byteLength_);
                table = new FieldRVATableEntry(valueIndex, peIndex_);
                peLib.PEOut().AddTableEntry(table);
            }
            break;
        }
        return true;
    }
}