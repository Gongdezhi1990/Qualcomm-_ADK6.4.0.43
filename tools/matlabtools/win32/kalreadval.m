function [ val ] = kalreadval ( name, varargin )
%KALREADVAL read variables/registers from Kalimba memory.
%   Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd 
%   
%   [VALUE] = KALREADVAL( NAME, [OFFSET], [FORMAT], [PM] ) reads one or
%   values from Kalimba.
%
%   NAME may be:
%    * The name of a variable or constant to read.
%    * A single numeric value or array. Each value is an address you wish
%      to read from.
%    * A register name. Interesting registers include the performance counter registers:
%      RUNCLKS represents the number of clock samples completed.
%      RUNINSTRS represents the number of actual instructions completed.
%      NUMSTALLS is a count of the number of stalls that have occurred.
%      KALREADVAL can be used to read these counters, whilst
%      KALWRITEVAL is used to zero them.
%
%
%   OFFSET specifies an offset from the address specified is NAME. If NAME
%   NAME was the name of a variable or constant, the address corresponding
%   to NAME is looked up and then the OFFSET is applied. When NAME is a symbol,
%   and an offset is used, only a single word is returned, regardless of the size 
%   of the block of memory referred to by NAME. For example, if NAME logically refers to
%   a structure, and OFFSET is a field offset, only the first word of that field will
%   be returned. Keep this in mind when dealing with multiple-word fields.
%
%   OFFSET may be supplied as a string, in which case it is assumed to be a 
%   constant symbol whose value is to be used as the offset. If NAME is a
%   register, or an array of locations, OFFSET is not supported.
%
%
%   FORMAT specifies how the data should be formatted.
%   Available formats are:
%     'FRAC'  - decimal fraction, range [-1.0, 1.0)
%     'INT'   - signed integer
%     'INT16' - signed 16-bit integer 
%     'INT24' - signed 24-bit integer 
%     'UINT'  - unsigned decimal integer
%     'HEX'   - unsigned hexadecimal
%     'ASCII' - string
%
%   INT16 and INT24 are used to interpret data that is wider than 16 or 24
%   bits as though it was 16 or 24 bits. For example if a 24-bit value is
%   0x0089ab, when interpreted as a 24-bit number it is positive. When
%   interpreted as a 16-bit number it is negative. This is typically useful
%   when reading signed data that has not been sign extended into a wider
%   bit width.
%
%   The default format, used if FORMAT is not specified, is INT. 
%   If you wish to change the default, pass one of the following for FORMAT:
%     'MODEFRAC'  => FRAC
%     'MODEINT'   => INT
%     'MODEINT16' => INT16
%     'MODEINT24' => INT24
%     'MODEUINT'  => UINT
%     'MODEHEX'   => HEX
%     'MODEASCII' => ASCII
%
%
%   'PM' specifies that the value should be read from the PM address space.
%
%
%   NOTE: C style numbering is used rather than Matlab style, i.e. the first
%   element is at OFFSET 0, not 1.
%
%   NOTE: These Matlab tools do not stop the Kalimba. If you are reading from
%   an array, ensure the processor has been stopped or values may change
%   as you read them, leading to erroneous results. Also if any
%   Kalimba MMU accesses are being performed reading from data memory bank
%   two will cause problems.
%
%   See also kalloadsym, kalwriteval, kalrunning.
    
persistent krv

% If called with no inputs, display the help information.
if nargin == 0
    help kalreadval
    return
end

% Get the DSP architecture number up-front. Asserts we have a connection too.
kalArch  = kalprocessor('KAL_ARCH');
archInfo = kaldspinfo.archInfoFromArchNumber(kalArch);
ka       = kxs.instance();

if isempty(krv)
    % Set the default options
    krv.numFormat = 'INT';
end

