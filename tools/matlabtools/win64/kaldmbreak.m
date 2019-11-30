function [breakInfo] = kaldmbreak(startAddress, endAddress, breakpointId, triggerOnRead, triggerOnWrite)
%KALDMBREAK sets the hardware data memory breakpoints on a Kalimba DSP.
% Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd 
% 
% KALDMBREAK( ) lists the currently-set DM breakpoints.
% breakInfo = KALDMBREAK ( ) returns the breakpoint information in an array of structs, rather than printing it. 
%
% KALDMBREAK( START_ADDRESS, END_ADDRESS, BREAKPOINT_ID, TRIGGER_ON_READ, TRIGGER_ON_WRITE ) 
% sets the hardware DM breakpoint specified by BREAKPOINT_ID to the specified address or addresses.
% 
% BREAKPOINT_ID must lie between 0 and (available hardware breakpoint count - 1).
% There are two hardware DM breakpoints (single address) on BC7, and two hardware DM breakpoints 
% supporting address ranges on newer chips.
%
% Passing -1 for START_ADDRESS will disable the specified breakpoint.
% If BREAKPOINT_ID is also -1, this will disable all hardware DM breakpoints.
%
% If the hardware supports address ranges for DM breakpoints (see above), then END_ADDRESS may be 
% greater than START_ADDRESS. Otherwise, END_ADDRESS must be equal to START_ADDRESS (or an error
% will be raised).
%
% The breakpoint can be configured to trigger on reads and/or on writes via the boolean parameters 
% TRIGGER_ON_READ and TRIGGER_ON_WRITE.
%
% If START_ADDRESS is a string it is treated as a hexadecimal value, unless it is ''help'', 
% which displays this message.

ka = kxs.instance();

if nargin == 0
    % Either display info about the breakpoints, or return it.
    if nargout ~= 0
        breakInfo = [];
    end
    
    numSet = 0;
    availableBreakpoints = ka.get_num_dm_breakpoints();
    for i = 0:availableBreakpoints - 1
        [startAddress, endAddress, triggerOnRead, triggerOnWrite] = ka.get_dm_breakpoint(i);
        enabled = (triggerOnRead || triggerOnWrite);
        
        if nargout == 0
            if enabled
                printBreakpointInfo(i, startAddress, endAddress, triggerOnRead, triggerOnWrite);
            else
                fprintf('DM breakpoint ID %i is free.\n', i);
            end
        else
            if enabled
                breakInfo(i+1).id             = i;
                breakInfo(i+1).startAddress   = double(startAddress);
                breakInfo(i+1).endAddress     = double(endAddress);
                breakInfo(i+1).triggerOnRead  = triggerOnRead;
                breakInfo(i+1).triggerOnWrite = triggerOnWrite;
            end
        end
        
        if enabled
            numSet = numSet + 1;
        end
    end
    
    if nargout == 0
        if numSet == 0
            fprintf('No DM breakpoints are currently set.\n')
        end
    end
    
    return;
elseif nargin < 5
    % Allow alternative help access.
    % Otherwise, either no args, or all five args, are required.
    if nargin == 1 && ischar(startAddress) && strcmpi(startAddress, 'help')
        help('kaldmbreak');
    else
        error('Invalid arguments. See help(kaldmbreak).');
    end
    
    return;
end

availableBreakpoints = ka.get_num_dm_breakpoints();
if breakpointId >= availableBreakpoints
    error('DM breakpoint ID (%i) is greater than the maximum allowed hardware breakpoint ID (%i).',...
          breakpointId, availableBreakpoints - 1);
end

% We assume string addresses are hex numbers.
if ischar(startAddress)
    startAddress = hex2dec(startAddress);
end
if ischar(endAddress)
    endAddress = hex2dec(endAddress);
end

% Are we in removal mode, or set mode?
if (startAddress == -1)
    if breakpointId < -1
        error('DM breakpoint ID %d is not valid. Use -1 to disable all breakpoints.', breakpointId)
    end

    if (breakpointId == -1)
        ka.clear_all_dm_breakpoints();
        fprintf('All DM breakpoints disabled.\n');
    else
        ka.clear_dm_breakpoint(breakpointId);
        fprintf('DM breakpoint ID %i disabled.\n', breakpointId);
    end 
else
    if startAddress < 0
        error('Invalid DM breakpoint start address: %d', startAddress);
    end
    if endAddress < 0
        error('Invalid DM breakpoint end address: %d', endAddress);
    end
    if endAddress < startAddress
        error('DM breakpoint end address (0x%x) was smaller than start address (0x%x)', startAddress, endAddress);
    end
    if breakpointId < 0
        error('DM breakpoint ID %i is not valid.', breakpointId)
    end
    ka.set_dm_breakpoint(breakpointId, startAddress, endAddress, triggerOnRead, triggerOnWrite);
    
    printBreakpointInfo(breakpointId, startAddress, endAddress, triggerOnRead, triggerOnWrite);
end

end % top-level function

function printBreakpointInfo(breakpointId, startAddress, endAddress, triggerOnRead, triggerOnWrite)
    if triggerOnRead
        triggerReadString = 'yes';
    else
        triggerReadString = 'no';
    end
    
    if triggerOnWrite
        triggerWriteString = 'yes';
    else
        triggerWriteString = 'no';
    end
    
    if startAddress == endAddress
        fprintf('DM breakpoint ID %i set at address 0x%x. Trigger on read: %s. Trigger on write: %s.\n',...
                breakpointId, startAddress, triggerReadString, triggerWriteString);
    else
        fprintf('DM breakpoint ID %i set. Start address: 0x%x. End address: 0x%x. Trigger on read: %s. Trigger on write: %s.\n',...
                breakpointId, startAddress, endAddress, triggerReadString, triggerWriteString);
    end
end
