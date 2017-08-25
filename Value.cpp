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
    bool Value::ILSrcDump(PELib &peLib) const
    {
        // used for types
        type_->ILSrcDump(peLib);
        return true;
    }
    void Value::ObjOut(PELib &peLib, int pass) const
    {
        type_->ObjOut(peLib, pass);
    }
    Value *Value::ObjIn(PELib &peLib, bool definition)
    {
    	switch (peLib.ObjBegin())
        {
            case 'B':
            case 't':
            {
                peLib.ObjBack();
                Type *type = Type::ObjIn(peLib);
                Value *rv = peLib.AllocateValue("", type);
                return rv;
            }
            case 'l':
                return Local::ObjIn(peLib, false);
            case 'p':
                return Param::ObjIn(peLib, false);
            case 'f':
                return FieldName::ObjIn(peLib, false);
            case 's':
                return MethodName::ObjIn(peLib, false);
            default:
                peLib.ObjError(oe_syntax);
                break;
        }
    }
    size_t Value::Render(PELib &peLib, int opcode, int operandType, Byte *result)
    {
        return type_->Render(peLib, result);
    }
    bool Local::ILSrcDump(PELib &peLib) const
    {
        peLib.Out() << "'" << name_ << "/" << index_ << "'";
        return true;
    }
    void Local::ObjOut(PELib &peLib, int pass) const
    {
        peLib.Out() <<  std::endl << "$lb" << peLib.FormatName(name_) << index_;
        if (pass != -1)
        {
            GetType()->ObjOut(peLib, pass);
        }
        peLib.Out() <<  std::endl << "$le";
    }
    Local *Local::ObjIn(PELib &peLib, bool definition)
    {
        std::string name = peLib.UnformatName();
        int index = peLib.ObjInt();
        Type *tp = nullptr;
        bool retry = false;
        try
        {
            // type is optional here
            tp = Type::ObjIn(peLib);
        }
        catch (ObjectError &)
        {
            retry = true;
        }
        if (peLib.ObjEnd(!retry) != 'l')
        {
            peLib.ObjError(oe_syntax);
        }
        Local *rv = peLib.AllocateLocal(name, tp);
        rv->Index(index);
        return rv;
    }
    size_t Local::Render(PELib &peLib, int opcode, int operandType, Byte *result)
    {
        int sz = 0;
        if (operandType == Instruction::o_index1)
        {
            *(Byte *)result = index_;
            sz = 1;
        }
        else if (operandType == Instruction::o_index2)
        {
            *(unsigned short *)result = index_;
            sz = 2;
        }
        return sz;
    }
    bool Param::ILSrcDump(PELib &peLib) const
    {
        peLib.Out() << "'" << name_ << "'";
        return true;
    }
    void Param::ObjOut(PELib &peLib, int pass) const
    {
        peLib.Out() <<  std::endl << "$pb" << peLib.FormatName(name_) << index_;
        if (pass != -1)
        {
            GetType()->ObjOut(peLib, pass);
        }
        peLib.Out() <<  std::endl << "$pe";
    }
    Param *Param::ObjIn(PELib &peLib, bool definition)
    {
        std::string name = peLib.UnformatName();
        int index = peLib.ObjInt();
        Type *tp = nullptr;
        if (definition)
        {
            tp = Type::ObjIn(peLib);
        }
        if (peLib.ObjEnd() != 'p')
        {
            peLib.ObjError(oe_syntax);
        }
        Param *rv = peLib.AllocateParam(name, tp);
        rv->Index(index);
        return rv;
    }
    size_t Param::Render(PELib &peLib, int opcode, int operandType, Byte *result)
    {
        int sz = 0;
        if (operandType == Instruction::o_index1)
        {
            *(Byte *)result = index_;
            sz = 1;
        }
        else if (operandType == Instruction::o_index2)
        {
            *(unsigned short *)result = index_;
            sz = 2;
        }
        return sz;
    }
    bool FieldName::ILSrcDump(PELib &peLib) const
    {
        if (field_->FieldType()->GetBasicType() == Type::cls)
            if (field_->FieldType()->GetClass()->Flags().Flags() & Qualifiers::Value)
                peLib.Out() << "valuetype ";
            else
                peLib.Out() << "class ";
        field_->FieldType()->ILSrcDump(peLib);
        peLib.Out() << " ";
        peLib.Out() << Qualifiers::GetName(field_->Name(), field_->GetContainer());
        return true;
    }
    void FieldName::ObjOut(PELib &peLib, int pass) const
    {
        field_->ObjOut(peLib, -1);
    }
    FieldName *FieldName::ObjIn(PELib &peLib, bool definition)
    {
        Field *fld = Field::ObjIn(peLib, false);
        FieldName *rv = peLib.AllocateFieldName(fld);
        return rv;
    }
    size_t FieldName::Render(PELib &peLib, int opcode, int operandType, Byte *result)
    {
        if (field_->GetContainer() && field_->GetContainer()->InAssemblyRef())
        {
            field_->PEDump(peLib);
            *(DWord *)result = field_->PEIndex() | (tMemberRef << 24);
        }
        else
        {
            *(DWord *)result = field_->PEIndex() | (tField << 24);
        }
        return 4;
    }
    MethodName::MethodName(MethodSignature *M) : signature_(M), Value("", nullptr)
    {
    }
    bool MethodName::ILSrcDump(PELib &peLib) const
    {
        signature_->ILSrcDump(peLib, false, false, false);
        return true;
    }
    void MethodName::ObjOut(PELib &peLib, int pass) const
    {
        signature_->ObjOut(peLib, -1);
    }
    MethodName *MethodName::ObjIn(PELib &peLib, bool defintion)
    {
        MethodSignature *sig = MethodSignature::ObjIn(peLib, nullptr, false);
        MethodName *rv = peLib.AllocateMethodName(sig);
        return rv;
    }
    size_t MethodName::Render(PELib &peLib, int opcode, int operandType, Byte *result)
    {
        if (opcode == Instruction::i_calli)
        {
            if (signature_->PEIndexType() == 0)
                signature_->PEDump(peLib, true);
            *(DWord *)result = signature_->PEIndexType() | (tStandaloneSig << 24);
        }
        else
        {
            if (signature_->PEIndex() == 0 && signature_->PEIndexCallSite() == 0)
                signature_->PEDump(peLib, false);
            if (signature_->PEIndex())
                *(DWord *)result = signature_->PEIndex() | (tMethodDef << 24);
            else
                *(DWord *)result = signature_->PEIndexCallSite() | (tMemberRef << 24);
        }
        return 4;
    }
}