% Check the extra options.
% offset  = An offset to apply to the address before reading
% bankSel = where, i.e. DM, PM or registers
% parseExtraMemArgs is generic, but accepts formats to disallow, in our case 'asciipack'.
[offset, bankSel, numFormat, modeChanged] = kalmemutils.parseExtraMemArgs(varargin, {'asciipack'});
if strcmp(numFormat, '')
    % No format supplied. Use saved one.
    numFormat = krv.numFormat;
elseif modeChanged
    % Mode changed. Update saved one. numFormat must have been supplied.
    krv.numFormat = numFormat;
end

[inputType, addr] = kalmemutils.parseNameArg(name, offset, bankSel, archInfo);
% If we discovered we're reading a register, then correct bankSel.
if inputType == kalmemutils.INPUT_TYPE_REGISTER
    bankSel = kalmemutils.READ_SOURCE_REGISTER;
end

% Actually read the data from the chip
switch inputType
    case {kalmemutils.INPUT_TYPE_ADDRESS, kalmemutils.INPUT_TYPE_SYMBOL}
        if bankSel == kalmemutils.READ_SOURCE_PM
            addr_per_word = kaldspinfo.addressesPerWordPM(archInfo);
            val        = ka.read_pm(addr, addr_per_word);
            data_width = archInfo.pm_data_width;
        else
            addr_per_word = kaldspinfo.addressesPerWordDM(archInfo);
            val        = ka.read_dm(addr, addr_per_word);
            data_width = archInfo.dm_data_width;
        end

    case kalmemutils.INPUT_TYPE_REGISTER
        val = ka.read_register(addr);
        data_width = ka.register_width(addr);
end

% Convert the data to the desired output format.
switch numFormat
    case 'FRAC'
        val = -2*(val >= 2^(data_width-1)) + (val * 2^-(data_width-1));
        val = val(:);

    case 'INT'
        val = -2^(data_width)*(val >= 2^(data_width-1)) + val;
        val = val(:);

    case 'INT16'
        % Only consider the bottom 16 bits.
        val = bitand(val, hex2dec('ffff'));
        % Treat the bottom 16 bits as a signed 16-bit integer.
        val = -2^16*(val >= 2^15) + val;
        val = val(:);
        
    case 'INT24'
        val = bitand(val, hex2dec('ffffff'));
        val = -2^24*(val >= 2^23) + val;
        val = val(:);
        
    case 'UINT'
        val = val(:);

    case 'HEX'
        if nargout ~= 0
            val = dec2hex(val);
        else
            val = val(:);
        end

    case 'ASCII'
        val = char(val);
end

% Pretty print to stdout if caller is not assigning result to a variable
if nargout == 0
    for i = 1:length(addr)
        switch upper(numFormat)
            case 'HEX'
                switch bankSel
                    case {kalmemutils.READ_SOURCE_PM, kalmemutils.READ_SOURCE_DM}
                        fprintf('    Value at location %5d\t(0x%0*X)\tis:  0x%08x\n', addr(i), archInfo.address_width/4, addr(i), val(i))
                    case kalmemutils.READ_SOURCE_REGISTER
                        fprintf('    Value of register %8s is:  0x%08x\n', name, val(i))
                end

            case 'ASCII'
                switch bankSel
                    case kalmemutils.READ_SOURCE_REGISTER
                        fprintf('    Value of register %8s is:  %s\n', name, val(i))
                    otherwise   % DM & PM
                        fprintf('    Value at location %5d\t(0x%0*X)\tis:  %s\n', addr(i), archInfo.address_width/4, addr(i), val(i))
                end

            otherwise
                switch bankSel
                    case kalmemutils.READ_SOURCE_REGISTER
                        fprintf('    Value of register %8s is:  %d\n', name, val(i))
                    otherwise   % DM & PM
                        fprintf('    Value at location %5d\t(0x%0*X)\tis:  %d\n', addr(i), archInfo.address_width/4, addr(i), val(i))
                end
        end
    end
    
    fprintf('\n')
    clear val
end
