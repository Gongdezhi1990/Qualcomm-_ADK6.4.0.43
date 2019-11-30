function [ out ] = kalmodname( progCount )
%KALMODNAME list the module containing the supplied program location.
% Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd 
% 
%   KALMODNAME( PROGCOUNT ) returns the module containing the line of code corresponding to
%   PROGCOUNT, where PROGCOUNT is a valid program counter value. Base 10 numbers and hexadecimal
%   strings for PROGCOUNT are accepted.
%
%   KALMODNAME( 'PC' ) if the input is the string 'PC', then the Kalimba program
%   counter is read and used as the input to KALMODNAME. This requires a chip connection.
%
%   [SYMB] = KALMODNAME( PROGCOUNT ) returns the data in the variable SYMB in the
%   following format:
%      SYMB.mod  = module name
%      SYMB.line = corresponding line number in the file of source code
%      SYMB.pc   = the value passed in PROGCOUNT (after conversion to a numerical location if PROGCOUNT was 'PC').
%      SYMB.file = name of the file of source code.
%
%   If a function KALMODPRINT is on the Matlab path, it will be used to print the
%   results. This allows users to supply their own routines to launch their
%   desired editor/viewer.
%
%   KALMODPRINT will be called with the following arguments:
%      KALMODPRINT( Module, LineNum, ProgramCounter, FileName )
%
%   See also kalloadsym, kalsymfind, kalreadval, kalwriteval, kalrunning.
%

symbs = kalvarprs('MODULE');

if ischar(progCount)
    if strcmpi(progCount, 'pc')
        progCount = kalreadval('pc', 'uint');
    else
        % Handle 0x prefix for hex strings. Matlab can't cope with such constructs.
        if length(progCount) > 2
            if strcmpi(progCount(1:2), '0x')
                progCount = progCount(3:end);
            end
        end
        
        % Try to convert to base 10. This throws if you pass it nonsense (unlike other conversion functions..).
        try
            progCount = hex2dec(progCount);
        catch
            progCount = NaN;
        end
    end
end
    
% Only accept a single integer for progCount (following any transformations done above).
if ~isscalar(progCount) || isnan(progCount) || ~(floor(progCount) == progCount)
    error('Invalid argument type for progCount. See help kalmodname');
end

% make a flag to check if we've found anything
found = 0;

for i = 1:size(symbs,1)
    if symbs{i,3} == progCount
        found = i;
        break;
    end
end

if found
    if (nargout)
        out.mod = symbs{found,1};
        out.line = symbs{found,2};
        out.pc = symbs{found,3};
        out.file = symbs{found,4};
    else
        if (exist('kalmodprint') == 2) % From the help: 'returns 2 if the argument is an M-file on MATLAB's search path'
            kalmodprint( symbs{found,:} );
        else
            fprintf('  Program location %d => Module: %s\n  In file: <a href="matlab:opentoline( ''%s'', %d )">%s at %d</a>\n', ...
                    progCount, symbs{found, 1}, symbs{found,4}, symbs{found,2}, symbs{found,4}, symbs{found,2} );
        end
    end
else
    if nargout
        out.mod = -1;
        out.line = -1;
        out.pc = progCount;
        out.file = -1;
    else
        fprintf('  The program location you supplied could not be located.\n')
    end
end