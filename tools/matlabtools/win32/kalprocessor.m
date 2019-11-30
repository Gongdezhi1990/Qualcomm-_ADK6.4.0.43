function [ varargout ] = kalprocessor( varargin )
%KALPROCESSOR returns or sets the chip name and architecture information.
% Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd 
% 
%   [ NAME ] = KALPROCESSOR returns the name of the connected chip. If this
%   is not known, it will query the chip to find out.
%
%   [ ARCH ] = KALPROCESSOR ( 'KAL_ARCH' ) returns the architecture number
%   of the chip.
%
%   KALPROCESSOR( NAME ) sets the chip name to NAME. Supported chips
%   include:
%       BC3MM    - BC3-Multimedia parts
%       BC5MM    - BC5-Multimedia parts
%       BC5ROM   - BC5-ROM parts
%       BC5AMAP  - BC5-AMAP parts
%       CSR8670  - BC7-Multimedia
%       CSR8675
%       CSRA68100
%       COREDUMP - Read values from a pre-loaded core dump
%
%   The following special values for NAME are also supported:
%
%       FORCE    - Re-read the chip identity from the target.
%       UNKNOWN  - Forget the chip identity, but do not re-read it from the
%                  target immediately.
%
%   NOTE: if you load the symbol table with a particular chip attached, and then
%   switch to a different chip, you must ensure that the chip identity is reset,
%   either by reconnecting via 'kalspi', or using this function with the
%   'FORCE' parameter.
%
%   See also kalspi, kalloadsym, kalsymfind, kalreadval, kalwriteval, kalrunning.
%

% Initialise persistent state.
persistent kalProcInfo;
if isempty(kalProcInfo)
    kalProcInfo.chipName = '';
    kalProcInfo.arch = 0;
    kalProcInfo.init = 0;
end

ka = kxs.instance();

for i = 1:nargin
    
    % Assume for now that the parameter is a chip name...
    name = upper(varargin{i});
    
    arch = ka.get_arch_from_name(name);
    if arch > 0
        % Caller is explicitly setting the chip name.
        kalProcInfo.chipName = name;
        kalProcInfo.arch = arch;
        kalProcInfo.init = 1;
    else
        switch name
            
            case 'COREDUMP'
                kalProcInfo.chipName = name;
                kalProcInfo.arch = -1; % oh, the joy of magic numbers
                kalProcInfo.init = 1;

            case {'MUL0','MUL1','MUL2','MUL3','MUL4','MUL5','MUL6','MUL7','MUL8','MUL9'}
                % For emulator use.
                kalspi('SETMUL', name(end));

            case 'FORCE'
                % Clear the initialised flag so that the next call to kalprocessor() will re-read
                % from the chip.
                kalProcInfo.init = 0;

            case 'CSR_DOC'
                % print out the help on the extra options
                s = help('kalprocessor');
                fprintf('\n%s',s)
                fprintf('    KALPROCESSOR( MULTIPLEX ) specifies the SPI multiplex to use, supported\n')
                fprintf('    options include:\n')
                fprintf('        MUL0-9   - Select SPI multiplex 0-9\n\n')
                fprintf('    KALPROCESSOR( CHIPNAME, MULTIPLEX ) sets the chip name to CHIPNAME and\n')
                fprintf('    the SPI multiplex to MULTIPLEX.\n\n')
                return

            case 'UNKNOWN'
                kalProcInfo.init = 0;
                kalProcInfo.chipName = 'UNKNOWN';
                kalProcInfo.arch = 0;
                return
                
            case 'KAL_ARCH'
              	continue; % The output argument is set up below; nothing to do here.

            otherwise
                error('Unknown command or unsupported chip.')
        end
    end
end

% have we got the chip information? if not get it
if (kalProcInfo.init == 0)
    kalProcInfo.chipName = ka.get_chip_name();
    kalProcInfo.arch = ka.get_arch();
    kalProcInfo.init = 1;
end

% Return or pretty-print the processor info.
switch nargout
    case 0
        fprintf(' The chip name is: %s. The Kalimba architecture number is: %d\n', kalProcInfo.chipName, kalProcInfo.arch)
    case 1
        if nargin==1 && strcmpi(varargin{i}, 'KAL_ARCH')
            varargout{1} = kalProcInfo.arch;
        else
            varargout{1} = kalProcInfo.chipName;
        end
end

return
