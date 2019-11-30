function kalwriteval ( name, val, varargin )
%KALWRITEVAL write values to memory/registers in Kalimba memory.
%   Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd 
%   
%   KALWRITEVAL( NAME, VALUE, [OFFSET], [FORMAT], [PM] ) writes one or more
%   values to Kalimba memory.
%
%   NAME may be:
%    * The name of a variable.
%    * A single numeric value or array of values. Each value is an address you wish
%      to write to.
%    * A register name. Interesting registers include the performance counter registers:
%      RUNCLKS represents the number of clock samples completed.
%      RUNINSTRS represents the number of actual instructions completed.
%      NUMSTALLS is a count of the number of stalls that have occurred.
%      KALREADVAL can be used to read these counters, whilst
%      KALWRITEVAL is used to zero them.
%
%
%   VALUE specifies the value(s) to write. The length of VALUE
%   should correspond to the number of words represented by NAME.
%
%
%   OFFSET specifies an offset from the address specified is NAME. If NAME
%   NAME was the name of a variable or constant, the address corresponding
%   to NAME is looked up and then the OFFSET is applied. 
%
%   OFFSET may be supplied as a string, in which case it is assumed to be a 
%   constant symbol whose value is to be used as the offset. In this case, 
%   a single word is written to the offset address. This mirrors the behaviour of
%   kalreadval. If NAME is a register, OFFSET is not supported.
%
%
%   FORMAT specifies how the data supplied in VALUE should be interpreted.
%   Available formats are:
%     'FRAC'      accepts decimal fractional values     -1.0 <= VALUE <= 1.0
%     'INT'       accepts decimal integer values  MIN_SIGNED <= VALUE <= MAX_SIGNED
%     'UINT'      accepts decimal unsigned integers        0 <= VALUE <= MAX_UNSIGNED
%     'HEX'       accepts hexadecimal values               0 <= VALUE <= hex(MAX_UNSIGNED)
%     'ASCII'     accepts strings and converts them to their ASCII values
%                 allocating one word per character.
%     'ASCIIPACK' accepts strings and converts them to their ASCII values
%                 packing three characters into one word.
%
%   The default format, used if FORMAT is not specified, is INT. 
%   If you wish to change the default, pass one of the following for FORMAT:
%     'MODEFRAC'  => FRAC
%     'MODEINT'   => INT
%     'MODEUINT'  => UINT
%     'MODEHEX'   => HEX
%     'MODEASCII' => ASCII
%     'MODEASCIIPACK' => ASCIIPACK
%
%
%   'PM' specifies that the value should be read from the PM address space.
%
%
%   NOTE: C style numbering is used rather than Matlab style, i.e. the first
%   element is at OFFSET 0, not 1.
%
%   NOTE: These Matlab tools do not stop the Kalimba. If you are writing to an
%   array, ensure the processor has been stopped or values may change as you
%   write them, leading to erroneous results. Also if any Kalimba MMU accesses are
%   being performed writing to data memory bank two will cause problems.
%
%   See also kalloadsym, kalreadval, kalrunning.

persistent krv

% If called with no inputs, display the help information.
if nargin == 0
    help kalwriteval
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
% parseExtraMemArgs is generic, but accepts formats to disallow, in our case 'int16' and 'int24'.
[offset, bankSel, numFormat, modeChanged] = kalmemutils.parseExtraMemArgs(varargin, {'int16', 'int24'});
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

% If we're going via a symbol name, then adjust the address range down
% to match the number of values passed. This allows e.g. writing a single word to a structure field,
% without having to write to all subsequent fields (as write_mems will reject mismatched address ranges
% and values to write). If enough values are provided to satisfy the address range as-is, then leave it untouched.
if (inputType == kalmemutils.INPUT_TYPE_SYMBOL) && (length(addr) > length(val))
    addr = addr(1:length(val));
end

% Get some useful numbers for the calculations below.
if bankSel == kalmemutils.READ_SOURCE_PM
    dataWidth = archInfo.pm_data_width;
elseif bankSel == kalmemutils.READ_SOURCE_DM
    dataWidth = archInfo.dm_data_width;
else
    dataWidth = ka.register_width(addr);
end

negShift = 2^(dataWidth);

