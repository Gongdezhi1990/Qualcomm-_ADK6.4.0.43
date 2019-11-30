function [ varargout ] = kalrunning( varargin )
%KALRUNNING Check if the DSP is running.
% Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd 
% 
%   KALRUNNING runs, pauses, steps and returns the running state of the DSP.
%
%   [STATE] = KALRUNNING returns the state of the DSP as either:
%      1  - running
%      0  - not running
%      -1 - powered off or not clocked
%
%   KALRUNNING( ACTION ) sets the state of the DSP, either running or paused.
%   Available options are:
%         ACTION        RESULT
%        'step' / -1    Single steps the DSP
%        'stop' / 0     Stop the DSP
%       'start' / 1     Start the DSP
%      'switch' / 2     If stopped, start of if running stop the DSP
%
%   NOTE if the DSP is powered off or not clocked, KALRUNNING will not attempt
%   to start or stop it.
%
%   [STATE] = KALRUNNING( ACTION ) sets the state of the DSP and returns the
%   the new state in STATE with the same values as above.
%
%   See also kalloadsym, kalsymfind, kalreadval, kalwriteval.
%

usageMessage = 'Invalid value for argument ACTION. Please enter a value of 0,1,2 or -1 or ''stop'', ''start'', ''switch'' or ''step''';

ka       = kxs.instance();
dspState = ka.ka_get_state();

% If the Kalimba is not enabled, bail out early.
if dspState == ka.KA_STATE_CLOCK_OR_POWER_OFF
    if nargout
        varargout{1} = -1;
    else
        fprintf('The DSP is either powered off or not clocked.\n');
    end
    return;
end

% If we have an arg, we're in modify-state mode.
if nargin
    % Normalize argument - convert string forms to their numerical equivalents
    if isstr(varargin{1})
        switch varargin{1}
            case 'stop'
                varargin{1} = 0;
            case 'start'
                varargin{1} = 1;
            case 'switch'
                varargin{1} = 2;
            case 'step'
                varargin{1} = -1;
            otherwise
                % Treat as a number disguised as a string
                varargin{1} = str2double(varargin{1});
                if isnan(varargin{1})
                    error(usageMessage);
                end
        end
    end
    
    switch varargin{1}
        case 0 % stop / pause
            ka.ka_pause();
        case 1 % run
            ka.ka_run();
        case 2 % switch
            if dspState == ka.KA_STATE_RUNNING
                ka.ka_pause();
            else
                ka.ka_run();
            end
        case -1 % step
            ka.ka_step();
        otherwise
            error(usageMessage);
    end
    
    % Read the state against, following actions above.
    dspState = ka.ka_get_state();
end

if dspState == ka.KA_STATE_RUNNING % running
    dspState = 1;
else                               % not running
    dspState = 0;
end

if nargout
    varargout{1} = dspState;
elseif ~nargin
    if dspState
        fprintf('The DSP is running.\n');
    else
        fprintf('The DSP is NOT running.\n');
    end
end

end % top-level function
