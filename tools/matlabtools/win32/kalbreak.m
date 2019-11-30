function [varargout] = kalbreak(pc, varargin)
%KALBREAK sets the hardware breakpoints on a Kalimba DSP.
% Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd 
% 
% KALBREAK( ADDR, BREAKPOINT_ID ) sets the specified hardware PM breakpoint
% to ADDR.
%
% If BREAKPOINT_ID is not specified, 0 is assumed.
%
% There is 1 hardware PM breakpoint on BC3, 4 on BC5, and 8 on BC7/8 and newer chips.
%
% Passing an ADDR of -1 will disable that hardware PM breakpoint.
%
% If BREAKPOINT_ID is also -1, this will disable all hardware PM breakpoints.
%
% If ADDR is a string it is treated as a hexadecimal value, unless
% it is ''help'', which displays this message.

ka = kxs.instance();

display_breakpoints = 0;
if (nargin == 0)
    % Display which hardware breakpoints are active
    pc = 0;
    breakpoint_id = 0;
    display_breakpoints = 1;
elseif (nargin == 1)
    breakpoint_id = 0;
else
    breakpoint_id = varargin{1};
end

max_breakpoints = ka.get_num_pm_breakpoints();

if breakpoint_id >= max_breakpoints
   fprintf(' Breakpoint ID (%i) is greater than the maximum allowed hardware breakpoint ID (%i).\n\n', breakpoint_id, max_breakpoints - 1)
   pc = 'help';
elseif breakpoint_id < -1
   fprintf(' Breakpoint ID (%i) is less than zero.\n\n', breakpoint_id)
   pc = 'help';
end

if isstr(pc)
    % is it help
    if strcmpi( pc, 'HELP' )
        help('kalbreak')
        return
    else
        % no it must be an address
        pc = hex2dec(pc);
    end
end

if display_breakpoints
    num_set = 0;
    for i = 0:max_breakpoints - 1
        [enabled, address] = ka.get_pm_breakpoint(i);
        if enabled
            fprintf(' Hardware breakpoint ID %i set at PC 0x%x\n', i, address)
            num_set = num_set + 1;
        end
    end
    
    if num_set == 0
        fprintf(' No hardware breakpoints are currently set.\n')
    end
    
    return
end

if (pc == -1)
    if (breakpoint_id == -1)
        % disable all hardware breakpoints
        ka.clear_all_pm_breakpoints();
    else
        % disable selected hardware breakpoint
        ka.clear_pm_breakpoint(breakpoint_id);
    end 
    fprintf(' Breakpoint ID %i disabled.\n', breakpoint_id)
else
    % set the breakpoint address
    ka.set_pm_breakpoint(breakpoint_id, pc);
    fprintf(' Hardware breakpoint ID %i set at PC 0x%x\n', breakpoint_id, pc)
end