% Convert the input number to the required format.
switch numFormat
    case 'FRAC'
        % check the number is in the correct range
        if or( (max(val) > 1), (min(val) < -1) )
            error(['  The value entered is outside acceptable limits.' ...
                   char(10) '  Acceptable values for ''frac'' are -1.0 <= VAL <= 1.0'])
        end
        
        if any(abs(val) < 2^(-dataWidth))
            warning('Fractional representation: one or more values will underflow to zero')
        end

        fracOne = 2^(dataWidth-1);

        % 1.0 is a special case, and is represented as, for example, 0x7FFFFF on 24-bit
        % architectures.
        val = val - (val == 1) / fracOne;
        
        val = round(fracOne * val);
        
        % In C, we'd just cast to unsigned and mask off the upper bits.
        val = (negShift * (val < 0)) + val;

    case 'INT'
        % Calculate the maximum and minimum values
        maxInt  =  2^(dataWidth-1) - 1;
        minInt  = -2^(dataWidth-1);

        % check the number is in the correct range
        if or( (max(val) > maxInt), (min(val) < minInt) )
            error(['  The value you have entered is outside acceptable '...
                   'limits.' char(10) '  Acceptable range for this ''int'' value is ' ...
                   int2str(minInt) ' <= VAL <= ' int2str(maxInt)])
        end

        % convert to the decimal representation of signed integers
        val = negShift*(val < 0) + val;

    case 'UINT'
        % Calculate the maximum and minimum values
        maxUint = 2^dataWidth - 1;
        minUint = 0;

        % check the number is in the correct range
        if or( (max(val) > maxUint), (min(val) < minUint) )
            error(['  The value you have entered is outside acceptable '...
                   'limits.' char(10) '  Acceptable range for this ''uint'' value is ' ...
                   int2str(minUint) ' <= VAL <= ' int2str(maxUint)])
        end
        
        % Nothing to do for this format
        
    case 'HEX'
        % Calculate the maximum and minimum values
        maxHex = 2^dataWidth - 1;
        minHex = 0;

        % Convert the number to decimal; this is all we need
        val = hex2dec(val);

        % check the number is in the correct range
        if or( (max(val) > maxHex), (min(val) < minHex) )
            error(['  The value you have entered is outside acceptable '...
                   'limits.' char(10) '  Acceptable range for this ''hex'' value is ' ...
                   int2str(minHex) ' <= VAL <= ' int2str(maxHex)])
        end

    case 'ASCII'
        % convert the string to a number
        val = val - 0;
        % pad with zeros (note that this produces a column vector, but that's OK).
        val = [val(:); zeros(length(addr) - length(val), 1)];

    case 'ASCIIPACK'
        % convert the string into numbers
        val = val - 0;
        % make it into a column
        val = val(:);
        % find out how many words we need
        octetsPerWord = dataWidth / 8;
        paddingRequired = mod(-length(val), octetsPerWord);
        numOctets = length(val) + paddingRequired;
        % pad with zeros
        val = [val; zeros(paddingRequired, 1)];
        % make up the new words
        % array indexing => start : step : stop
        if dataWidth == 32
            val = [val(1:4:numOctets)*2^24 + val(2:4:numOctets)*2^16 + val(3:4:numOctets)*2^8 + val(4:4:numOctets)];
        elseif dataWidth == 24
            val = [val(1:3:numOctets)*2^16 + val(2:3:numOctets)*2^8 + val(3:3:numOctets)];
        elseif dataWidth == 16
            val = [val(1:2:numOctets)*2^8 + val(2:2:numOctets)];
        else
            error('Unsupported data width for ASCIIPACK format specifier')
        end
end

% Check the data supplied is the same size as the destination. At this point this
% should always be true. See adjustment above, after the parseNameArg call, for details.
if (length(addr) ~= length(val))
    error(['The size of the data vector is different to the size of the destination'])
end

% Finally do the write.
switch inputType
    case {kalmemutils.INPUT_TYPE_ADDRESS, kalmemutils.INPUT_TYPE_SYMBOL}
        if bankSel == kalmemutils.READ_SOURCE_PM
            addr_per_word = kaldspinfo.addressesPerWordPM(archInfo);
            ka.write_pm(addr, val, addr_per_word);
        else
            addr_per_word = kaldspinfo.addressesPerWordDM(archInfo);
            ka.write_dm(addr, val, addr_per_word);
        end
    case kalmemutils.INPUT_TYPE_REGISTER
        ka.write_register(addr, val);
end
