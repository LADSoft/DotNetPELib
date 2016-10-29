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
    bool Operand::isnanorinf() const
    {
        // little endian architectures only
        longlong check;
        *(double *)&check = floatValue_;
        if (check == 0)
            return false;
        // infinity or nan, or denormal... (exponent all ones or all zeros)
        check >>= 32;
        check &= 0x7ff00000;
        return check == 0x7ff00000 || check == 0;
    }
    bool Operand::ILSrcDump(PELib &peLib) const
    {
        switch (type_)
        {
        case t_none: // no operand, nothing to display
            break;
        case t_value:
            refValue_->ILSrcDump(peLib);
            break;
        case t_int:
            peLib.Out() << intValue_;
            break;
        case t_real:
            if (isnanorinf())
            {
                Byte buf[8];
                int sz1, i;
                if (sz_ == r4)
                {
                    sz1 = 4;
                    *(float *)buf = floatValue_;
                }
                else
                {
                    sz1 = 8;
                    *(double *)buf = floatValue_;
                }

                peLib.Out() << "(" << std::hex;
                for (i = 0; i < sz1; i++)
                {
                    peLib.Out() << std::setw(2) << std::setfill('0') << (int)buf[i] << " ";
                }
                peLib.Out() << ")" << std::dec;
            }
            else
            {
                peLib.Out() << floatValue_;
            }
            break;
        case t_string:
            peLib.Out() << "\"" << stringValue_ << "\"";
            break;
        case t_label:
            peLib.Out() << stringValue_;
            break;
        }
        return true;
    }
    size_t Operand::Render(PELib &peLib, int opcode, int operandType, Byte *result)
    {
        int sz = 0;
        switch (type_)
        {
        case t_none: // no operand, nothing to display
            break;
        case t_value:
            sz = refValue_->Render(peLib, opcode, operandType, result);
            break;
        case t_int:
            switch (operandType)
            {
            case Instruction::o_immed1:
                result[sz++] = intValue_;
                break;
            case Instruction::o_immed4:
                *(int *)(result) = intValue_;
                sz += 4;
                break;
            case Instruction::o_immed8:
                *(longlong *)result = intValue_;
                sz += 8;
                break;
            }
            break;
        case t_real:
            switch (operandType)
            {
            case Instruction::o_float4:
                *(float *)result = floatValue_;
                sz += 4;
                break;
            case Instruction::o_float8:
                *(double *)result = floatValue_;
                sz += 8;
                break;
            }
            break;
        case t_string:
        {
            wchar_t *buf = new wchar_t[stringValue_.size() + 1];
            for (int i = 0; i < stringValue_.size() + 1; i++)
                buf[i] = stringValue_.c_str()[i];
            size_t usIndex = peLib.PEOut().HashUS(buf);
            *(int *)(result) = usIndex | (0x70 << 24);
            sz += 4;
            delete[] buf;
            break;
        }
        }
        return sz;
    }
}